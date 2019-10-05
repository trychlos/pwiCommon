#ifndef __PWI_COMMON_H__
#define __PWI_COMMON_H__

/*
 * pwi 2019-10- 5 v191001 creation
 */

/*
 * The PGMSTR macro let us use globally defined PROGMEM strings in the code
 * Typical use:
 * 	static char const myString[] PROGMEM = "This is a globally defined constant string";
 *  sendSketchInfo( PGMSTR( myString ));
 */
#ifndef PGMSTR
#include <drivers/Linux/Arduino.h>
#define PGMSTR(x) (( const __FlashStringHelper *) x)
#endif

#endif // __PWI_COMMON_H__

