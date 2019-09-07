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
 */

#include <core/MySensorsCore.h>

// uncomment to debugging this file
//#define SENSOR_DEBUG

static const char * const strMinPeriodTimer = "MinPeriodTimer";
static const char * const strMaxPeriodTimer = "MaxPeriodTimer";

/**
 * pwiSensor::pwiSensor:
 * 
 * Constructor.
 *
 * Public.
 */
pwiSensor::pwiSensor( void )
{
    this->id = 0;
    this->type = 0;
    memset( this->label, '\0', 1+MAX_PAYLOAD );
    this->armed = false;
    this->sendCb = NULL;
  	this->measureCb = NULL;
    this->send_on_change = true;

    this->setArmed( true );
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
 * pwiSensor::isArmed:
 * 
 * Returns: %TRUE if the sensor is armed.
 * 
 * If the sensor is not armed, no measure is taken.
 * In this case, the sent messages are ARMED=false, TRIPPED=false.
 *
 * At construction time, pwiSensor's default to be armed.
 *
 * Public.
 */
bool pwiSensor::isArmed()
{
    return( this->armed );
}

/**
 * pwiSensor::measureAndSend:
 *
 * Take the measure, sending the message if needed.
 * Reset timers as a side effect.
 *
 * Public.
 */
void pwiSensor::measureAndSend()
{
    if( this->armed ){
        if( this->measureCb ){
            bool changed = this->measureCb( this->user_data );
            if( changed && this->send_on_change ){
                this->send();
                this->max_timer.restart();
            }
        }
    }
}

/**
 * pwiSensor::present:
 * @id: the child identifier inside of this MySensor node; must be unique inside
 *  of this node.
 * @type: the MySensor type of this child sensor.
 * @label: a qualifying label for this child sensor.
 *  A copy of the string is taken; the provided @label may so be safely freed by
 *  the caller after this function returns.
 *
 * Present the node to the controller.
 * This also may be done directly by the main program, but this method lets the
 * object take a copy of some characteristics of the sensor node, and these data
 * may be usefully used in debug messages.
 *
 * So this method is just for having readable debug messages.
 *
 * Public
 */
void pwiSensor::present( uint8_t id, uint8_t type, const char *label )
{
#ifdef SENSOR_DEBUG
    Serial.print( F( "pwiSensor::present() id=" ));
    Serial.print( id );
    Serial.print( F( ", type=" ));
    Serial.print( type );
    Serial.print( F( ", label=" ));
    Serial.println( label );
#endif
    this->id = id;
    this->type = type;
    strncpy( this->label, label, MAX_PAYLOAD );
    ::present( this->id, this->type, this->label );
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
    if( this->armed ){
        if( this->sendCb ){
            this->sendCb( this->user_data );
            this->min_timer.restart();
        }
    }
}

/**
 * pwiSensor::setArmed:
 * @armed: whether this sensor must be armed.
 *  As a reminder, only armed sensors take measure, and consequently send changes
 *  to the controller.
 *  As a side effect, unarming the sensor stops all the timers.
 * 
 * Arm/unarm the sensor.
 *
 * Public
 */
void pwiSensor::setArmed( bool armed )
{
    this->armed = armed;
    if( !armed ){
        this->min_timer.stop();
        this->max_timer.stop();
    }
}

/**
 * pwiSensor::setArmed:
 * @payload: a string received from the controller, which is expected to be an
 *  arm or unarm command.
 *  Accepted strings are:
 *  'ARM=1': arm the sensor (take measures, send changes, and so on)
 *  'ARM=0': unarm the sensor, disabling all measures.
 *
 * Arm the sensor depending of a received message.
 *
 * Returns: %PWI_SENSOR_OK if the arm status has been successfully set, or the
 * error code.
 *
 * Public
 */
uint8_t pwiSensor::setArmed( const char *payload )
{
    if( strncmp( payload, "ARM=", 4 ) != 0 ){
        return( PWI_SENSOR_ERR03 );
    }
    if( strlen( payload ) != 5 ){
        return( PWI_SENSOR_ERR03 );
    }
    char arg = payload[4];
    if( arg != '0' && arg != '1' ){
        return( PWI_SENSOR_ERR03 );
    }
    this->setArmed( arg == '1' );
    return( PWI_SENSOR_OK );
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
    this->max_timer.setup( strMaxPeriodTimer, delay_ms, false, pwiSensor::OnMaxPeriodCb, this );
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
    min_timer.setup( strMinPeriodTimer, delay_ms, false, pwiSensor::OnMinPeriodCb, this );
    this->min_timer.start();
    return( PWI_SENSOR_OK );
}

/**
 * pwiSensor::setSendOnChange:
 * @send: whether a message should be sent to the controller each time the
 *  measure changes.
 *  This is the default behavior set at construction time for all our sensor
 *  nodes.
 *  But, this should not be the case for alarm nodes, where the message must
 *  only be send after the expiration of the grace period.
 *
 * Configure the 'send on change' behavior.
 *
 * Public
 */
void pwiSensor::setSendOnChange( bool send )
{
    this->send_on_change = send;
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

