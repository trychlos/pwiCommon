#include "pwiTimer.h"
#include <pwiList.h>

/*
 * pwi 2017- 5-20 v3 add getRemaining() method
 * pwi 2019- 5-25 v4 remove start() with argument method
 *                   set() is renamed setup()
 * pwi 2019- 5-27 v5 renamed to pwiTimer
 * pwi 2019- 6- 3 v6 improve resetting the timer delay
 * pwi 2019- 9- 4 getDelay() new method
 *                remove untilNow() function
 *                see https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
 * pwi 2019- 9- 5 use new pwiList class
 */

// uncomment to debugging this file
//#define TIMER_DEBUG

// single linked list of allocated pwiTimer's
static pwiList st_list;

/**
 * pwiTimer::pwiTimer:
 * 
 * Constructor.
 *
 * Public.
 */
pwiTimer::pwiTimer( void )
{
    /* configuration data
     */
    this->label = NULL;
    this->delay_ms = 0;
    this->once = true;
    this->cb = NULL;
    this->user_data = NULL;

    /* runtime data
     */
    this->start_ms = 0;

    /* keep a single linked list of allocated pwiTimer's
     */
    st_list.add( this );
}

/**
 * pwiTimer::dump:
 *
 * Dump the pwiTimer timer.
 *
 * Public.
 */
void pwiTimer::dump( void )
{
#ifdef TIMER_DEBUG
    /* to be written */
#endif
}

/**
 * pwiTimer::getDelay:
 *
 * Returns: the delay configured for this timer.
 *
 * Public.
 */
unsigned long pwiTimer::getDelay( void )
{
    return( this->delay_ms );
}

/**
 * pwiTimer::getRemaining:
 *
 * Returns: zero if the timer is disabled (delay_ms=0), or the remaining count
 * of ms.
 *
 * Public.
 */
unsigned long pwiTimer::getRemaining( void )
{
    unsigned long remaining = 0;
    if( this->isRunnable()){
        unsigned long duration = 0;
        if( this->isStarted()){
            unsigned long now = millis();
            duration = now - this->start_ms;
        }
        remaining = this->delay_ms - duration;
    }
#ifdef TIMER_DEBUG
    Serial.print( F( "[pwiTimer::getRemaining] label=" ));
    Serial.print( this->label );
    Serial.print( F( ", delay_ms=" ));
    Serial.print( this->delay_ms );
    Serial.print( F( ", start_ms=" ));
    Serial.print( this->start_ms );
    Serial.print( F( ", now=" ));
    Serial.print( now );
    Serial.print( F( ", duration=" ));
    Serial.print( duration );
    Serial.print( F( ", remaining=" ));
    Serial.println( remaining );
#endif
    return( remaining );
}

/**
 * pwiTimer::isRunnable:
 * 
 * Returns: %TRUE if the timer can be started.
 *
 * Public.
 */
bool pwiTimer::isRunnable( void )
{
    return( this->delay_ms > 0 );
}

/**
 * pwiTimer::isStarted:
 * 
 * Returns: %TRUE if the timer is started.
 *
 * Public.
 */
bool pwiTimer::isStarted( void )
{
    return( this->start_ms > 0 );
}

/**
 * pwiTimer::restart:
 * 
 * Start the timer,
 *  or restart it before its expiration thus reconducting a new @delay_ms.
 *
 * We get rid of and ignore the question whether we are authorized to restart a
 *  currently running timer.
 *
 * Note: actually restart() is just another name for start().
 *
 * Public.
 */
void pwiTimer::restart( void )
{
    this->start();
}

/**
 * pwiTimer::setDelay:
 * @delay_ms: the duration of the timer.
 *
 * Remarks :
 * 1. if the new delay is zero, the timer is stopped.
 * 2. if already running, then the timer is restarted with the provided new
 *    delay: the callback will so be next triggered with the new delay.
 * 
 * Change the configured delay.
 *
 * Public.
 */
void pwiTimer::setDelay( unsigned long delay_ms )
{
    this->delay_ms = delay_ms;

    if( delay_ms == 0 ){
        this->stop();

    } else if( this->isStarted()){
        unsigned long remaining = this->getRemaining();		// zero if not started
        if( remaining > delay_ms ){
            this->restart();
        }
	}
}

