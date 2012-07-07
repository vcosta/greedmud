/***************************************************************************
 *  GreedMud 0.99.7 improvements copyright (C) 1997-2001                   *
 *  by Vasco Costa.                                                        *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



/*
 * Structure types.
 */
typedef	struct	bfs_queue		BFS_DATA;

/*
 * Hunting parameters.
 */
#define BFS_ERROR		-1
#define BFS_ALREADY_THERE	-2
#define BFS_NO_PATH		-3

#define MARK_ROOM( room )	( SET_BIT   ((room)->room_flags, ROOM_MARKED) )
#define UNMARK_ROOM( room )	( REMOVE_BIT((room)->room_flags, ROOM_MARKED) )
#define IS_MARKED_ROOM( room )	( IS_SET    ((room)->room_flags, ROOM_MARKED) )


struct	bfs_queue
{
    ROOM_INDEX_DATA *	room;
    int			dir;
};


static BFS_DATA *	queue;
static int		nqueue;



void bfs_enqueue(ROOM_INDEX_DATA *room, int dir)
{
  MARK_ROOM(room);
  queue 	      = (BFS_DATA *)realloc(queue, (nqueue+1)*sizeof(BFS_DATA));
  queue[nqueue].room  = room;
  queue[nqueue].dir   = dir;
  nqueue++;
}

void bfs_cleanup(void)
{
  int i;

  for (i=0; i<nqueue; i++)
    UNMARK_ROOM(queue[i].room);
  free(queue);
}


int find_path( ROOM_INDEX_DATA *src, ROOM_INDEX_DATA *dst, int maxdist )
{
  int count;
  int k;
  int door;

  if ( !src || !dst )
  {
    bug( "Illegal value passed to find_first_step (hunt.c)", 0 );
    return BFS_ERROR;
  }

  if ( src == dst )
    return BFS_ALREADY_THERE;

  queue		= NULL;
  nqueue	= 0;

  MARK_ROOM(src);

  /* first, enqueue the first steps, saving which direction we're going. */
  for ( door = 0; door < MAX_DIR; door++ )
  {
    ROOM_INDEX_DATA * to_room;
    EXIT_DATA *	      pexit;

    pexit = src->exit[door];
    if ( pexit && (to_room=pexit->to_room) && !IS_MARKED_ROOM(to_room) )
      bfs_enqueue(to_room, door);
  }

  count = 0;
  k	= 0;
  while ( nqueue > k )
  {
    ROOM_INDEX_DATA * room;
    int 	      dir;

    if ( ++count > maxdist )
    {
      bfs_cleanup();
      UNMARK_ROOM(src);
      return BFS_NO_PATH;
    }

    room = queue[k].room;
    dir  = queue[k].dir;

    if ( room == dst )
    {
      bfs_cleanup();
      UNMARK_ROOM(src);
      return dir;
    }
    else
    {
      k++;

      for ( door = 0; door < MAX_DIR; door++ )
      {
        ROOM_INDEX_DATA * to_room;
        EXIT_DATA *	  pexit;

        pexit = room->exit[door];
        if ( pexit && (to_room=pexit->to_room) && !IS_MARKED_ROOM(to_room) )
	  bfs_enqueue(to_room, dir);
      }
    }
  }
  bfs_cleanup();
  UNMARK_ROOM(src);
  return BFS_NO_PATH;
}


void found_prey( CHAR_DATA *ch, CHAR_DATA *victim )
{
     char buf		[ MAX_STRING_LENGTH ];
     char victname	[ MAX_STRING_LENGTH ];

     if ( !victim || victim->deleted )
     {
	bug( "Found_prey: null victim", 0 );
	return;
     }

     if ( !victim->in_room )
     {
        bug( "Found_prey: null victim->in_room", 0 );
        return;
     }

     sprintf( victname, IS_NPC( victim ) ? victim->short_descr : victim->name );

     if ( !can_see( ch, victim ) )
     {
        if ( number_percent( ) < 90 )
	  return;
	switch( number_bits( 2 ) )
 	{
	case 0: sprintf( buf, "Don't make me find you, %s!", victname );
		do_say( ch, buf );
	        break;
	case 1: act( "$n sniffs around the room for $N.", ch, NULL, victim, TO_NOTVICT );
		act( "You sniff around the room for $N.", ch, NULL, victim, TO_CHAR );
		act( "$n sniffs around the room for you.", ch, NULL, victim, TO_VICT );
		sprintf( buf, "I can smell your blood!" );
		do_say( ch, buf );
		break;
	case 2: sprintf( buf, "I'm going to tear %s apart!", victname );
		do_yell( ch, buf );
		break;
	case 3: do_say( ch, "Just wait until I find you...");
		break;
        }
	return;
     }

     if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
     {
	if ( number_percent( ) < 90 )
	  return;
	switch( number_bits( 2 ) )
	{
	case 0:	do_say( ch, "C'mon out, you coward!" );
		sprintf( buf, "%s is a bloody coward!", victname );
		do_yell( ch, buf );
		break;
	case 1: sprintf( buf, "Let's take this outside, %s", victname );
		do_say( ch, buf );
		break;
	case 2: sprintf( buf, "%s is a yellow-bellied wimp!", victname );
		do_yell( ch, buf );
		break;
	case 3: act( "$n takes a few swipes at $N.", ch, NULL, victim, TO_NOTVICT );
		act( "You try to take a few swipes $N.", ch, NULL, victim, TO_CHAR );
		act( "$n takes a few swipes at you.", ch, NULL, victim, TO_VICT );
		break;
	}
	return;
     }

     switch( number_bits( 2 ) )
     {
     case 0: sprintf( buf, "Your blood is mine, %s!", victname );
	     do_yell( ch, buf);
	     break;
     case 1: sprintf( buf, "Alas, we meet again, %s!", victname );
     	     do_say( ch, buf );
     	     break;
     case 2: sprintf( buf, "What do you want on your tombstone, %s?", victname );
     	     do_say( ch, buf );
     	     break;
     case 3: act( "$n lunges at $N from out of nowhere!", ch, NULL, victim, TO_NOTVICT );
	     act( "You lunge at $N catching $M off guard!", ch, NULL, victim, TO_CHAR );
	     act( "$n lunges at you from out of nowhere!", ch, NULL, victim, TO_VICT );
     }
     stop_hunting( ch );
     multi_hit( ch, victim, TYPE_UNDEFINED );
     return;
} 



