/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
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
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



/*
 * Local functions.
 */
void	talk_channel	args( ( CHAR_DATA *ch, char *argument,
			    int channel, const char *verb ) );



/*
 * How to make a string look drunk by Apex <robink@htsa.hva.nl>
 * Modified and enhanced by Maniac from Mythran
 */
char *makedrunk( char *s, CHAR_DATA *ch )
{
    char buf  [ MAX_STRING_LENGTH ];
    int  i;
    int  drunklevel = 0;
    int  pos        = 0;

    /* Check how drunk a person is... */
    if ( !IS_NPC( ch ) && 
	( drunklevel = ch->pcdata->condition[ COND_DRUNK ] ) > 0 )
    {
	while ( *s )
	{
	    char tmp;
	    int  fuzzy;

	    tmp = UPPER( *s );
	    i   = tmp - 'A';

	    if ( tmp >= 'A' && tmp <= 'Z' )
	    {
	    	if ( drunklevel > drunk[i].min_drunk_level )
	    	{
	    	    fuzzy = number_range( 0, drunk[i].number_of_rep );
	    	    strcpy( &buf[pos], drunk[i].replacement[fuzzy] );
	    	    pos  += strlen( drunk[i].replacement[fuzzy] );
	    	}
	    	else
		{
	    	    buf[pos++] = *s;
		}
	    }
	    else
	    {
	    	if ( tmp >= '0' && tmp <= '9' )
	    	    buf[pos++] = '0' + number_range( 0, 9 );
	    	else
	    	    buf[pos++] = *s;
	    }

	    s++;
	}
	buf[pos]		= '\0';
	buf[MAX_INPUT_LENGTH-1]	= '\0';
	strcpy( s, buf );
    }

    return s;
}

/*
 * Generic channel function.
 */
