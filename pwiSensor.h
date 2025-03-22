#ifndef __PWI_SENSOR_H__
#define __PWI_SENSOR_H__

/*
 * A base class for any measurement sensor.
 *
 * A measurement sensor defines:
 * - the max frequency at which send to the controller the changes of the measure through the 'min_period_ms' timer
 * - the min frequency at which even an unchanged measure must be sent to the controller through the 'max_period_ms' timer
 * - appropriate callbacks to:
 *   > take the measure
 *   > send the measure.
 *
 * The two included timers are automatically managed (restarted) by this class.
 *
 * Usage synopsys:
 *
 * a) define the sensor variable:
 *    pwiSensor mySensor( id );
 *
 * b) present the sensor to the controller:
 *    present( id, type, label );
 *
 * c) configure the sensor:
 *    mySensor.setMinPeriod( min_period );
 *    mySensor.setMaxPeriod( max_period );
 *    mySensor.setMeasureCb( measureCb );
 *    mySensor.setMeasureUserdata( measureUserdata );
 *    mySensor.setSendCb( measureCb );
 *    mySensor.setSendUserdata( measureUserdata );
 *    or:
 *    mySensor.setup( min_period, max_period, measureCb, sendCb, user_data );
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
 *                new methods: getMaxPeriod(), getMinPeriod()
 *                let sending the measure be forced
 * pwi 2019- 9- 8 id and pin are construction-time data
 *                remove unused send_on_change flag
 *                remove setArmed() methods
 *                all methods become        
 * pwi 2019- 9-11 remove label
 *                remove present() method
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

#include "pwiTimer.h"
 
enum {
    PWI_SENSOR_OK = 0,
    PWI_SENSOR_ERR01,                           // max period greater than zero, but smaller than min period
    PWI_SENSOR_ERR02                            // min period greater than max period (and max period is set)
};

class pwiSensor {
    public:
                                  pwiSensor( void );
                                  pwiSensor( uint8_t id );

		/* getters
		 */
                uint8_t           getId();
                pwiTimer         &getMaxTimer( void );
                pwiTimer         &getMinTimer( void );

		/* setters
		 */
                void              setId( uint8_t id );
                uint8_t           setMaxPeriod( unsigned long delay_ms );
                uint8_t           setMinPeriod( unsigned long delay_ms );
				uint8_t	          setTimers( unsigned long min_ms, unsigned long max_ms );

	protected:
		/* virtuals MUST be implemented by the derived class
		 */
        virtual bool              vMeasure() = 0;
        virtual void              vSend() = 0;

    private:
        /* construction data
         */
                uint8_t           id;

        /* runtime data
         */
                pwiTimer          min_timer;                // min period, aka max frequency
                pwiTimer          max_timer;                // max period, aka unchanged timeout

        /* private methods
         */
                void              init();

        static  void              OnMinPeriodCb( pwiSensor *sensor );
        static  void              OnMaxPeriodCb( pwiSensor *sensor );
};

#endif // __PWI_SENSOR_H__

