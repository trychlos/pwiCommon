#ifndef __PWI_DALLAS_SENSOR_H__
#define __PWI_DALLAS_SENSOR_H__

#include <Arduino.h>
#include <DallasTemperature.h>
#include <pwiSensor.h>

/*
 * Dallas DS18B20 temperature sensor.
 * 
 * This class actually manages all Dallas temperature sensors attached to a same
 *  single OneWire bus.
 * - the pin to which the OneWire bus is attached has to be specified at
 *   construction time;
 * - temperature sensors found on this bus are given incremental id's starting
 *   from the id specified at construction time.
 * 
 * In this version, we manage a maximum of 5 sensors on a same bus.
 *
 * pwi 2019- 9- 5 creation
 * pwi 2019- 9- 8 update pwiSensor base class
 */

#define PWI_DALLAS_SENSOR_MAX_ATTACHED   5

class pwiDallasSensor : public pwiSensor {
    public:
                                  pwiDallasSensor( uint8_t id, uint8_t pin );
                void              before();
        virtual void              present();
        virtual void              setup();

    private:
        /* construction data
         */

        /* runtime data
         */
                OneWire           oneWireBus;
                DallasTemperature dallasTemperatureBus;
                bool              dallasBusInitialized;
                uint8_t           device_count;
                uint16_t          measures[PWI_DALLAS_SENSOR_MAX_ATTACHED];

        /* private methods
         */
                void              init();
                bool              measure();
                void              send();

                static bool       Measure( pwiDallasSensor *sensor ) { return( sensor->measure()); }
                static void       Send( pwiDallasSensor *sensor )    { sensor->send(); }
};

#endif // __PWI_DALLAS_SENSOR_H__

