#ifndef __PWI_TIMER2_H__
#define __PWI_TIMER2_H__

#include <pwiTimer.h>

/*
 * This is an interrupt-driven timer which relies on :
 * - MsTimer2 namespace code
 * - pwiTimer architecture.
 * 
 * As a MsTimer2, it makes use of Timer2 Arduino hardware timer, programmed
 * for milliseconds resolution.
 * 
 * Several pwiTimer2 timers can be defined, provided they all be reduced to a
 * common divisor delay. For a better precision, please choose a resolution
 * which is half of the highest common factor.
 * E.g., if we want a 3s timer and a 0,1s timer, just set the common resolution
 * to 50ms.
 * 
 * pwiTimer2 takes advantage of the pwiTimer architecture as a base class.
 *
 * pwi 2019-10-13 creation
 */

class pwiTimer2 : public pwiTimer {
    public:
                                    pwiTimer2( void );
        virtual   const char       *getType();

        /* static methods
         */
        static    void              setResolution( unsigned long delay_ms );

    private:
        /* configuration data
         */

        /* static data
         */
        static    bool              initialized;
        static    unsigned long     resolution_ms;
        static    const char       *className;

        /* static methods
         */
        static    void              init( void );
        static    void              timer_isr( void );
};

#endif // __PWI_TIMER2_H__

