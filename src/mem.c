/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Envy Diku Mud improvements copyright (C) 1994 by Michael Quan, David   *
 *  Love, Guilherme 'Willie' Arnold, and Mitchell Tse.                     *
 *                                                                         *
 *  EnvyMud 2.0 improvements copyright (C) 1995 by Michael Quan and        *
 *  Mitchell Tse.                                                          *
 *                                                                         *
 *  EnvyMud 2.2 improvements copyright (C) 1996, 1997 by Michael Quan.     *
 *                                                                         *
 *  GreedMud 0.99.3 improvements copyright (C) 1997, 1998, 1999            *
 *  by Vasco Costa.                                                        *
 *                                                                         *
 *  In order to use any part of this Envy Diku Mud, you must comply with   *
 *  the original Diku license in 'license.doc', the Merc license in        *
 *  'license.txt', as well as the Envy license in 'license.nvy'.           *
 *  In particular, you may not remove either of these copyright notices.   *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



/*
 * Globals
 */
EXTRA_DESCR_DATA *	extra_descr_free;
HHF_DATA *		hhf_free;


void free_extra_descr( EXTRA_DESCR_DATA *pExtra )
{
    free_string( pExtra->keyword );
    free_string( pExtra->description );

    pExtra->next	= extra_descr_free;
    extra_descr_free	= pExtra;
    return;
}


void free_affect( AFFECT_DATA *pAf )
{
    pAf->next	= affect_free;
    affect_free	= pAf;
    return;
}


HHF_DATA *new_hhf_data( void )
{
    HHF_DATA *pHhf;

    if ( !hhf_free )
    {
	pHhf = (HHF_DATA *) alloc_perm( sizeof( *pHhf ) );
	top_hhf++;
    }
    else
    {
	pHhf		= hhf_free;
	hhf_free	= hhf_free->next;
    }

    pHhf->next		= NULL;
    pHhf->name		= NULL;
    pHhf->who		= NULL;

    return pHhf;
}


void free_hhf_data( HHF_DATA *pHhf )
{
    free_string( pHhf->name );

    pHhf->next		= hhf_free;
    hhf_free		= pHhf;
    return;
}
