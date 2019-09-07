#ifndef __PWI_SENSOR_H__
#define __PWI_SENSOR_H__

#include <Arduino.h>
#include <core/MyMessage.h>
#include "pwiTimer.h"

/*
 * A base class for any measurement sensor.
 *
 * A measurement sensor defines:
 * - the max frequency at which send to the controller the changes of the
 *   measure through the 'min_period_ms' timer
 * - the min frequency at which even an unchanged measure must be sent to
 *   the controller through the 'max_period_ms' timer
 * - appropriate callbacks to:
 *   > take the measure
 *   > send the measure.
 *
 * The two timers are automatically restarted.
 *
 * Usage synopsys:
 * a) define the sensor variable:
 *    pwiSensor mySensor;
 * b) present the sensor to the controller:
 *    mySensor.present( id, type, label );
 * c) configure the sensor:
 *    mySensor.setup( min_period, max_period, measure_cb, send_cb, user_data );
 *
 * pwi 2019- 5-18 v1 creation
 * pwi 2019- 5-18 v2 the sensor can be armed/unarmed
 * pwi 2019- 5-25 v3 new trigger() method
 *                   fix setArmed() with payload
 * pwi 2019- 5-27 v4 renamed to pwiSensor
 * pwi 2019- 9- 3 review comments
 *                introduce PWI_SENSOR return codes
 *                setup() takes first min, then max periods => breaks backward compatibility
 *                trigger() is renamed to measureAndSend()
 * pwi 2019- 9- 7 take a copy of the sensor label
 */

/* The prototype for the send callback function to be provided by the caller.
   This function receives the 'user_data' parameter provided at setup() time,
   and should send the last taken measure to the controller.
   No return value is expected.
 */
typedef void ( *pwiSendCb )( void * );

/* The prototype for the measure callback function to be provided by the caller.
   This function receives the 'user_data' parameter provided at setup() time.
   It is expected to take the measure, and store the result in a place available
   to the send callback.
   This function must return %TRUE if the measure has changed.
 */
typedef bool ( *pwiMeasureCb )( void * );

enum {
    PWI_SENSOR_OK = 0,
    PWI_SENSOR_ERR01,                           // max period greater than zero, but smaller than min period
    PWI_SENSOR_ERR02,                           // min period greater than max period (and max period is set)
    PWI_SENSOR_ERR03                            // invalid ARM command, 'ARM=0|1' expected
};

class pwiSensor {
    public:
                      pwiSensor();
        uint8_t       getId();
        bool          isArmed();
        void          measureAndSend();
        void          present( uint8_t id, uint8_t type, const char *label );
        void          send();
        void          setArmed( bool armed );
        uint8_t       setArmed( const char *payload );
        uint8_t       setMaxPeriod( unsigned long delay_ms );
        uint8_t       setMinPeriod( unsigned long delay_ms );
        void          setSendOnChange( bool send );
        void          setup( unsigned long min_period_ms, unsigned long max_period_ms, pwiMeasureCb measureCb, pwiSendCb sendCb, void *user_data=NULL );

    private:
        uint8_t       id;
        uint8_t       type;
        char          label[1+MAX_PAYLOAD];
        bool          armed;
        bool          send_on_change;
        pwiTimer      min_timer;                // min period, aka max frequency
        pwiTimer      max_timer;                // max period, aka unchanged timeout
        pwiMeasureCb  measureCb;
        pwiSendCb     sendCb;
        void         *user_data;

        static void   OnMinPeriodCb( pwiSensor *node);
        static void   OnMaxPeriodCb( pwiSensor *node);
};

#endif // __PWI_SENSOR_H__

