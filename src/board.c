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



struct	board_data	board_table	[MAX_BOARD]	=
{

    {
	"general",	"General discussion",		0,      2,
	"all",					BOARD_INCLUDE,	21
    },

    {
	"ideas",	"Suggestion for improvement",	0,      2,
	"all",					BOARD_NORMAL,	60
    },

    {
	"announce",	"Announcements from Immortals",	0,  L_APP,
	"all",					BOARD_NORMAL,	60
    },

    {
	"bugs",		"Typos, bugs, errors",		0,      1,
	"imm",					BOARD_NORMAL,	60
    },

    {
	"personal",	"Personal messages",		0,      1,
	"all",					BOARD_EXCLUDE,	28
    }

};



/*
 * Find a board number based on a string.
 */
int board_lookup( const char *name )
{
    int i;

    for ( i = 0; i < MAX_BOARD; i++ )
	if ( !str_cmp( board_table[i].short_name, name ) )	return i;

    return -1;
}



/*
 * Find the number of a board.
 */
int board_number( const BOARD_DATA *board )
{
    int i;

    for ( i = 0; i < MAX_BOARD; i++ )
	if ( board == &board_table[i] )  return i;

    return -1;
}



void note_attach( CHAR_DATA *ch )
{
    NOTE_DATA *pnote;

    if ( ch->pcdata->note )
	return;

    if ( !note_free )
    {
	pnote	  = (NOTE_DATA *) alloc_perm( sizeof( *ch->pcdata->note ) );
    }
    else
    {
	pnote	  = note_free;
	note_free = note_free->next;
    }

    pnote->next		= NULL;
    pnote->sender	= str_dup( ch->name );
    pnote->date		= str_dup( "" );
    pnote->to_list	= str_dup( "" );
    pnote->subject	= str_dup( "" );
    pnote->text		= str_dup( "" );
    pnote->expire	= current_time +
			  ch->pcdata->board->purge_days * 24L * 3600L;
    ch->pcdata->note	= pnote;
    return;
}



/*
 * Recycle a note.
 */
void free_note( NOTE_DATA *pnote )
{
    if ( pnote->sender  )	free_string( pnote->sender  );
    if ( pnote->to_list )	free_string( pnote->to_list );
    if ( pnote->subject )	free_string( pnote->subject );
    if ( pnote->date    )	free_string( pnote->date    );
    if ( pnote->text    )	free_string( pnote->text    );
    	    
    pnote->next	= note_free;
    note_free	= pnote;      
    return;
}



/*
 * Remove list from the list.  Do not free note.
 */
void unlink_note( BOARD_DATA *board, NOTE_DATA *note )
{
    NOTE_DATA *p;

    if ( board->note_first == note )
	board->note_first = note->next;
    else
    {
	for ( p = board->note_first; p && p->next != note; p = p->next )
	    ;

	if ( !p )
	    bug( "Unlink_note: couldn't find note." , 0 );
	else
	    p->next = note->next;
    }

    return;
}



/*
 * Append this note to the given file.
 */
void append_note( FILE *fp, NOTE_DATA *pnote )
{
    fprintf( fp, "Sender  %s~\n", pnote->sender			);
    fprintf( fp, "Date    %s~\n", pnote->date			);
    fprintf( fp, "Stamp   %ld\n", (uiptr) pnote->date_stamp	);
    fprintf( fp, "Expire  %ld\n", (uiptr) pnote->expire		);
    fprintf( fp, "To      %s~\n", pnote->to_list		);
    fprintf( fp, "Subject %s~\n", pnote->subject		);
    fprintf( fp, "Text\n%s~\n\n", fix_string( pnote->text )	);
    return;
}