void talk_channel( CHAR_DATA *ch, char *argument, int channel,
		  const char *verb )
{
    DESCRIPTOR_DATA *d;
    char            *lingo;
    char            *speech;
    char             buf  [ MAX_STRING_LENGTH ];
    char             buf1 [ MAX_INPUT_LENGTH  ];
    int              position;

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "%s what?\n\r", verb );
	buf[0] = UPPER( buf[0] );
	return;
    }

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SILENCE ) )
    {
	sprintf( buf, "You can't %s.\n\r", verb );
	send_to_char( buf, ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_MUTE )
        || IS_SET( race_table[ch->race].race_abilities, RACE_MUTE )
        || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
    {
        send_to_char( "Your lips move but no sound comes out.\n\r", ch );
        return;
    }

    REMOVE_BIT( ch->deaf, channel );

    switch ( channel )
    {
    default:
	sprintf( buf, "You %s '%s'\n\r", verb, argument );
	send_to_char( buf, ch );
	sprintf( buf, "$n %ss '$t'",     verb );
	break;
 
     case CHANNEL_MUSIC:
 	sprintf( buf, "{yYou %s '%s{x{y'{x\n\r", verb, argument );
 	send_to_char( buf, ch );
 	sprintf( buf, "{y$n %ss '$t{x{y'{x",     verb );
 	break;
 
     case CHANNEL_QUESTION:
 	sprintf( buf, "{yYou %s '%s{x{y'{x\n\r", verb, argument );
 	send_to_char( buf, ch );
 	sprintf( buf, "{y$n %ss '$t{x{y'{x",     verb );
 	break;
 
     case CHANNEL_SHOUT:
 	sprintf( buf, "{rYou %s '%s{x{r'{x\n\r", verb, argument );
 	send_to_char( buf, ch );
 	sprintf( buf, "{r$n %ss '$t{x{r'{x",     verb );
 	break;
 
     case CHANNEL_YELL:
 	sprintf( buf, "{bYou %s '%s{x{b'{x\n\r", verb, argument );
 	send_to_char( buf, ch );
 	sprintf( buf, "{b$n %ss '$t{x{b'{x",     verb );
 	break;
 
     case CHANNEL_CHAT:
 	sprintf( buf, "{mYou %s '%s{x{m'{x\n\r", verb, argument );
 	send_to_char( buf, ch );
 	sprintf( buf, "{m$n %ss '$t{x{m'{x",     verb );
 	break;
 
     case CHANNEL_GRATS:
 	sprintf( buf, "{yYou %s '%s{x{y'{x\n\r", verb, argument );
 	send_to_char( buf, ch );
 	sprintf( buf, "{y$n %ss '$t{x{y'{x",     verb );
 	break;
 
     case CHANNEL_CLANTALK:
 	sprintf( buf, "{o{y[{r$n{y]: $t{x" );
	act( buf, ch, argument, NULL, TO_CHAR );
 	break;

    case CHANNEL_IMMTALK:
 	sprintf( buf, "{c[{y$n{c]: $t{x" );
	position	= ch->position;
	ch->position	= POS_STANDING;
	act( buf, ch, argument, NULL, TO_CHAR );
	ch->position	= position;
	break;
    }

    /*
     * Make the words look drunk if needed...
     */
    strcpy( buf1, argument );

    argument = makedrunk( argument, ch );

    for ( d = descriptor_list; d; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ( d->connected == CON_PLAYING
	    && vch != ch
	    && !IS_SET( och->deaf, channel )
            && !IS_SET( och->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
	{
	    if ( channel == CHANNEL_IMMTALK && !IS_HERO( och ) )
		continue;
	    if ( channel == CHANNEL_CLANTALK
	        && ( !is_clan( och ) || !is_same_clan( ch, och ) ) )
		continue;
	    if ( channel == CHANNEL_YELL
		&& vch->in_room->area != ch->in_room->area )
	        continue;

	    position		= vch->position;
	    if ( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL )
		vch->position	= POS_STANDING;

      	    if ( ch->speaking == 0 )
	    {
	        speech = argument;
	    }
	    else
	    {
	        lingo  = buf1;
	        lingo  = translate( lingo, ch->speaking, vch, ch );
		lingo  = makedrunk( lingo, ch );
		speech = lingo;
	    }

	    act( buf, ch, speech, vch, TO_VICT );
	    vch->position	= position;
	}
    }

    return;
}



void talk_auction( char *argument )
{
    DESCRIPTOR_DATA *d;
    char             buf	[ MAX_STRING_LENGTH ];
    CHAR_DATA       *och;

    sprintf( buf, "{yAuction: %s{x", argument );

    for ( d = descriptor_list; d; d = d->next )
    {
	och = ( d->original ? d->original : d->character );

	if ( d->connected == CON_PLAYING
	    && och->level > 2
	    && !IS_SET( och->deaf, CHANNEL_AUCTION ) 
            && !IS_SET( och->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
	act( buf, och, NULL, NULL, TO_CHAR );
    }

    return;
}



void do_chat( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_CHAT, "chat" );
    return;
}



/*
 * Alander's new channels.
 */
void do_music( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_MUSIC, "music" );
    return;
}



void do_question( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_QUESTION, "question" );
    return;
}



void do_answer( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_QUESTION, "answer" );
    return;
}



void do_shout( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_SHOUT, "shout" );
    WAIT_STATE( ch, 12 );
    return;
}



void do_grats( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_GRATS, "congratulate" );
    return;
}


void do_yell( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_YELL, "yell" );
    return;
}



void do_immtalk( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
  
    rch = get_char( ch );

    if ( !authorized( rch, "immtalk" ) )
        return;

    talk_channel( ch, argument, CHANNEL_IMMTALK, "immtalk" );
    return;
}



void do_wartalk( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
  
    rch = get_char( ch );

    if ( !is_clan( rch ) )
    {
	send_to_char( "You aren't a clansman!\n\r", ch );
        return;
    }

    talk_channel( ch, argument, CHANNEL_CLANTALK, "clantalk" );
    return;
}



