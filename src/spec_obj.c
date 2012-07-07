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
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



struct vehicle_path_type       path_table      [ ]     =
{
       {
	       "Mayor's office to Mayor's office",
	       "33003330001110111333333222221122121111.",
	       "33003330001110111333333222221122121111.",
	       6,
	       20
       }
};
 
OBJ_FUN *spec_obj_lookup( const char *name )
{
    int cmd;

    for ( cmd = 0; *spec_obj_table[cmd].spec_name; cmd++ )
        if ( !str_cmp( name, spec_obj_table[cmd].spec_name ) )
            return spec_obj_table[cmd].spec_fun;

    return 0;
}


char *spec_obj_string( OBJ_FUN *fun )
{
    int cmd;

    for ( cmd = 0; *spec_obj_table[cmd].spec_fun; cmd++ )
        if ( fun == spec_obj_table[cmd].spec_fun )
            return spec_obj_table[cmd].spec_name;

    return 0;
}

/*
 * Object special function commands.
 */
const	struct	spec_obj_type	spec_obj_table	[ ]	=
{
    { "spec_giggle",		spec_giggle		},
    { "spec_soul_moan",		spec_soul_moan		},
    { "",			0			}
};  



/*
 * Special procedures for objects.
 */
bool spec_giggle( OBJ_DATA *obj, CHAR_DATA *keeper )
{
    if ( !keeper || !keeper->in_room )
	return FALSE;

    if ( number_percent( ) < 5 )
    {
	act( "$p carried by $n starts giggling to itself!",
	    keeper, obj, NULL, TO_ROOM );
	act( "$p carried by you starts giggling to itself!",
	    keeper, obj, NULL, TO_CHAR );
	return TRUE;
    }

    return FALSE;
}


bool spec_soul_moan( OBJ_DATA *obj, CHAR_DATA *keeper )
{
    if ( !keeper || !keeper->in_room )
	return FALSE;

    if ( number_percent( ) < 2 )
    {
	act( "The soul in $p carried by $n moans in agony.",
	    keeper, obj, NULL, TO_ROOM );
	act( "The soul in $p carried by you moans to be set free!",
	    keeper, obj, NULL, TO_CHAR );
	return TRUE;
    }

    if ( number_percent( ) < 2 )
    {
	act( "The soul in $p carried by $n tries to free itself!",
	    keeper, obj, NULL, TO_ROOM );
	act( "The soul in $p carried by you starts writhing!",
	    keeper, obj, NULL, TO_CHAR );
	return TRUE;
    }

    return FALSE;
}


/*
 * By Aioros.
 */
bool spec_vehicle( OBJ_DATA *obj, CHAR_DATA *keeper )
{
    char *path;
    int *pos;
    int *move;
    int *dhour1;
    int *dhour2;
    int *moved;

    if ( obj->item_type != ITEM_VEHICLE )
      	return FALSE;
    
    pos     = &obj->value[4];
    move    = &obj->value[3];
    dhour1  = &path_table[obj->value[0]].hour1;
    dhour2  = &path_table[obj->value[0]].hour2;
    moved   = &obj->value[2];

    if ( !*move )
    {
    	if ( *pos >= 0 && 
    	    ( time_info.hour == *dhour1 - 1 || time_info.hour == *dhour2 - 1 ) )
    	{
    	    send_to_room( "The driver says: 'We will depart in one hour.'",
    	    	    	 obj->in_room );
    	    send_to_room( "The driver says: 'We will depart in one hour.'",
    	    	    	 get_room_index( obj->value[1] ) );
    	    *pos = -1;
    	}

    	if ( time_info.hour == *dhour1 && *moved == 0 )
    	{
    	    send_to_room( "You feel the ground shake as you start moving.",
    	    	    	 get_room_index( obj->value[1] ) );
    	    *moved  = 1;
    	    *move   = 1;
    	    *pos    = 0;
    	}

    	if ( time_info.hour == *dhour2 && *moved == 1 )
    	{
    	    send_to_room( "You feel the ground shake as you start moving.",
    	    	    	 get_room_index( obj->value[1] ) );
    	    *moved  = 0;
    	    *move   = 2;
    	    *pos    = 0;
    	}
    }

    if ( !*move )
    	return FALSE;

    if ( *move == 1 )
    	path = &path_table[obj->value[0]].path1[0];
    else
    	path = &path_table[obj->value[0]].path2[0];

    switch ( path[*pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
    	move_obj( obj, path[*pos] - '0' );
    	break;

    case '.' :
    	*move = 0;
    	send_to_room( "The driver says: 'We have arrived.'",
    	    	     obj->in_room );
    	send_to_room( "The driver says: 'We have arrived.'",
    	    	     get_room_index( obj->value[1] ) );
    	break;
    }

    ++*pos;
    return FALSE;
}
