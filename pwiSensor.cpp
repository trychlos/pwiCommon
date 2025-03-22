/*
 * pwi 2019- 5-18 v1 creation
 * pwi 2019- 5-19 v2 add debug messages
 *                   fix MySensors present() scope
 * pwi 2019- 5-20 v3 stop the timers on unarming
 * pwi 2019- 5-25 v4 pwiTimer::set() becomes pwiTimer::setup()
 *                   new trigger() method
 *                   fix setArmed() with payload
 * pwi 2019- 5-27 v5 renamed to pwiSensor
 * pwi 2019- 9- 3 fix typo
 * pwi 2019- 9- 7 take a copy of the sensor label
 *                new methods: getMaxPeriod(), getMinPeriod()
 *                let sending the measure be forced
 * pwi 2019- 9- 8 id and pin are construction-time data
 *                remove unused send_on_change flag
 *                remove setArmed() methods
 *                all methods become virtual
 * pwi 2019- 9-11 remove label
 *                remove present() method
 *                move constant strings to flash memory and keep them there
 * pwi 2019- 9-15 v190904
 *                define an empty constructor
 *                new setId() method
 * pwi 2025- 3-21 v250321
 *                new setPin() method
 * pwi 2025- 3-22 BREAKING CHANGE: remove pin from this class - this has now to be implemented by the derived class
 *                BREAKING CHANGE: replace measureCb() method by protected virtual vMeasure()
 *                BREAKING CHANGE: replace sendCb() method by protected virtual vSend()
 *                BREAKING CHANGE: remove setup() method
 *                new setMeasureCb(), setSendCb() methods
 */

#include <core/MySensorsCore.h>
#include "pwiSensor.h"

// uncomment to debugging this file
//#define SENSOR_DEBUG

static char const strMinTimer[] PROGMEM = "MinTimer #";
static char const strMaxTimer[] PROGMEM = "MaxTimer #";

/**
 * pwiSensor::pwiSensor:
 * @id: the child identifier inside of this MySensor node; must be unique for this node.
 * 
 * Constructor.
 *
 * Public.
 */
pwiSensor::pwiSensor( void )
{
    this->init();
}

pwiSensor::pwiSensor( uint8_t id )
{
    this->init();

    this->id = id;
}

/*
 * Private pwiSensor::init:
 */
void pwiSensor::init( void )
{
    this->id = 0;
}

/**
 * pwiSensor::getId:
 * 
 * Returns: the node identifier as provided to the present() method.
 *
 * Public.
 */
uint8_t pwiSensor::getId()
{
    return( this->id );
}

/**
 * pwiSensor::getMaxTimer:
 *
 * Returns: a reference to the max timer (unchanged timeout).
 *
 * Public.
 */
pwiTimer &pwiSensor::getMaxTimer()
{
    return( this->max_timer );
}

/**
 * pwiSensor::getMinTimer:
 *
 * Returns: a reference to the min timer (max frequency).
 *
 * Public.
 */
pwiTimer &pwiSensor::getMinTimer()
{
    return( this->min_timer );
}

/**
 * pwiSensor::setId:
 * @id: the child identifier inside of this MySensor node; must be unique for
 *  this node.
 *
 * Set the child sensor identifier.
 *
 * Public
 */
void pwiSensor::setId( uint8_t id )
{
    this->id = id;
}

/**
 * pwiSensor::setMaxPeriod:
 * @delay_ms: the maximal period at which the measure has to be sent.
 *  This is also called the unchanged timeout: the delay for re-sending a measure
 *  which has not changed.
 *  This maximal period corresponds to a sort of heartbeat for the sensor: if
 *  no message has been received after this interval, then the sensor should be
 *  considered as dead.
 *  If zero, the corresponding timer is disabled.
 *  Else, and greater than the min period, the timer is setup and started.
 *  If not zero, but smaller than the min period, then an error is logged and
 *  returned. The previous max period is left unchanged.
 *
 * Configure and start the max timer.
 *
 * Returns: %PWI_SENSOR_OK if the timer has been successfully set, or the error
 * code.
 *
 * Public
 */
uint8_t pwiSensor::setMaxPeriod( unsigned long delay_ms )
{
    unsigned long min_period = this->min_timer.getDelay();
    if( delay_ms && ( delay_ms < min_period )){
        return( PWI_SENSOR_ERR01 );
    }
	// delay_ms may be zero
    char label[1+MAX_PAYLOAD];
    snprintf_P( label, sizeof( label ), PSTR( "%S%u" ), strMaxTimer, this->getId());
    this->max_timer.setup( label, delay_ms, false, pwiSensor::OnMaxPeriodCb, this );
    this->max_timer.start();
    return( PWI_SENSOR_OK );
}

/**
 * pwiSensor::setMinPeriod:
 * @delay_ms: the minimal period at which the measure has to be taken.
 *  This is also called the maximal frequency: the minimal delay for the
 *  controller not to be flooded. At this period, the measure is taken and
 *  sent to the controller.
 *  If zero, then stop the timer.
 *  If greater than the max period, then an error is logged and returned. The
 *  previous min period is left unchanged.
 *
 * Configure and start the min timer.
 *
 * Returns: %PWI_SENSOR_OK if the timer has been successfully set, or the error
 * code.
 *
 * Public
 */
uint8_t pwiSensor::setMinPeriod( unsigned long delay_ms )
{
    unsigned long max_period = this->max_timer.getDelay();
    if( max_period && max_period < delay_ms ){
        return( PWI_SENSOR_ERR02 );
    }
	// delay_ms may be zero
    char label[1+MAX_PAYLOAD];
    snprintf_P( label, sizeof( label ), PSTR( "%S%u" ), strMinTimer, this->getId());
    min_timer.setup( label, delay_ms, false, pwiSensor::OnMinPeriodCb, this );
    this->min_timer.start();
    return( PWI_SENSOR_OK );
}

/**
 * pwiSensor::setTimers():
 * 
 * Define min and max periods.
 * 
 * Public
 */
uint8_t pwiSensor::setTimers( unsigned long min_ms, unsigned long max_ms )
{
	uint8_t minRes = this->setMinPeriod( min_ms );
    uint8_t maxRes = this->setMaxPeriod( max_ms );
	return( max( minRes, maxRes ));
}

/**
 * pwiSensor::OnMaxPeriodCb:
 * 
 * Callback to handle the maximum period (the heartbeat).
 * Unconditionnaly send the last taken measure through a virtual method which MUST be implemented by the derived class.
 *
 * Note: the @max_timer will be automatically restarted by the pwiTimer class
 *  after this method has returned.
 * 
 * Private Static.
 */
void pwiSensor::OnMaxPeriodCb( pwiSensor *sensor )
{
#ifdef SENSOR_DEBUG
    Serial.print( F( "pwiSensor::OnMaxPeriodCb() id=" ));
    Serial.println( sensor->id );
#endif
    sensor->vSend();
}

/**
 * pwiSensor::OnMinPeriodCb:
 * 
 * Callback to handle the minimum period (the max frequency of the measure).
 *
 * Note: the @min_timer will be automatically restarted by the pwiTimer class
 *  after this method has returned.
 * 
 * Private Static.
 */
void pwiSensor::OnMinPeriodCb( pwiSensor *sensor )
{
#ifdef SENSOR_DEBUG
    Serial.print( F( "pwiSensor::OnMinPeriodCb() id=" ));
    Serial.println( node->id );
#endif
	if( sensor->vMeasure()){
		sensor->vSend();
	}
}
