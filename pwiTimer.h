#ifndef __PWI_TIMER_H__
#define __PWI_TIMER_H__

#include <Arduino.h>

/*
 * This is a very simple timer which let us trigger a function at the end
 * of a predefined delay.
 * Contrarily to other standard real timers, this one does not rely on any
 * of the internal Arduino timers, nor on any interrupts. Instead this one
 * relies on being called repeatidly on each loop.
 *
 * The timer is caracterized by the delay, the callback and the user_data.
 *
 * Do not expect a great precision here. In best case, it will be about 1-2 ms.
 * The precision may decrease until 10-20 ms if the main loop is too long.
 *
 * Synopsys:
 * a) define a new timer:
 *    pwiTimer myTimer;
 * b) configure the timer:
 *    myTimer.setup( label, delay_ms, once, cb, user_data );
 * c) start the timer:
 *    myTimer.start();
 *
 * At end of the predefined delay, the callback will be called with
 * the passed-in user data.
 *
 * This simplissim timer relies on being repeatedly called by the main loop.
 *
 * Note: the class does not provide any free/remove primitive in order to keep
 *  it as simple as possible.
 *
 * pwi 2017- 5-20 v3 add getRemaining() method
 * pwi 2019- 5-25 v4 remove start() with argument method
 *                   set() is renamed setup()
 * pwi 2019- 5-27 v5 renamed to pwiTimer
 * pwi 2019- 6- 3 v6 improve resetting the timer delay
 * pwi 2019- 9- 4 getDelay() new method
 */

/* The prototype for the timer callback function to be provided by the caller.
   This function receives the 'user_data' parameter provided at setup() time.
   No return value is expected.
 */
typedef void ( *pwiTimerCb )( void * );

class pwiTimer {
    public:
                                  pwiTimer( void );
        virtual void              dump( void );
        virtual unsigned long     getDelay();
        virtual unsigned long     getRemaining();
        virtual bool              isRunnable();
        virtual bool              isStarted();
        virtual void              restart( void );
        virtual void              setDelay( unsigned long delay_ms );
        virtual void              setup( const char *label, unsigned long delay_ms, bool once=true, pwiTimerCb cb=NULL, void *user_data=NULL );
        virtual void              start( void );
        virtual void              stop( void );

        static  void              Dump();
        static  void              Loop();

    private:
        /* configuration data
         * see setup()
         */
                const char       *label;
                unsigned long     delay_ms;
                bool              once;
                pwiTimerCb        cb;
                void             *user_data;

        /* runtime data
         * @start_ms: startup timestamp.
         *  =0 timer not started
         *  >0 timestamp of the timer startup.
		 */
                unsigned long     start_ms;

        static  void              DumpCb( pwiTimer *timer, void *user_data );
        static  void              LoopCb( pwiTimer *timer, void *user_data );

                void              loop();
};

#endif // __PWI_TIMER_H__

