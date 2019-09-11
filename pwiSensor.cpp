#include "pwiSensor.h"
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
 */

#include <core/MySensorsCore.h>

// uncomment to debugging this file
//#define SENSOR_DEBUG

static char const strMinTimer[] PROGMEM = "MinTimer #";
static char const strMaxTimer[] PROGMEM = "MaxTimer #";

/**
 * pwiSensor::pwiSensor:
 * @id: the child identifier inside of this MySensor node; must be unique for
 *  this node.
 * @pin: the pin number to which the physical device is attached.
 * 
 * Constructor.
 *
 * Public.
 */
pwiSensor::pwiSensor( uint8_t id, uint8_t pin )
{
    this->init();

    this->id = id;
    this->pin = pin;
}

/*
 * Private pwiSensor::init:
 */
void pwiSensor::init( void )
{
    this->id = 0;
    this->pin = 0;

    this->type = 0;
    this->sendCb = NULL;
  	this->measureCb = NULL;
    this->user_data = NULL;
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
 * pwiSensor::getPin:
 * 
 * Returns: the pin number to which the physical device is attached.
 *
 * Public.
 */
uint8_t pwiSensor::getPin()
{
    return( this->pin );
}

/**
 * pwiSensor::measureAndSend:
 * @forced: whether the measure should be sent even if unchanged.
 *
 * Take the measure, sending the message if needed.
 * Reset timers as a side effect.
 *
 * Public.
 */
void pwiSensor::measureAndSend( bool force )
{
    if( this->measureCb ){
        bool changed = this->measureCb( this->user_data );
        if( changed || force ){
            this->send();
            this->max_timer.restart();
        }
    }
}

/**
 * pwiSensor::send:
 *
 * Send the message unconditionnally (at least if the sensor is armed).
 *
 * Public
 */
void pwiSensor::send()
{
    if( this->sendCb ){
        this->sendCb( this->user_data );
        this->min_timer.restart();
    }
}

/**
 * pwiSensor::setMaxPeriod:
 * @delay_ms: the maximal period at which the measure has to be sent.
 *  This is also called the unchanged timeout: the delay for re-sending a measure
 *  which has not changed.
 *  This minimal frequency corresponds to a sort of heartbeat for the sensor: if
 *  no message has been received during this interval, then the sensor should be
 *  considered as dead.
 *  If zero, the corresponding timer is disabled.
 *  Else, and greater than the min period, the timer is setup and started.
 *  If not zero, but smaller than the min period, then an error is logged and
 *  returned. The max timer is left unchanged.
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
 *  controller not to be flooded. At this frequency, the measure is taken and
 *  sent to the controller (if send_on_change is %TRUE, which is the constructor
 *  default).
 *  If zero, then stop the timer.
 *  If greater than the max period, then an error is logged and returned. The
 *  min timer is left unchanged.
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
 * pwiSensor::setup:
 * @min_period_ms: the min period, aka the max frequency, in ms.
 *  Only applies if greater than zero, the sensor is armed, and a @measureCb
 *  callback function has been provided.
 *  Measures are taken at this exact frequency.
 *  Set to zero to disable the timer, and thus disable all measures.
 *  Must be smaller than the @max_period_ms if this later is greater than zero.
 * @max_period_ms: the max period, aka the unchanged timeout, in ms.
 *  Only applies if greater than zero, the sensor is armed, and a @sendCb
 *  callback function has been provided.
 *  Last taken measure is unconditionnally sent to the controller.
 *  Set to zero to disable the timer, and thus disable all messages.
 * @measureCb: the callback function which takes the measure.
 * @sendCb: the callback function which sends the last measure to the controller.
 * @user_data: [allow-none]: the user data to be passed to the callbacks.
 *  As callbacks are most often static methods, @user_data should be a pointer
 *  to the sensor object.
 *
 * Configure the sensor.
 * 
 * Public
 */
void pwiSensor::setup( unsigned long min_period_ms, unsigned long max_period_ms, pwiMeasureCb measureCb, pwiSendCb sendCb, void *user_data )
{
#ifdef SENSOR_DEBUG
    Serial.print( F( "pwiSensor::setup() id=" ));
    Serial.print( this->id );
    Serial.print( F( ", min_period_ms=" ));
    Serial.print( min_period_ms );
    Serial.print( F( ", max_period_ms=" ));
    Serial.println( max_period_ms );
#endif
    // setup the timers
    this->setMinPeriod( min_period_ms );
    this->setMaxPeriod( max_period_ms );
    // record the callbacks
    this->measureCb = measureCb;
    this->sendCb = sendCb;
    this->user_data = user_data;
}

/**
 * pwiSensor::OnMaxPeriodCb:
 * 
 * Callback to handle the maximum period (the heartbeat).
 * Unconditionnaly send the last taken measure.
 *
 * Note: the @max_timer will be automatically restarted by the pwiTimer class
 *  after this method has returned.
 * 
 * Private Static.
 */
void pwiSensor::OnMaxPeriodCb( pwiSensor *node )
{
#ifdef SENSOR_DEBUG
    Serial.print( F( "pwiSensor::OnMaxPeriodCb() id=" ));
    Serial.println( node->id );
#endif
    node->send();
}

/**
 * pwiSensor::OnMinPeriodCb:
 * 
 * Callback to handle the minimum period (the max frequency of the measure).
 * If the sensor is not armed, then no measure is taken.
 *
 * Note: the @min_timer will be automatically restarted by the pwiTimer class
 *  after this method has returned.
 * 
 * Private Static.
 */
void pwiSensor::OnMinPeriodCb( pwiSensor *node )
{
#ifdef SENSOR_DEBUG
    Serial.print( F( "pwiSensor::OnMinPeriodCb() id=" ));
    Serial.println( node->id );
#endif
	node->measureAndSend();
}