void do_say( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char      *lingo;
    char      *speech;
    char       buf [ MAX_INPUT_LENGTH ];

    if ( argument[0] == '\0' )
    {
 	send_to_char_bw( "Say what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_MUTE )
        || IS_SET( race_table[ch->race].race_abilities, RACE_MUTE )
        || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
    {
        send_to_char( "Your lips move but no sound comes out.\n\r", ch );
        return;
    }

    strcpy( buf, argument );

    argument = makedrunk( argument, ch );

    MOBtrigger = FALSE;
    act( "{gYou say '$T{x{g'{x", ch, NULL, argument, TO_CHAR );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch->deleted || rch == ch )
	    continue;

      	if ( ch->speaking == 0 )
	{
	    speech = argument;
	}
        else
      	{
            lingo  = buf;
	    lingo  = translate( lingo, ch->speaking, rch, ch );
	    lingo  = makedrunk( lingo, ch );
	    speech = lingo;
      	}

	act( "{g$n says '$t{x{g'{x", ch, speech, rch, TO_VICT );

	if ( !IS_NPC( ch ) && IS_NPC( rch ) && HAS_TRIGGER( rch, TRIG_SPEECH ) )
	    mp_act_trigger( argument, rch, ch, NULL, NULL, TRIG_SPEECH );
    }

    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
    int        position;

    if ( IS_AFFECTED( ch, AFF_MUTE )
        || IS_SET( race_table[ch->race].race_abilities, RACE_MUTE )
        || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
    {
        send_to_char( "Your lips move but no sound comes out.\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( !( victim = get_char_world( ch, arg ) )
	|| ( IS_NPC( victim ) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ( !IS_NPC( ch ) && (   xIS_SET( ch->act, PLR_SILENCE )
                             || xIS_SET( ch->act, PLR_NO_TELL ) ) )
        || IS_SET( victim->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
    {
        send_to_char( "Your message didn't get through.\n\r", ch );
        return;
    }

    if ( !victim->desc )
    {
	act( "$N is link dead.", ch, 0, victim, TO_CHAR );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL( ch ) && !IS_AWAKE( victim ) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    argument = makedrunk( argument, ch );

    act( "{mYou tell $N '$t{x{m'{x", ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    act( "{m$n tells you '$t{x{m'{x", ch, argument, victim, TO_VICT );
    victim->position	= position;
    victim->reply	= ch;

    if ( xIS_SET( victim->act, PLR_AFK ) )
        act( "Just so you know, $E is AFK.", ch, NULL, victim, TO_CHAR );

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int        position;

    if ( IS_AFFECTED( ch, AFF_MUTE )
        || IS_SET( race_table[ch->race].race_abilities, RACE_MUTE )
        || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
    {
        send_to_char( "Your lips move but no sound comes out.\n\r", ch );
        return;
    }

    if ( !( victim = ch->reply ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ( !IS_NPC( ch ) && (   xIS_SET( ch->act, PLR_SILENCE )
                             || xIS_SET( ch->act, PLR_NO_TELL ) ) )
        || IS_SET( victim->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
    {
        send_to_char( "Your message didn't get through.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Reply what?\n\r", ch );
        return;
    }

    if ( !victim->desc )
    {
	act( "$N is link dead.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( !IS_IMMORTAL( ch ) && !IS_AWAKE( victim ) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    argument = makedrunk( argument, ch );

    act( "{mYou tell $N '$t{x{m'{x",  ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    act( "{m$n tells you '$t{x{m'{x", ch, argument, victim, TO_VICT );
    victim->position	= position;
    victim->reply	= ch;

    if ( xIS_SET( victim->act, PLR_AFK ) )
        act( "Just so you know, $E is AFK.", ch, NULL, victim, TO_CHAR );

    return;
}



void do_emote( CHAR_DATA *ch, char *argument )
{
    char  buf [ MAX_STRING_LENGTH ];
    char *plast;

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_NO_EMOTE ) )
    {
	send_to_char( "You can't show your emotions.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }

    for ( plast = argument; *plast != '\0'; plast++ )
	;

    strcpy( buf, argument );
    if ( isalpha( plast[-1] ) )
	strcat( buf, "." );

    act( "$n $T", ch, NULL, buf, TO_ROOM );
    MOBtrigger = FALSE;
    act( "$n $T", ch, NULL, buf, TO_CHAR );
    return;
}



void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC( ch ) )
	return;

    level = UMIN( ch->level, MAX_POSE );
    pose  = number_range( 0, level );

    act( ch->cclass[0]->pose[pose][0], ch, NULL, NULL, TO_CHAR );
    act( ch->cclass[0]->pose[pose][1], ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    char strsave [ MAX_INPUT_LENGTH ];

    sprintf( strsave, "%s%s", SYSTEM_DIR, BUG_FILE );

    if ( argument[0] == '\0' )
    {
	send_to_char( "The Implementors look at you quizzically.\n\r", ch );
	return;
    }

    append_file( ch, strsave,  argument );
    send_to_char( "Ok.  Thanks.\n\r", ch );
    return;
}



void do_idea( CHAR_DATA *ch, char *argument )
{
    char strsave [ MAX_INPUT_LENGTH ];

    sprintf( strsave, "%s%s", SYSTEM_DIR, IDEA_FILE );

    if ( argument[0] == '\0' )
    {
	send_to_char( "The Implementors look at you quizzically.\n\r", ch );
	return;
    }

    append_file( ch, strsave,  argument );
    send_to_char( "Ok.  Thanks.\n\r", ch );
    return;
}



void do_typo( CHAR_DATA *ch, char *argument )
{
    char strsave [ MAX_INPUT_LENGTH ];

    sprintf( strsave, "%s%s", SYSTEM_DIR, TYPO_FILE );

    if ( argument[0] == '\0' )
    {
	send_to_char( "The Implementors look at you quizzically.\n\r", ch );
	return;
    }

    append_file( ch, strsave,  argument );
    send_to_char( "Ok.  Thanks.\n\r", ch );
    return;
}



void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}



void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_NPC( ch ) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    if ( auction.item && ( ch == auction.buyer || ch == auction.seller ) )
    {
	send_to_char( "Wait until you trade the item on auction.\n\r", ch );
	return;
    }

    send_to_char( "{o{cAdde parvum parvo magnus acervus erit.{x\n\r", ch );
    send_to_char( "{o{w[Add little to little ",                     ch );
    send_to_char( "and there will be a big pile]{x\n\r\n\r",      ch );

    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    logln( "%s has quit.", ch->name );

    wiznet ( ch, WIZ_LOGINS, get_trust( ch ), log_buf );

    /*
     * After extract_char the ch is no longer valid!
     */
    save_char_obj( ch );
    d = ch->desc;
    extract_char( ch, TRUE );
    if ( d )
	close_socket( d );

    return;
}



void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
	return;

    if ( ch->level < 2 )
    {
	send_to_char( "You must be at least second level to save.\n\r", ch );
	return;
    }

    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( !ch->master )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower( ch );
	return;
    }

    if ( ( ch->level - victim->level < -5
	  || ch->level - victim->level >  5 )
	&& !IS_HERO( ch ) )
    {
	send_to_char( "You are not of the right caliber to follow.\n\r", ch );
	return;
    }

    if ( ch->master )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}



void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
  
    if ( ch->master )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{

    if ( !ch->master )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	xREMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
	affect_strip( ch, gsn_domination   );
    }

    if ( can_see( ch->master, ch ) )
	act( "$n stops following you.",
	    ch, NULL, ch->master, TO_VICT );
    act( "You stop following $N.",
	ch, NULL, ch->master, TO_CHAR );

    ch->master = NULL;
    ch->leader = NULL;
    return;
}



void die_follower( CHAR_DATA *ch, char *name )
{
    CHAR_DATA *fch;

    if ( ch->master )
	stop_follower( ch );

    ch->leader = NULL;

    for ( fch = char_list; fch; fch = fch->next )
    {
        if ( fch->deleted )
	    continue;
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = NULL;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    char       arg [ MAX_INPUT_LENGTH ];
    bool       found;
    bool       fAll;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( !IS_AFFECTED( victim, AFF_CHARM ) || victim->master != ch )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    if ( !str_cmp( argument, "mob" ) )
    {
	send_to_char( "That shall NOT be done.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( och = ch->in_room->people; och; och = och_next )
    {
        och_next = och->next_in_room;

        if ( och->deleted )
	    continue;

	if ( IS_AFFECTED( och, AFF_CHARM )
	    && och->master == ch
	    && ( fAll || och == victim ) )
	{
	    found = TRUE;
	    act( "$n orders you to '$t'.", ch, argument, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
	send_to_char( "Ok.\n\r", ch );
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}


void do_group( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       buf [ MAX_STRING_LENGTH ];
    char       arg [ MAX_INPUT_LENGTH  ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = ( ch->leader ) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS( leader, ch ) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch; gch = gch->next )
	{
	    if ( gch->deleted )
	        continue;
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5d xp\n\r",
			gch->level,
			class_short( gch ),
			capitalize( PERS( gch, ch ) ),
			gch->hit,   gch->max_hit,
			gch->mana,  gch->max_mana,
			gch->move,  gch->max_move,
			gch->exp );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master || ( ch->leader && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	act( "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	return;
    }

    if (   ch->level - victim->level < -5
	|| ch->level - victim->level >  5 )
    {
	act( "$N cannot join your group.",  ch, NULL, victim, TO_CHAR       );
	act( "You cannot join $n's group.", ch, NULL, victim, TO_VICT       );
	act( "$N cannot join $n's group.",  ch, NULL, victim, TO_NOTVICT    );
	return;
    }

    victim->leader = ch;
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
    char       buf [ MAX_STRING_LENGTH ];
    char       arg [ MAX_INPUT_LENGTH  ];
    int        members;
    int        amount;
    int        share;
    int        extra;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount = atoi( arg );

    if ( amount < 0 )
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold < amount )
    {
	send_to_char( "You don't have that much gold.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
        if ( gch->deleted )
	    continue;
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    sprintf( buf,
	"You split %d gold coins.  Your share is %d gold coins.\n\r",
	amount, share + extra );
    send_to_char( buf, ch );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	amount, share );

    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
        if ( gch->deleted )
	    continue;
	if ( gch != ch && is_same_group( gch, ch ) )
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
    char       buf [ MAX_STRING_LENGTH ];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_MUTE )
        || IS_SET( race_table[ch->race].race_abilities, RACE_MUTE )
        || IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
    {
        send_to_char( "Your lips move but no sound comes out.\n\r", ch );
        return;
    }

    if ( xIS_SET( ch->act, PLR_NO_TELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    argument = makedrunk( argument, ch );

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf( buf, "%s tells the group '{g%s{x'.\n\r", ch->name, argument );
    
    for ( gch = char_list; gch; gch = gch->next )
    {
        if ( is_same_group( gch, ch )
            && !IS_SET( gch->in_room->room_flags, ROOM_CONE_OF_SILENCE )
            && !IS_SET( race_table[gch->race].race_abilities, RACE_MUTE )
            && !IS_AFFECTED( gch, AFF_MUTE ) )
            send_to_char( buf, gch );
    }

    return;
}


/* Sent in by Judson Knott <jek@conga.oit.unc.edu> */
void do_beep( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH  ];
    char       buf [ MAX_STRING_LENGTH ];

    if ( IS_NPC( ch ) )
        return;

    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Beep who?\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char( "They are not beepable.\n\r", ch );
	return;
    }

    sprintf( buf, "You beep %s.\n\r", victim->name );
    send_to_char( buf, ch );
    
    sprintf( buf, "\a\a%s has beeped you.\n\r", ch->name );
    send_to_char( buf, victim );

    return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->deleted || bch->deleted )
        return FALSE;

    if ( ach->leader ) ach = ach->leader;
    if ( bch->leader ) bch = bch->leader;
    return ach == bch;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_clan( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->deleted || bch->deleted )
        return FALSE;

    if ( is_clan( ach ) && is_clan( bch ) )
	return ach->pcdata->clan == bch->pcdata->clan;
    else
	return FALSE;
}

/*
 * Colour setting and unsetting, way cool, Lope Oct '94
 */
void do_colour( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_STRING_LENGTH ];

    argument = one_argument( argument, arg );

    if ( !*arg )
    {
	if( !xIS_SET( ch->act, PLR_COLOUR ) )
	{
	    xSET_BIT( ch->act, PLR_COLOUR );
	    send_to_char( "{bC{ro{yl{co{mu{gr{x is now {rON{x, Way Cool!\n\r", ch );
	}
	else
	{
	    send_to_char_bw( "Colour is now OFF, <sigh>\n\r", ch );
	    xREMOVE_BIT( ch->act, PLR_COLOUR );
	}
	return;
    }
    else
    {
	send_to_char_bw( "Colour Configuration is unavailable in this\n\r", ch );
	send_to_char_bw( "version of colour, sorry\n\r", ch );
    }

    return;
}


/*
 * "retir" command is a trap to the "retire" command. (Zen)
 */
void do_retir( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to RETIRE, you have to spell it out.\n\r", ch );
 
    return;
}
 
 
/* 
 * Made by Zen :)
 * some inspiration from bits here and there in the Envy code...
 */
void do_retire( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
      return;

    if ( ch->position == POS_FIGHTING )
    {
      send_to_char( "No way! You are fighting.\n\r", ch );
      return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
      send_to_char( "You're not DEAD yet.\n\r", ch );
      return;
    }

    /* If playes does not have level 2 he should not have a file... */
    if ( ch->level < 2 )
    {
      send_to_char( "You can't even save and you want to retire?\n\r", ch );
      return;
    }

    write_to_buffer( ch->desc, "Password: ", 0 );
    write_to_buffer( ch->desc, echo_off_str, 0 );
    ch->desc->connected = CON_RETIRE_GET_PASSWORD;

    return;
}


void send_ansi_title( DESCRIPTOR_DATA *d )
{
    FILE *titlefile;
    char  buf     [ MAX_STRING_LENGTH*2 ];
    char  strsave [ MAX_INPUT_LENGTH    ];
    int   num;

    num = 0;

    fclose( fpReserve );

    sprintf( strsave, "%s%s", SYSTEM_DIR, ANSI_TITLE_FILE );

    if ( ( titlefile = fopen( strsave, "r" ) ) )
    {
	int c;

	while ( ( c = fgetc( titlefile ) ) != EOF )
	    buf[num++] = c;
	fclose( titlefile );
	buf[num] = '\0';
	write_to_buffer( d, buf, num );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void send_ascii_title( DESCRIPTOR_DATA *d )
{
    FILE *titlefile;
    char  buf     [ MAX_STRING_LENGTH*2 ];
    char  strsave [ MAX_INPUT_LENGTH    ];
    int   num;

    num = 0;

    fclose( fpReserve );

    sprintf( strsave, "%s%s", SYSTEM_DIR, ASCII_TITLE_FILE );

    if ( ( titlefile = fopen( strsave, "r" ) ) )
    {
	int c;

	while ( ( c = fgetc( titlefile ) ) != EOF )
	    buf[num++] = c;
	fclose( titlefile );
	buf[num] = '\0';
	write_to_buffer( d, buf, num );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


void news_channel( CHAR_DATA *ch, const char *string )
{
    DESCRIPTOR_DATA *d;
    char             buf [ MAX_STRING_LENGTH ];

    sprintf( buf, "{o{w{B[NEWS]{x {o{w%s{x\n\r", string );

    for ( d = descriptor_list; d; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ( d->connected == CON_PLAYING
	    && vch != ch
	    && !IS_SET( och->deaf, CHANNEL_NEWS )
            && !IS_SET( och->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
	    send_to_char( buf, vch );
    }

    return;
}
