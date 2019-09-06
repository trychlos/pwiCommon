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
 * - the pin to which the OneWire bus is attached has to be specified at construction time;
 * - temperature sensors found on this bus are given incremental id's starting from 10.
 * 
 * In this version, we manage a maximum of 5 sensors on a same bus.
 *
 * pwi 2019- 9- 5 creation
 */

#define PWI_DALLAS_SENSOR_MAX_ATTACHED   5
#define PWI_DALLAS_SENSOR_START_ID      10

class pwiDallasSensor : public pwiSensor {
    public:
                          pwiDallasSensor( void );
                          pwiDallasSensor( uint8_t oneWirePin );
        void              before();
        void              present();
        void              setOneWirePin( uint8_t oneWirePin );
        void              setup();

    private:
        /* construction data
         */
        uint8_t           oneWirePin;

        /* runtime data
         */
        OneWire           oneWireBus;
        DallasTemperature dallasTemperatureBus;
        bool              dallasBusInitialized;
        uint8_t           device_count;
        uint8_t           measures[PWI_DALLAS_SENSOR_MAX_ATTACHED];

        static bool       Measure( pwiDallasSensor *sensor )    { return( sensor->measure()); }
        static void       Send( pwiDallasSensor *sensor )       { sensor->send(); }

        void              init();
        bool              measure();
        void              send();
};

#endif // __PWI_DALLAS_SENSOR_H__