void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    BOARD_DATA *board;
    NOTE_DATA  *prev;
    char       *to_list;
    char        to_new  [ MAX_INPUT_LENGTH ];
    char        to_one  [ MAX_INPUT_LENGTH ];

    /*
     * Build a new to_list.
     * Strip out this recipient.
     */
    to_new[0]	= '\0';
    to_list	= pnote->to_list;
    while ( *to_list != '\0' )
    {
	to_list	= one_argument( to_list, to_one );
	if ( to_one[0] != '\0' && str_cmp( ch->name, to_one ) )
	{
	    strcat( to_new, " "    );
	    strcat( to_new, to_one );
	}
    }

    /*
     * Just a simple recipient removal?
     */
    if ( str_cmp( ch->name, pnote->sender ) && to_new[0] != '\0' )
    {
	free_string( pnote->to_list );
	pnote->to_list = str_dup( to_new + 1 );
	return;
    }

    board = ch->pcdata->board;

    /*
     * Remove note from linked list.
     */
    if ( pnote == board->note_first )
    {
	board->note_first = pnote->next;
    }
    else
    {
	for ( prev = board->note_first; prev; prev = prev->next )
	{
	    if ( prev->next == pnote )
		break;
	}

	if ( !prev )
	{
	    bug( "Note_remove: pnote not found.", 0 );
	    return;
	}

	prev->next = pnote->next;
    }

    free_note( pnote );

    save_board( board );
    return;
}



bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    if ( !str_cmp( ch->name, pnote->sender ) )
	return TRUE;

    if ( is_name( "all", pnote->to_list ) )
	return TRUE;

    if ( IS_IMMORTAL( ch ) && (   is_name( "immortal",  pnote->to_list )
			       || is_name( "immortals", pnote->to_list )
			       || is_name( "imm",       pnote->to_list )
			       || is_name( "immort",    pnote->to_list ) ) )
	return TRUE;

    if ( is_name( ch->name, pnote->to_list ) )
	return TRUE;

    if ( is_number( pnote->to_list )
	&& get_trust( ch ) >= atoi( pnote->to_list ) )
	return TRUE;

    return FALSE;
}



int unread_notes( CHAR_DATA *ch, BOARD_DATA *board )
{
    NOTE_DATA *note;
    time_t     last_read;
    int        count = 0;

    if ( board->read_level > get_trust( ch ) )
	return -1;

    last_read = ch->pcdata->last_note[ board_number( board ) ];

    for ( note = board->note_first; note; note = note->next )
	if ( is_note_to( ch, note ) && last_read < note->date_stamp )
	    count++;

    return count;
}



bool next_board( CHAR_DATA *ch )
{
    int i = board_number( ch->pcdata->board ) + 1;

    while ( i < MAX_BOARD && unread_notes( ch, &board_table[i] ) == -1 )
	i++;
    	    
    if ( i == MAX_BOARD )
	return FALSE;

    ch->pcdata->board = &board_table[i];
    return TRUE;
}



