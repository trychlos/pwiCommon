#include <Arduino.h>
#include "toHex.h"

/*
 * pwi 2019- 9-12 v190902
 *                creation
 */

// uncomment to debugging this file
//#define PRINT_ADDRESS_DEBUG

static char sHexa[7];		// up to 64 KB address

/**
 * toHex16:
 * @ptr: a pointer to any object.
 *
 * Returns: the @ptr address as an hexadecimal, '0x'-prefixed, string.
 */
char *toHex16( void *ptr )
{
    snprintf( sHexa, sizeof( sHexa ), "0x%4.4x", ( int ) ptr );
    return( sHexa );
}