/**
 * pwiTimer::setup:
 * @label: [allow-none]: a label to identify or qualify the timer;
 *  Please note that the method get a copy of the provided pointer, not a copy
 *  of the string itself. The caller should make sure that the provided pointer
 *  will stay safe during execution.
 * @delay_ms: the duration of the timer; zero for disable the timer.
 * @once: whether the @cb callback must be called only once, or regularly.
 *  On %TRUE, the timer will be automatically restarted on return of the callback.
 * @cb: [allow-none]: the callback.
 * @user_data: [allow-none]: the user data to be passed to the callback.
 * 
 * Initialize the timer.
 * After having been initialized, the timer may be started.
 * If set, the @cb callback will be called at the expiration of the @delay_ms.
 * 
 * If @once is %TRUE, the timer is then disabled, and will stay inactive
 * until started another time.
 * If @once is %FALSE, the @cb callback will be called regularly each
 * @delay_ms.
 *
 * Public.
 */
void pwiTimer::setup( const char *label, unsigned long delay_ms, bool once, pwiTimerCb cb, void *user_data )
{
    this->label = label;
    this->setDelay( delay_ms );
    this->once = once;
    this->cb = cb;
    this->user_data = user_data;
}

/**
 * pwiTimer::start:
 * 
 * Start/restart a timer.
 * 
 * The pre-set @delay_ms must be greater than zero.
 *
 * Public.
 */
void pwiTimer::start( void )
{
    if( this->isRunnable()){
        this->start_ms = millis();
        // manage the millis() rollover to make sure start_ms is not zero
        if( this->start_ms == 0 ){
            this->start_ms += 1;
        }
    } else {
#ifdef TIMER_DEBUG
        Serial.print( F( "[pwiTimer::start] label=" ));
        Serial.print( this->label );
        Serial.println( F( ": unable to start the timer while delay is not set" ));
#endif
        this->stop();
    }
}

/**
 * pwiTimer::stop:
 *
 * Stop a timer.
 *
 * Public.
 */
void pwiTimer::stop( void )
{
    this->start_ms = 0;
}

/**
 * pwiTimer::Dump:
 * 
 * Dump all registered pwiTimer's.
 * 
 * Public Static.
 */
void pwiTimer::Dump( void )
{
    st_list.iter( pwiTimer::DumpCb );
}

/**
 * pwiTimer::Loop:
 * 
 * This function is meant to be repeatedly called from the main loop.
 * 
 * Public Static.
 */
void pwiTimer::Loop( void )
{
    st_list.iter( pwiTimer::LoopCb );
}

/**
 * pwiTimer::DumpCb:
 * @timer: the to-be-dumped pwiTimer.
 * 
 * pwiList::iter() callback function: dump the pwiTimer element.
 * 
 * Private Static.
 */
void pwiTimer::DumpCb( pwiTimer *timer, void *user_data )
{
    timer->dump();
}

/**
 * pwiTimer::LoopCb:
 * 
 * pwiList::iter() callback function: check the pwiTimer element for expiration
 *  of the @delay_ms.
 * 
 * Private Static.
 */
void pwiTimer::LoopCb( pwiTimer *timer, void *user_data )
{
    timer->loop();
}

/**
 * pwiTimer::loop:
 * 
 * Check the pwiTimer element for expiration of the @delay_ms.
 * 
 * Private.
 */
void pwiTimer::loop( void )
{
    if( this->isStarted()){
        unsigned long now = millis();
        unsigned long duration = now - this->start_ms;
        if( duration >= this->delay_ms ){
#ifdef TIMER_DEBUG
            Serial.print( F( "[pwiTimer::objLoop] " ));
            if( strlen( this->label )){
                Serial.print( F( "label=" ));
                Serial.print( this->label );
                Serial.print( ", " );
            }
            Serial.print( F( "delay_ms=" ));
            Serial.print( this->delay_ms );
            Serial.print( F( ", start_ms=" ));
            Serial.print( this->start_ms );
            Serial.print( F( ", duration=" ));
            Serial.println( duration );
#endif
            if( this->cb ){
                this->cb( this->user_data );
            }
            if( this->once ){
                this->stop();
            } else {
                this->restart();
            }
        }
    }
}

