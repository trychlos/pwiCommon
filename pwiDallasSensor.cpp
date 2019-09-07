#include "pwiDallasSensor.h"
#include <core/MySensorsCore.h>

/*
 * pwi 2019- 9- 5 creation
 * pwi 2019- 9- 7 identify the sensor on its label rather than on its MySensor Id.
 */

// uncomment to debugging this file
//#define DALLAS_DEBUG

/**
 * pwiDallasSensor::pwiDallasSensor:
 * @oneWirePin: the pin number which is to be managed as our OneWire bus.
 * 
 * Constructor.
 *
 * Public.
 */
pwiDallasSensor::pwiDallasSensor( void )
{
    this->init();
}

pwiDallasSensor::pwiDallasSensor( uint8_t oneWirePin )
{
    this->init();
    this->setOneWirePin( oneWirePin );
}

/* private initialization method
 */
void pwiDallasSensor::init( void )
{
    this->oneWirePin = 0;
    this->dallasBusInitialized = false;
    this->device_count = 0;

    for( int i=0 ; i<PWI_DALLAS_SENSOR_MAX_ATTACHED ; ++i ){
        this->measures[i] = 0;
    }
}

/**
 * pwiDallasSensor::before:
 * 
 * Initialise the OneWire bus.
 * This requires that the OneWire pin has been previously defined.
 *
 * Public.
 */
void pwiDallasSensor::before()
{
#ifdef DALLAS_DEBUG
    Serial.print( F( "pwiDallasSensor::before() oneWirePin=" ));
    Serial.println( this->oneWirePin );
#endif
    if( this->oneWirePin > 0 ){
        this->dallasTemperatureBus.begin();
        this->dallasBusInitialized = true;
    }
}

/**
 * pwiDallasSensor::present:
 * 
 * Present each temperature sensor, with ID's from 10 to n.
 *
 * Public.
 */
void pwiDallasSensor::present()
{
#ifdef DALLAS_DEBUG
    Serial.print( F( "pwiDallasSensor::present() oneWirePin=" ));
    Serial.println( this->oneWirePin );
#endif
    String label = "Temperature sensor #";
    if( this->dallasBusInitialized ){
        //this->device_count = this->dallasTemperatureBus.getDeviceCount();
        this->device_count = this->dallasTemperatureBus.getDS18Count();
        if( this->device_count > PWI_DALLAS_SENSOR_MAX_ATTACHED ){
            this->device_count = PWI_DALLAS_SENSOR_MAX_ATTACHED;
        }
    }
    for( int i=0 ; i<this->device_count ; ++i ){
        int id = i+PWI_DALLAS_SENSOR_START_ID;
        String str = label + i;
        dynamic_cast< pwiSensor& >( *this ).present( id, S_TEMP, str.c_str());
    }
}

/**
 * pwiDallasSensor::setOneWirePin:
 * @oneWirePin: the pin number which is to be managed as our OneWire bus.
 * 
 * Set the pin number to which the OneWire bus is attached.
 *
 * Public.
 */
void pwiDallasSensor::setOneWirePin( uint8_t pin )
{
    this->oneWirePin = pin;

    this->oneWireBus.begin( pin );
    this->dallasTemperatureBus.setOneWire( &this->oneWireBus );
}

/**
 * pwiDallasSensor::setup:
 * 
 * Configure the temperature sensor.
 *
 * Public.
 */
void pwiDallasSensor::setup( void )
{
#ifdef DALLAS_DEBUG
    Serial.print( F( "pwiDallasSensor::setup() oneWirePin=" ));
    Serial.println( this->oneWirePin );
#endif
    unsigned long min_period = 60000;     // 1 mn
    unsigned long max_period = 1800000;   // 30 mn

    if( this->device_count > 0 ){
        // requestTemperatures() will not block current thread
        this->dallasTemperatureBus.setWaitForConversion( false );
        dynamic_cast< pwiSensor& >( *this ).setup( min_period, max_period, pwiDallasSensor::Measure, pwiDallasSensor::Send, this );
    }
}

/* Private pwiDallasSensor::measure:
 */
bool pwiDallasSensor::measure()
{
    bool changed = false;

    if( this->device_count > 0 ){
        // Fetch temperatures from Dallas sensors
        this->dallasTemperatureBus.requestTemperatures();
    
        // query conversion time and sleep until conversion completed
        //int16_t conversionTime = temp_Dallas.millisToWaitForConversion(temp_Dallas.getResolution());
        // sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
        //wait( conversionTime );
    
        // Fetch and round temperature to one decimal
        for( int i=0 ; i<this->device_count ; i++ ){
            float temperature = this->dallasTemperatureBus.getTempCByIndex(i);
            int itemp = ( int )( temperature * 10. );
#ifdef DALLAS_DEBUG
            Serial.print( F( "pwiDallasSensor::measure() oneWirePin=" ));
            Serial.print( this->oneWirePin );
            Serial.print( F( ", temp=" ));
            Serial.print( temperature );
            Serial.print( F( ", itemp=" ));
            Serial.println( itemp );
#endif
            if( itemp != -1270 && itemp != 850 ){
                bool temp_changed = ( this->measures[i] != itemp );
                changed |= temp_changed;
                this->measures[i] = itemp;
            }
        }
    }

    return( changed );
}

/* Private pwiDallasSensor::send:
 */
void pwiDallasSensor::send()
{
#ifdef DALLAS_DEBUG
    Serial.print( F( "pwiDallasSensor::send() oneWirePin=" ));
    Serial.println( this->oneWirePin );
#endif
    MyMessage msg;
    for( int i=0 ; i<this->device_count ; ++i ){
        float ftemp = this->measures[i] / 10.;
        msg.clear();
        ::send( msg.setSensor( i+PWI_DALLAS_SENSOR_START_ID ).setType( V_TEMP ).set( ftemp, 1 ));
    }
}

