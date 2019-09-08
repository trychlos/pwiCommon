#include "pwiDallasSensor.h"
#include <core/MySensorsCore.h>

/*
 * pwi 2019- 9- 5 creation
 * pwi 2019- 9- 7 identify the sensor on its label rather than on its MySensor Id
 * pwi 2019- 9- 8 update pwiSensor base class
 */

// uncomment to debugging this file
//#define DALLAS_DEBUG

/**
 * pwiDallasSensor::pwiDallasSensor:
 * @id: the child identifier inside of this MySensor node; must be unique for
 *  this node.
 * @pin: the pin number to which the physical device is attached; here, this is
 *  the OneWire pin.
 * 
 * Constructor.
 *
 * Public.
 */
pwiDallasSensor::pwiDallasSensor( uint8_t id, uint8_t pin ) : pwiSensor( id, pin )
{
    this->init();
}

/* private initialization method
 */
void pwiDallasSensor::init( void )
{
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
 *
 * Public.
 */
void pwiDallasSensor::before()
{
#ifdef DALLAS_DEBUG
    Serial.print( F( "pwiDallasSensor::before() id=" ));
    Serial.println( this->getId());
#endif
    uint8_t pin = this->getPin();
    if( pin > 0 ){
        this->oneWireBus.begin( pin );
        this->dallasTemperatureBus.setOneWire( &this->oneWireBus );
        this->dallasTemperatureBus.begin();
        // requestTemperatures() will not block current thread
        this->dallasTemperatureBus.setWaitForConversion( false );
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
    uint8_t id = this->getId();
#ifdef DALLAS_DEBUG
    Serial.print( F( "pwiDallasSensor::present() id=" ));
    Serial.println( id );
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
        int child_id = id+i;
        String str = label + i;
        if( i == 0 ){
            dynamic_cast< pwiSensor& >( *this ).present( S_TEMP, str.c_str());
        } else {
            ::present( child_id, S_TEMP, str.c_str());
        }
    }
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
    Serial.print( F( "pwiDallasSensor::setup() id=" ));
    Serial.println( this->getId());
#endif
    unsigned long min_period = 60000;     // 1 mn
    unsigned long max_period = 1800000;   // 30 mn

    if( this->device_count > 0 ){
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
            Serial.print( F( "pwiDallasSensor::measure() id=" ));
            Serial.print( this->getId());
            Serial.print( F( ", i=" ));
            Serial.print( i );
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
    uint8_t id = this->getId();
#ifdef DALLAS_DEBUG
    Serial.print( F( "pwiDallasSensor::send() id=" ));
    Serial.println( id );
#endif
    MyMessage msg;
    for( int i=0 ; i<this->device_count ; ++i ){
        float ftemp = this->measures[i] / 10.;
        msg.clear();
        ::send( msg.setSensor( id+i ).setType( V_TEMP ).set( ftemp, 1 ));
    }
}