void do_track( CHAR_DATA *ch, char *argument )
{
    char       buf [ MAX_STRING_LENGTH ];
    char       arg [ MAX_STRING_LENGTH ];
    CHAR_DATA *victim;
    int        dir;
    bool       fArea;

    if ( !can_use( ch, gsn_track ) )
    {
	send_to_char( "You do not know of this skill yet.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Whom are you trying to track?\n\r", ch );
	return;
    }

    if ( ch->riding )
    {
        send_to_char( "You can't sniff a trail mounted.\n\r", ch );
        return;
    }

    /* only imps can hunt to different areas */
    fArea = ( get_trust( ch ) < L_DIR );

    if ( fArea )
	victim = get_char_area( ch, arg );
    else
	victim = get_char_world( ch, arg );

    if ( !victim )
    {
	send_to_char( "You can't find a trail of anyone like that.\n\r", ch );
	return;
    }

    if ( ch->in_room == victim->in_room )
    {
	act( "You're already in $N's room!", ch, NULL, victim, TO_CHAR );
	return;
    }

    /*
     * Deduct some movement.
     */
    if ( ch->move > 2 )
	ch->move -= 3;
    else
    {
	send_to_char( "You're too exhausted to hunt anyone!\n\r", ch );
	return;
    }

    act( "$n carefully sniffs the air.", ch, NULL, NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_track].beats );

    /*
     * Give a random direction if the player misses the die roll.
     */
    if (   (  IS_NPC( ch ) && number_percent( ) > 75 )	/* NPC @ 25% */
	|| ( !IS_NPC( ch ) && number_percent( ) >	/* PC @ norm */
		ch->pcdata->learned[gsn_track] ) )
    {
	do
	{
	    dir = number_door( );
	}
	while (   !( ch->in_room->exit[dir] )
	       || !( ch->in_room->exit[dir]->to_room ) );

	learn( ch, gsn_track, FALSE );
    }
    else
    {
        dir = find_path( ch->in_room, victim->in_room, 100 + ch->level * 30 );

        if ( dir < 0 )
        {
	    act( "You can't sense $N's trail from here.",
		ch, NULL, victim, TO_CHAR );
	    return;
	}

	learn( ch, gsn_track, TRUE );
    }

    /*
     * Display the results of the search.
     */
    sprintf( buf, "You sense $N's trail %s from here...", dir_name[dir] );
    act( buf, ch, NULL, victim, TO_CHAR );
    return;
}



void hunt_victim( CHAR_DATA *ch )
{
    CHAR_DATA *tmp;
    int        dir;
    bool       found;

    if ( !ch || ch->deleted || !ch->hunting )
	return;

    /*
     * Make sure the victim still exists.
     */
    found = FALSE;
    for ( tmp = char_list; tmp; tmp = tmp->next )
	if ( ch->hunting->who == tmp )
	{
	    found = TRUE;
	    break;
	}

    if ( !found )
    {
	do_say( ch, "Damn!  My prey is gone!!" );
	stop_hunting( ch );
	return;
    }

    if ( ch->in_room == ch->hunting->who->in_room )
    {
	if ( ch->fighting )
	    return;
	found_prey( ch, ch->hunting->who );
	return;
    }

    /*
     * Give a random direction if the mob misses the die roll.
     */
    if ( number_percent( ) > 75 )	/* @ 25% */
    {
	do
	{
	    dir = number_door( );
	}
	while (   !( ch->in_room->exit[dir] )
	       || !( ch->in_room->exit[dir]->to_room ) );
    }
    else
    {
	dir = find_path( ch->in_room, ch->hunting->who->in_room,
			500 + ch->level * 25 );

	if ( dir < 0 )
	{
	    act( "$n says 'Damn!  Lost $M!'", ch, NULL, ch->hunting->who,
	        TO_ROOM );
	    stop_hunting( ch );
	    return;
	}
    }
    
    {
	EXIT_DATA *pexit;

	pexit = ch->in_room->exit[dir];

	if ( IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
	    || ( xIS_SET( ch->act, ACT_STAY_AREA )
		&& pexit->to_room->area != ch->in_room->area ) )
	{
	    stop_hunting( ch );
	    return;
	}

	if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
	{
	    do_open( ch, dir_name[dir] );
	    return;
	}
    }

    move_char( ch, dir );
    
    if ( ch->deleted )
    {
	return;
    }

    if ( !ch->hunting )
    {
	if ( !ch->in_room )
	{
	    char buf [ MAX_STRING_LENGTH ];

	    sprintf( buf, "Hunt_victim: no ch->in_room!  Mob #%d, name: %s.  Placing mob in limbo.",
		    ch->pIndexData->vnum, ch->name );
	    bug( buf, 0 );
	    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
	    return;
	} 
	do_say( ch, "Damn!  Lost my prey!" );
	return;
    }
    if ( ch->in_room == ch->hunting->who->in_room )
	found_prey( ch, ch->hunting->who );
    return;
}
