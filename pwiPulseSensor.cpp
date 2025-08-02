/*
 * A Pulse sensor is a sensor which counts impulsions on a given pin.
 * Impulsions are detected at loop time by just reading the pin state.
 * Impulsions can be detected on falling or rising edge.
 * 
 * pwi 2025- 3-22 creation
 * pwi 2025- 3-23 initialize input pin mode and state
 * pwi 2025- 7-31 add bounce protection
 */

#include "pwiPulseSensor.h"

// uncomment to debugging this file
//#define SENSOR_DEBUG

/*
 * Constructor
 */

pwiPulseSensor::pwiPulseSensor()
{
	this->init();
}

pwiPulseSensor::pwiPulseSensor( uint8_t id, uint8_t input_pin, uint8_t edge )
{
	this->init();
	this->setId( id );
	this->setInputPin( input_pin );
	this->setEdge( edge );
}

void pwiPulseSensor::init()
{
	// setup
	this->input_pin = 0;
	this->edge = 0;
	// runtime
	this->last_state = 0;
	this->imp_count = 0;
	this->last_ms = 0;
}

/**
 * pwiPulseSensor::getEdge():
 * 
 * Public
 */
uint8_t pwiPulseSensor::getEdge()
{
    return( this->edge );
}

/**
 * pwiPulseSensor::getInputPin():
 * 
 * Public
 */
uint8_t pwiPulseSensor::getInputPin()
{
    return( this->input_pin );
}

/**
 * pwiPulseSensor::getPulsesCount():
 * 
 * Public
 */
uint32_t pwiPulseSensor::getPulsesCount()
{
    return( this->imp_count );
}

/**
 * pwiPulseSensor::setEdge():
 * 
 * Public
 */
void pwiPulseSensor::setEdge( uint8_t edge )
{
    this->edge = edge;
}

/**
 * pwiPulseSensor::setInputPin():
 * 
 * Public
 */
void pwiPulseSensor::setInputPin( uint8_t input_pin )
{
    this->input_pin = input_pin;
	if( input_pin ){
		digitalWrite( input_pin, HIGH );
		pinMode( input_pin, INPUT );
	}
}

/**
 * pwiPulseSensor::setPulseLength():
 *
 * Set the length of the impulsion in ms
 * 
 * Public
 */
void pwiPulseSensor::setPulseLength( uint8_t length_ms )
{
    this->length_ms = length_ms;
}

/**
 * pwiPulseSensor::loopInput():
 * 
 * Test for a falling/rising edge on the input pin: this is counted as *one* impulsion
 * Debouncing: do not even look at the pin state during the length of last impulsion
 *
 * Returns true if a pulse has been detected
 * 
 * Public
 */
bool pwiPulseSensor::loopInput()
{
	bool isEdge = false;
	uint32_t now = millis();
	uint32_t start_ms = this->last_ms + this->length_ms;
	if( now > start_ms ){
		uint8_t state = digitalRead( this->input_pin );

		isEdge = ( this->edge == FALLING && state == LOW && this->last_state == HIGH )
					|| ( this->edge == RISING && state == HIGH && this->last_state == LOW );

		if( isEdge ){
			this->imp_count += 1;
			this->last_ms = now;
#ifdef SENSOR_DEBUG
			Serial.print( F( "edge detected count=" ));
			Serial.println( this->imp_count );
#endif
		}
		this->last_state = state;
	}
	return isEdge;
}
