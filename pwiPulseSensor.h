#ifndef __PWI_PULSE_SENSOR_H__
#define __PWI_PULSE_SENSOR_H__

/*
 * A Pulse sensor is a sensor which counts impulsions on a given pin.
 * Impulsions are detected at loop time by just reading the pin state.
 * Impulsions can be detected on FALLING or RISING edge.
 * 
 * pwi 2025- 3-22 creation
 * pwi 2025- 7-31 add bounce protection
 */

#include "pwiSensor.h"

class pwiPulseSensor : public pwiSensor {
	public:
					pwiPulseSensor();
                    pwiPulseSensor( uint8_t id, uint8_t input_pin, uint8_t edge );

		/* getters
		 */
		uint8_t		getEdge();
        uint8_t     getInputPin();
		uint32_t    getPulsesCount();

		/* actors
		 */
        bool        loopInput();

		/* setters
		 */
		void        setEdge( uint8_t edge );
		void        setInputPin( uint8_t input_pin );
		void        setPulseLength( uint8_t length_ms );

	private:
        // setup
        uint8_t     input_pin;
		uint8_t 	edge;
        uint8_t     length_ms;

        // runtime
        uint8_t     last_state;
        uint32_t    imp_count;
        uint32_t    last_ms;

		void 		init();
};

#endif // __PWI_PULSE_SENSOR_H__