void do_board( CHAR_DATA *ch, char *argument )
{
    int  i;
    int  count;
    char buf	[ MAX_INPUT_LENGTH ];

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	int unread;

	count = 1;
	send_to_char( "\n\r{B{o{b[ {wBULLETIN BOARDS {b]{x{b-----------------------------------------------------{x\n\r", ch );
	for ( i = 0; i < MAX_BOARD; i++ )
	{
	    unread = unread_notes( ch, &board_table[i] );

	    if ( unread == -1 )
		continue;

	    sprintf( buf, "{o%s%2d    %c%c    %s%4d {r%s%-12s - %38s{x\n\r",
		    ( !strcmp( ch->pcdata->board->short_name,
				board_table[i].short_name ) ) ? "{R" : "{b",
		    count,
		    ( ch->pcdata->board->read_level  <= get_trust( ch ) )
			? 'R' : ' ',
		    ( ch->pcdata->board->write_level <= get_trust( ch ) )
			? 'W' : ' ',
		    ( unread == 0 )
			? "{b" : "{y",
		    unread,
		    ( !strcmp( ch->pcdata->board->short_name,
				board_table[i].short_name ) ) ? "->{w" : "  {b",
		    board_table[i].short_name,
		    board_table[i].long_name );
	    send_to_char( buf, ch );

	    count++;
	}

	send_to_char( "{b------------------------------------------------------------------------{x\n\r\n\r", ch );

	return;
    }

    if ( ch->pcdata->note )
    {
	send_to_char( "Please finish your interrupted note first.\n\r", ch );
	return;
    }

    i = atoi( argument ) - 1;

    if ( is_number( argument ) )
    {
	if ( i < 0 || i >= MAX_BOARD )
	{
	    send_to_char( "No such board.\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( i = board_lookup( argument ) ) == -1 )
	{
	    send_to_char( "No such board.\n\r", ch );
	    return;
	}
    }

    if ( unread_notes( ch, &board_table[i] ) == -1 )
    {
	send_to_char( "No such board.\n\r", ch );
	return;
    }

    ch->pcdata->board = &board_table[i];

    sprintf( buf, "Current board changed to {n%s{x. %s.\n\r",
	    board_table[i].short_name,
	    ( get_trust( ch ) < board_table[i].write_level )
		? "You can only read here" 
		: "You can both read and write here" );
    send_to_char( buf, ch );
    return;
}



/* Date stamp idea comes from Alander of ROM */
void do_note( CHAR_DATA *ch, char *argument )
{
    BOARD_DATA *board;
    NOTE_DATA  *pnote;
    char        buf     [ MAX_STRING_LENGTH   ];
    char        buf1    [ MAX_STRING_LENGTH*7 ];
    char        arg     [ MAX_INPUT_LENGTH    ];
    char        strsave [ MAX_INPUT_LENGTH    ];
    int         vnum;
    int         anum;
    time_t     *last_note;

    if ( IS_NPC( ch ) )
	return;

    argument = one_argument( argument, arg );
    smash_tilde( argument );

    if ( arg[0] == '\0' )
    {
	do_note( ch, "read" );
	return;
    }

    board	=  ch->pcdata->board;
    last_note	= &ch->pcdata->last_note[board_number(board)];

    if ( !str_cmp( arg, "list" ) )
    {
	vnum    = 0;
	buf1[0] = '\0';

	sprintf( buf, "\n\r{y-{R{o{r[ {y%-30s {r]{x{y------------------------------------{x\n\r", board->short_name );
	strcat( buf1, buf );

	for ( pnote = board->note_first; pnote; pnote = pnote->next )
	{
	    vnum++;

	    if ( !is_note_to( ch, pnote ) )
		continue;

	    sprintf( buf, "%4d%c    %-44s{x   - %-14s\n\r",
		    vnum, 
		    pnote->date_stamp > *last_note ? 'N' : ' ',
		    pnote->subject, pnote->sender );
	    strcat( buf1, buf );
	}

	strcat( buf1, "{y-----------------------------------------------------------------------{x\n\r\n\r" );

	send_to_char( buf1, ch );
	return;
    }

    if ( !str_cmp( arg, "read" ) )
    {
	bool fAll;

	if ( !str_cmp( argument, "all" ) )
	{
	    fAll = TRUE;
	    anum = 0;
	}
	else if ( argument[0] == '\0' || !str_prefix( argument, "next" ) )
	  /* read next unread note */
	{
	    vnum    = 1;
	    buf1[0] = '\0';
	    for ( pnote = board->note_first; pnote; pnote = pnote->next, vnum++ )
	    {
		if ( is_note_to( ch, pnote )
		    && *last_note < pnote->date_stamp )
		{
		    break;
		}
	    }
	    if ( pnote )
	    {
    sprintf( buf1+strlen( buf1 ),
	"{W-----------------------------------------------------------------------{x\n\r"
	"{o{y     :{x  %-52s  {x{o{ynote %-4d{x\n\r"
	"{o{yDate :{x  %s{x\n\r"
	"{o{yTo   :{x  %s{x\n\r"
	"{o{yAutor:{x  %s{x\n\r"
	"{W-----------------------------------------------------------------------{x\n\r"
	"%s{x\n\r",
	    pnote->subject,
	    vnum,
	    pnote->date,
	    pnote->to_list,
	    pnote->sender,
	    pnote->text );

		*last_note = UMAX( *last_note, pnote->date_stamp );
		send_to_char( buf1, ch );
		return;
	    }

	    sprintf( buf, "No new notes in this board" );
		
	    if ( next_board( ch ) )
		sprintf( buf+strlen( buf ), "; changed to {n%s{x board",
			ch->pcdata->board->short_name );
	    strcat( buf, ".\n\r" );
			
	    send_to_char( buf, ch );
	    return;
	}
	else if ( is_number( argument ) )
	{
	    fAll = FALSE;
	    anum = atoi( argument );
	}
	else
	{
	    send_to_char( "Note read which number?\n\r", ch );
	    return;
	}

	vnum    = 0;
	buf1[0] = '\0';
	for ( pnote = board->note_first; pnote; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) )
	    {
	        if ( ++vnum == anum || fAll )
		{
    sprintf( buf1+strlen( buf1 ),
	"{W-----------------------------------------------------------------------{x\n\r"
	"{o{y     :{x  %-52s  {x{o{ynote %-4d{x\n\r"
	"{o{yDate :{x  %s{x\n\r"
	"{o{yTo   :{x  %s{x\n\r"
	"{o{yAutor:{x  %s{x\n\r"
	"{W-----------------------------------------------------------------------{x\n\r"
	"%s{x\n\r",
	    pnote->subject,
	    vnum,
	    pnote->date,
	    pnote->to_list,
	    pnote->sender,
	    pnote->text );

		    if ( !fAll )
		      send_to_char( buf1, ch );
		    else
		      strcat( buf1, "\n\r" );
		    *last_note = UMAX( *last_note, pnote->date_stamp );
		    if ( !fAll )
		      return;
		}
	    }
	}

	if ( !fAll )
	    send_to_char( "No such note.\n\r", ch );
	else
	    send_to_char( buf1, ch );
	return;
    }

    if ( !str_cmp( arg, "edit" ) )
    {
	note_attach( ch );
	string_append( ch, &ch->pcdata->note->text );
	return;
    }

    if ( !str_cmp( arg, "expire" ) )
    {
	if ( !IS_IMMORTAL( ch ) )
	    return;

	if ( !is_number( argument ) )
	{
	    send_to_char( "Note expire which number of days?\n\r", ch );
	    return;
	}

	anum = atoi( argument );

	ch->pcdata->note->expire	= current_time + anum * 24L * 3600L;
	return;
    }

    if ( !str_cmp( arg, "subject" ) )
    {
	note_attach( ch );
	free_string( ch->pcdata->note->subject );
	ch->pcdata->note->subject = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "to" ) )
    {
	note_attach( ch );
	free_string( ch->pcdata->note->to_list );
	ch->pcdata->note->to_list = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "clear" ) )
    {
	if ( ch->pcdata->note )
	{
	    free_note( ch->pcdata->note );
	    ch->pcdata->note	= NULL;
	}

	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "catchup" ) )
    {
	for ( pnote=board->note_first; pnote && pnote->next; pnote=pnote->next )
	    ;

	if ( !pnote )
	    send_to_char( "Alas, there are no notes in that board.\n\r", ch );
	else
	{
	    *last_note = pnote->date_stamp;
	    send_to_char( "All messages skipped.\n\r", ch );
	}
	return;
    }

    if ( !str_cmp( arg, "show" ) )
    {
	if ( !ch->pcdata->note )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

    sprintf( buf1,
	"{W-----------------------------------------------------------------------{x\n\r"
	"{o{y     :{x  %-52s{x\n\r"
	"{o{yTo   :{x  %s{x\n\r"
	"{o{yAutor:{x  %s{x\n\r"
	"{W-----------------------------------------------------------------------{x\n\r"
	"%s{x\n\r",
	    ch->pcdata->note->subject,
	    ch->pcdata->note->to_list,
	    ch->pcdata->note->sender,
	    ch->pcdata->note->text );
	send_to_char( buf1, ch );
	return;
    }

    if ( !str_cmp( arg, "post" ) || !str_prefix( arg, "send" ) )
    {
	FILE *fp;
	char *strtime;

	if ( !ch->pcdata->note )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

	if ( get_trust( ch ) < ch->pcdata->board->write_level )
	{
	    send_to_char ( "You can't post notes on this board.\n\r", ch );
	    return;
	}

	switch ( board->force_type )
	{
	case BOARD_NORMAL:
		if ( !str_cmp( ch->pcdata->note->to_list, "" ) )
		{
		    free_string( ch->pcdata->note->to_list );
		    ch->pcdata->note->to_list = str_dup( board->names );

		    sprintf( buf, "Default recipient '{n%s{x' chosen.\n\r",
			    board->names );
		    send_to_char( buf, ch );
		}
		break;
	case BOARD_INCLUDE:
		if ( !is_name( board->names, ch->pcdata->note->to_list ) )
		{
		    strcpy( buf, ch->pcdata->note->to_list );
		    strcat( buf, board->names );
		    free_string( ch->pcdata->note->to_list );
		    ch->pcdata->note->to_list = str_dup( buf );

		    sprintf( buf, "Included mandatory recipient '{n%s{x'.\n\r",
			board->names );
		    send_to_char( buf, ch );
		}
		break;

	case BOARD_EXCLUDE:
		if ( is_name( board->names, ch->pcdata->note->to_list ) )
		{
		    sprintf( buf, "The recipient may not include '{n%s{x'.\n\r",
			board->names );
		    send_to_char( buf, ch );
		    return;
		}
		break;
	}

	if ( !str_cmp( ch->pcdata->note->to_list, "" ) )
	{
	    send_to_char(
	      "You need to provide a recipient (name, all, or immortal).\n\r",
			 ch );
	    return;
	}

	if ( !str_cmp( ch->pcdata->note->subject, "" ) )
	{
	    send_to_char( "You need to provide a subject.\n\r", ch );
	    return;
	}

	ch->pcdata->note->next		= NULL;
	strtime				= ctime( &current_time );
	strtime[strlen(strtime)-1]	= '\0';
	free_string( ch->pcdata->note->date );
	ch->pcdata->note->date		= str_dup( strtime );
	ch->pcdata->note->date_stamp	= current_time;
	ch->pcdata->note->expire	= current_time +
					  board->purge_days * 24L * 3600L;

	if ( !board->note_first )
	{
	    board->note_first	= ch->pcdata->note;
	}
	else
	{
	    for ( pnote = board->note_first; pnote->next; pnote = pnote->next )
		;
	    pnote->next	= ch->pcdata->note;
	}
	pnote			= ch->pcdata->note;
	ch->pcdata->note	= NULL;

	fclose( fpReserve );

	sprintf( strsave, "%s%s", NOTE_DIR, board->short_name );

	if ( !( fp = fopen( strsave, "a" ) ) )
	{
	    perror( board->short_name );
	}
	else
	{
	    append_note( fp, pnote );
	    fclose( fp );
	}
	fpReserve = fopen( NULL_FILE, "r" );

	sprintf( buf, "Note posted.  This note will expire %s\r",
		ctime( &pnote->expire ) );
	send_to_char( buf, ch );
	return;
    }

    if ( !str_cmp( arg, "remove" ) )
    {
	if ( !is_number( argument ) )
	{
	    send_to_char( "Note remove which number?\n\r", ch );
	    return;
	}

	anum = atoi( argument );
	vnum = 1;
	for ( pnote = board->note_first; pnote; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) && vnum++ == anum )
	    {
		note_remove( ch, pnote );
		send_to_char( "Ok.\n\r", ch );
		return;
	    }
	}

	send_to_char( "No such note.\n\r", ch );
	return;
    }

    send_to_char( "Huh?  Type 'help note' for usage.\n\r", ch );
    return;
}
