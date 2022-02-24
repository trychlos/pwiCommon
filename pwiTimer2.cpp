#include "pwiTimer2.h"

#include <MsTimer2.h>
#include <pwiList.h>

/*
 * pwi 2019-10-13 creation
 */

// uncomment to debugging this file
//#define TIMER2_DEBUG

static bool          pwiTimer2::initialized = false;
static unsigned long pwiTimer2::resolution_ms = 50;
static const char   *pwiTimer2::className = "pwiTimer2";

/**
 * pwiTimer2::pwiTimer2:
 * 
 * Constructor.
 *
 * Public.
 */
pwiTimer2::pwiTimer2( void ) : pwiTimer()
{
    pwiTimer2::init();
}

/**
 * pwiTimer2::getType:
 *
 * Returns: the object type name (aka the class label).
 *
 * Rationale: cf. pwiTimer.cpp code.
 *
 * Public.
 */
const char *pwiTimer2::getType( void )
{
	return( pwiTimer2::className );
}

/**
 * pwiTimer2::setResolution:
 * @delay_ms: the common resolution to all pwiTimer2 used in the program.
 *  This should be the highest common factor (HCF) of our timers.
 * 
 * Public Static.
 */
void pwiTimer2::setResolution( unsigned long delay_ms )
{
    pwiTimer2::resolution_ms = delay_ms;
    MsTimer2::set( pwiTimer2::resolution_ms, pwiTimer2::timer_isr );
    MsTimer2::start();
}

/**
 * pwiTimer2::init:
 * 
 * Initialize the MsTimer2 Arduino hardware timer timer/counter.
 * 
 * Private Static.
 */
void pwiTimer2::init( void )
{
    if( !pwiTimer2::initialized ){
        pwiTimer2::setResolution( pwiTimer2::resolution_ms );
        pwiTimer2::initialized = true;
    }
}

/**
 * pwiTimer2::timer_isr:
 * 
 * Interrupt Service Routine (ISR) triggered on MsTimer2 timer/counter overflow.
 * We can rely on the fact that this ISR is exactly triggered every resolution_ms
 * period.
 * 
 * Private Static.
 */
void pwiTimer2::timer_isr( void )
{
    pwiTimer::Loop( pwiTimer2::className );
}

