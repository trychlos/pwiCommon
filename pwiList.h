#ifndef __PWI_LIST_H__
#define __PWI_LIST_H__

#include <Arduino.h>

/*
 * pwiList
 *
 * Implement a simple linked list.
 *
 * Synopsys:
 * a) just define the initial pointer to the list
 *    pwiList myList;
 * b) add elements as needed
 *    myList.add( myElement );
 * c) iter through the list
 *    myList.iter( cb, user_data );
 *
 * Note: not using standard Arduino LinkedList class
 *  (https://www.arduinolibraries.info/libraries/linked-list)
 *  because of the 8.2 KB size.
 *
 * Note: the class does not provide any free/remove primitive in order to keep
 *  it as simple as possible.
 *
 * pwi 2019- 9- 5 creation
 */

/* The definition of the callback function to be provided on list iteration.
 */
typedef void ( pwiListIterCb )( void *element, void *user_data );

class pwiList {
    public:
                 pwiList( void );
        void     add( void *element );
        void     iter( pwiListIterCb cb, void* user_data=NULL );

    private:
        void    *data;
        void    *next;

        pwiList *last();
};

#endif // __PWI_LIST_H__

