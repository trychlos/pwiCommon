#include "pwiList.h"

/*
 * pwi 2019- 9- 5 creation
 */

/**
 * pwiList::pwiList:
 *
 * Constructor.
 */
pwiList::pwiList( void )
{
    data = NULL;
    next = NULL;
}

/**
 * pwiList::add:
 * @element: the element to be added to the list.
 *
 * Add a new item to the list.
 *
 * Public.
 */
void pwiList::add( void *element )
{
    // relevant items in the list are those which hold a non-null 'data' member.
    // taking this into account is needed because the list may be statically
    // allocated.
    if( !this->data ){
        this->data = element;
    } else {
        pwiList *last = this->last();
        last->next = new pwiList;
        (( pwiList * ) last->next )->data = element;
    }
}

/**
 * pwiList::iter:
 * @cb: the callback function to be called foreach element of the list.
 * @user_data: user-provided data to be passed to the @cb callback.
 *
 * Calls the @cb callback function for each item which have a non-null 'data'
 *  member (which may be zero).
 *
 * Public.
 */
void pwiList::iter( pwiListIterCb cb, void* user_data )
{
    if( cb ){
        if( this->data ){
            cb( this->data, user_data );
        }
        if( this->next ){
            (( pwiList * ) this->next )->iter( cb, user_data );
        }
    }
}

/*
 * pwiList::last:
 *
 * Returns: the last item of the list (which may be this one).
 *
 * Private.
 */
pwiList *pwiList::last()
{
    if( !this->next ){
        return( this );
    }
    return((( pwiList * ) this->next )->last());
}

