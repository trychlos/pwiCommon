/*
 * A Pulse sensor is a sensor which counts impulsions on a given pin.
 * Impulsions are detected at loop time by just reading the pin state.
 * Impulsions can be detected on falling or rising edge.
 * 
 * pwi 2025- 3-22 creation
 */

#include "pwiPulseSensor.h"

// uncomment to debugging this file
#define SENSOR_DEBUG

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
	this->input_pin = input_pin;
	this->edge = edge;
}

void pwiPulseSensor::init()
{
	// setup
	this->input_pin = 0;
	this->edge = 0;
	// runtime
	this->last_state = 0;
	this->imp_count = 0;
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
 * pwiPulseSensor::getImpulsionsCount():
 * 
 * Public
 */
uint32_t pwiPulseSensor::getImpulsionsCount()
{
    return( this->imp_count );
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
}

/**
 * pwiPulseSensor::loopInput():
 * 
 * Test for a falling/rising edge on the input pin: this is counted as *one* impulsion
 * 
 * Public
 */
void pwiPulseSensor::loopInput()
{
	uint8_t state = digitalRead( this->input_pin );

	bool isEdge = ( this->edge == FALLING && state == LOW && this->last_state == HIGH )
				|| ( this->edge == RISING && state == HIGH && this->last_state == LOW );

	if( isEdge ){
		this->imp_count += 1;
#ifdef SKETCH_DEBUG
		Serial.print( F( "edge detected count=" ));
		Serial.println( this->imp_count );
#endif
	}

	this->last_state = state;
}
