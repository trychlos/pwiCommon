#include "pwiTimer.h"

/*
 * pwi 2017- 5-20 v3 add getRemaining() method
 * pwi 2019- 5-25 v4 remove start() with argument method
 *                   set() is renamed setup()
 * pwi 2019- 5-27 v5 renamed to pwiTimer
 * pwi 2019- 6- 3 v6 improve resetting the timer delay
 * pwi 2019- 8- 4 getDelay() new method
 *                remove untilNow() function
 *                see https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
 */

// uncomment to debugging this file
//#define TIMER_DEBUG

// max count of allocatable pwiTimer's
#define PWITIMER_MAX 15

static pwiTimer *st_timers[PWITIMER_MAX];
static uint8_t   st_count = 0;

/**
 * pwiTimer::pwiTimer:
 * 
 * Constructor.
 */
pwiTimer::pwiTimer( void )
{
    this->delay_ms = 0;
    this->once = true;
    this->cb = NULL;
    this->user_data = NULL;
    this->start_ms = 0;
    this->label = NULL;
    this->debug = true;

    if( st_count == 0 ){
        memset( st_timers, '\0', sizeof( st_timers ));
    }
    if( st_count < PWITIMER_MAX ){
        st_timers[st_count++] = this;
    }
}

/**
 * pwiTimer::getDelay:
 *
 * Returns: the delay configured for this timer.
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
 */
unsigned long pwiTimer::getRemaining( void )
{
    unsigned long now = millis();
    unsigned long duration = now - this->start_ms;
    unsigned long remaining = 0;
    if( this->delay_ms ){
        remaining = this->delay_ms - duration;
    }
#ifdef TIMER_DEBUG
    Serial.print( F( "[pwiTimer::getRemaining] delay_ms=" ));
    Serial.print( this->delay_ms );
    Serial.print( F( ", start_ms=" ));
    Serial.print( this->start_ms );
    Serial.print( F( ", now=" ));
    Serial.print( now );
    Serial.print( F( ", duration=" ));
    Serial.print( duration );
    Serial.print( F( ", remaining=" ));
    Serial.print( remaining );
#endif
    return( remaining );
}

/**
 * pwiTimer::isStarted:
 * 
 * Returns: %TRUE if the timer is started.
 */
bool pwiTimer::isStarted( void )
{
    return( this->start_ms > 0 );
}

/**
 * pwiTimer::restart:
 * 
 * Start the timer,
 * or restart it before its expiration thus reconducting a new delay.
 */
void pwiTimer::restart( void )
{
    this->objStart( true );
}

/**
 * pwiTimer::setDelay:
 * @delay_ms: the duration of the timer.
 *
 * Remarks :
 * 1. if the new delay is zero, the timer is stopped.
 * 2. the timer is restarted with the provided new delay: the callback will so
 *    be next triggered with this same new delay.
 * 
 * Change the configured delay.
 */
void pwiTimer::setDelay( unsigned long delay_ms )
{
    this->delay_ms = delay_ms;

	if( delay_ms == 0 ){
		this->stop();
	} else {
		this->restart();
	}
}

/**
 * pwiTimer::setup:
 * @label: [allow-none]: a label to identify or qualify the timer;
 *  Please note that the method get a copy of the provided pointer, not a copy
 *  of the string itself. The caller should make sure that the provided pointer
 *  will stay safe if use during execution.
 * @delay_ms: the duration of the timer; zero for disable the timer.
 * @once: whether the @cb callback must be called only once, or regularly.
 * @cb: [allow-none]: the callback.
 * @user_data: [allow-none]: the user data to be passed to the callback.
 * @debug: [allow-none]: whether the pwiTimer behavior should be debugged.
 * 
 * Initialize the timer.
 * After having been initialized, the timer may be started.
 * If set, the @cb callback will be called at the expiration of the @delay_ms.
 * 
 * If @once is %TRUE, the timer is then disabled, and will stay inactive
 * until started another time.
 * If @once is %FALSE, the @cb callback will be called regularly each
 * @delay_ms.
 */
void pwiTimer::setup( const char *label, unsigned long delay_ms, bool once, pwiTimerCb cb, void *user_data, bool debug )
{
    if( strlen( label )){
        this->label = label;
    }
    this->delay_ms = delay_ms;
    this->once = once;
    this->cb = cb;
    this->user_data = user_data;
    this->debug = debug;
}

/**
 * pwiTimer::start:
 * 
 * Start a timer.
 * 
 * The pre-set delay must be greater than zero.
 */
void pwiTimer::start( void )
{
    this->objStart( false );
}

/**
 * pwiTimer::stop:
 *
 * Stop a timer.
 */
void pwiTimer::stop( void )
{
    this->start_ms = 0;
}

/**
 * pwiTimer::dump:
 * 
 * Dump.
 * 
 * Public Static
 */
void pwiTimer::Dump( void )
{
#ifdef TIMER_DEBUG
    Serial.print( F( "[pwiTimer::dump]: st_count=" ));
    Serial.print( st_count );
    Serial.print( "/" );
    Serial.println( PWITIMER_MAX );
    for( uint8_t i=0 ; i<st_count ; ++i ){
        st_timers[i]->objDump( i );
    }
#endif
}

/**
 * pwiTimer::Loop:
 * 
 * This function is meant to be called from the main loop.
 * 
 * Public Static
 */
void pwiTimer::Loop( void )
{
    for( uint8_t i=0 ; i<st_count ; ++i ){
        st_timers[i]->objLoop();
    }
}

/**
 * pwiTimer::objDump:
 * 
 * Dump the object.
 * 
 * Private
 */
void pwiTimer::objDump( uint8_t idx )
{
#ifdef TIMER_DEBUG
    Serial.print( F( "[pwiTimer::objDump] idx=" ));
    Serial.print( idx );
    if( strlen( this->label )){
        Serial.print( F( ", label=" ));
        Serial.print( this->label );
    }
    Serial.print( F( ", delay_ms=" ));
    Serial.print( this->delay_ms );
    Serial.print( F( ", once=" ));
    Serial.print( once ? "True":"False" );
    Serial.print( F( ", cb=" ));
    Serial.print(( int ) cb );
    Serial.print( F( ", user_data=" ));
    Serial.print(( int ) user_data );
    Serial.print( F( ", debug=" ));
    Serial.print( debug ? "True":"False" );
    Serial.print( F( ", start_ms=" ));
    Serial.println( this->start_ms );
#endif
}

/**
 * pwiTimer::objLoop:
 * 
 * If the timer is started, and has expired, then call the callback. 
 * Stop the timer if set for running once.
 * 
 * Private
 */
void pwiTimer::objLoop( void )
{
    if( this->start_ms > 0 ){
        unsigned long now = millis();
        unsigned long duration = now - this->start_ms;
        if( duration >= this->delay_ms ){
#ifdef TIMER_DEBUG
            if( this->debug ){
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
            }
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

/**
 * pwiTimer::objStart:
 * @restart: whether the initial request was to start() or restart() the timer.
 * 
 * Start or restart the timer.
 *
 * There is no difference between start() and restart(): in the two cases, the
 * timer is started from now, whatever be its previous state.
 * 
 * Private
 */
void pwiTimer::objStart( bool restart )
{
    if( this->delay_ms ){
        this->start_ms = millis();
        // manage the millis() rollover to make sure start_ms is not zero
        if( !this->start_ms ){
            this->start_ms += 1;
        }
    } else {
        Serial.println( F( "[pwiTimer::objStart] unable to start the timer while delay is not set" ));
    }
}

