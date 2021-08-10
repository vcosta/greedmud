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
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#if defined( sun )
#include <memory.h>
#endif

extern	int	_filbuf		args( (FILE *) );

#if defined( sun )
int     system          args( ( const char *string ) );
#endif



/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[ MAX_NEST ];


int stat;

/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			       FILE *fp, int iNest ) );
int	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
int	envy_fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );
int	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );


/* Courtesy of Yaz of 4th Realm */
char *initial( const char *str )
{
    static char strint [ MAX_STRING_LENGTH ];

    strint[0] = LOWER( str[ 0 ] );
    return strint;

}

/*
 * Backups a character and inventory.
 * Courtesy of Zen :)
 */
void backup_char_obj( CHAR_DATA *ch )
{
    FILE *fp;
#if defined( USE_GZIP )
    char  buf     [ MAX_STRING_LENGTH ];
#endif
    char  strsave [ MAX_INPUT_LENGTH  ];

    if ( IS_NPC( ch ) || ch->level < 2 )
	return;

    if ( ch->desc && ch->desc->original )
	ch = ch->desc->original;

    ch->save_time = current_time;
    fclose( fpReserve );

    /* player files parsed directories by Yaz 4th Realm */
    sprintf( strsave, "%s%s%s%s", BACKUP_DIR, initial( ch->name ),
	    DIR_SEPARATOR, capitalize( ch->name ) );

    if ( !( fp = fopen( strsave, "w" ) ) )
    {
        bugf( "Backup_char_obj: fopen %s: ", ch->name );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->carrying )
	    fwrite_obj( ch, ch->carrying, fp, 0 );
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

#if defined( USE_GZIP )
    sprintf( buf, "gzip -fq %s", strsave );
    system( buf );
#endif
    return;
}

/*
 * Delete a character's file.
 * Used for retire & delete commands for now.
 * Courtesy of Zen :)
 */
void delete_char_obj( CHAR_DATA *ch )
{
    char  strsave [ MAX_INPUT_LENGTH  ];

    if ( IS_NPC( ch ) || ch->level < 2 )
	return;

    /* player files parsed directories by Yaz 4th Realm */
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( ch->name ),
	    DIR_SEPARATOR, capitalize( ch->name ) );

    if ( remove( strsave ) )
    {
        bugf( "Delete_char_obj: remove %s: ", ch->name );
	perror( strsave );
    }
    return;
}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    FILE *fp;
#if defined( USE_GZIP )
    char  buf     [ MAX_STRING_LENGTH ];
#endif
    char  strsave [ MAX_INPUT_LENGTH  ];

    if ( IS_NPC( ch ) || ch->level < 2 )
	return;

    if ( ch->desc && ch->desc->original )
	ch = ch->desc->original;

    ch->save_time = current_time;
    fclose( fpReserve );

    /* player files parsed directories by Yaz 4th Realm */
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( ch->name ),
	    DIR_SEPARATOR, capitalize( ch->name ) );

    if ( !( fp = fopen( strsave, "w" ) ) )
    {
        bugf( "Save_char_obj: fopen %s: ", ch->name );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->carrying )
	    fwrite_obj( ch, ch->carrying, fp, 0 );
	fprintf( fp, "#END\n" );
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );

#if defined( USE_GZIP )
    sprintf( buf, "gzip -fq %s", strsave );
    system( buf );
#endif
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int          sn;
    ALIAS_DATA  *alias;

    fprintf( fp, "#%s\n", IS_NPC( ch ) ? "MOB" : "PLAYER"		);

    fprintf( fp, "Nm          %s~\n",	ch->name			);
    fprintf( fp, "ShtDsc      %s~\n",	ch->short_descr			);
    fprintf( fp, "LngDsc      %s~\n",	ch->long_descr			);
    fprintf( fp, "Dscr        %s~\n",	fix_string( ch->description )	);
    fprintf( fp, "Prmpt       %s~\n",	ch->pcdata->prompt		);
    fprintf( fp, "Sx          %d\n",	ch->sex				);

    fprintf( fp, "Class      " );
    for ( sn = 0; sn < MAX_MULTICLASS; sn++ )
      fprintf( fp, " %s~", ch->cclass[sn] ? ch->cclass[sn]->name : "" );
    fprintf( fp, "\n" );

    fprintf( fp, "Race        %s~\n",	race_table[ ch->race ].name 	);

    fprintf( fp, "Lvl         %d\n",	ch->level			);
    fprintf( fp, "Trst        %d\n",	ch->trust			);
    fprintf( fp, "Playd       %d\n",
	ch->played + (int) ( current_time - ch->logon )			);

    for ( sn = 0; sn < MAX_BOARD; sn++ )
	fprintf( fp, "Board       %ld '%s'\n",
		(uiptr)ch->pcdata->last_note[sn],
		board_table[sn].short_name				);

    fprintf( fp, "Room        %d\n",
	    (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	     && ch->was_in_room )
	    ? ch->was_in_room->vnum
	    : ch->in_room->vnum );

    fprintf( fp, "HpMnMv      %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    fprintf( fp, "Gold        %d\n",	ch->gold		);
    fprintf( fp, "Exp         %d\n",	ch->exp			);
    fprintf( fp, "ActF        %s~\n",   write_xbv( plr_flags, ch->act ) );
    fprintf( fp, "AffectedBy  %s~\n",	write_xbv( affect_flags, ch->affected_by ) );

    if ( ch->resistant )
	fprintf( fp, "Res         %d\n",	ch->resistant		);
    if ( ch->immune )
	fprintf( fp, "Imm         %d\n",	ch->immune			);
    if ( ch->susceptible )
	fprintf( fp, "Susc        %d\n",	ch->susceptible		);

    fprintf( fp, "Langs       %d %d\n",		ch->speaks, ch->speaking );

    /* Bug fix from Alander */
    fprintf( fp, "Pos         %d\n",
	    ch->position == POS_FIGHTING ? POS_STANDING : ch->position );

    fprintf( fp, "Prac        %d\n",	ch->practice		);
    fprintf( fp, "SavThr      %d\n",	ch->saving_throw	);
    fprintf( fp, "Align       %d\n",	ch->alignment		);
    fprintf( fp, "Hit         %d\n",	ch->hitroll		);
    fprintf( fp, "Dam         %d\n",	ch->damroll		);
    fprintf( fp, "Armr        %d\n",	ch->armor		);
    fprintf( fp, "Wimp        %d\n",	ch->wimpy		);
    fprintf( fp, "Deaf        %d\n",	ch->deaf		);

    if ( IS_NPC( ch ) )
    {
	fprintf( fp, "Vnum        %d\n",	ch->pIndexData->vnum	);
    }
    else
    {
	fprintf( fp, "Paswd       %s~\n",	ch->pcdata->pwd		);
	fprintf( fp, "Bmfin       %s~\n",	ch->pcdata->bamfin	);
	fprintf( fp, "Bmfout      %s~\n",	ch->pcdata->bamfout	);
	fprintf( fp, "Immskll     %s~\n",	ch->pcdata->immskll	);
	fprintf( fp, "Wiznet      %d\n",	ch->pcdata->wiznet	);
	fprintf( fp, "Ttle        %s~\n",	ch->pcdata->title	);
	fprintf( fp, "AtrPrm      %d %d %d %d %d\n",
		ch->perm_str,
		ch->perm_int,
		ch->perm_wis,
		ch->perm_dex,
		ch->perm_con );

	fprintf( fp, "AtrMd       %d %d %d %d %d\n",
		ch->mod_str, 
		ch->mod_int, 
		ch->mod_wis,
		ch->mod_dex, 
		ch->mod_con );

	fprintf( fp, "Cond        %d %d %d\n",
		ch->pcdata->condition[0],
		ch->pcdata->condition[1],
		ch->pcdata->condition[2] );

	fprintf( fp, "Security    %d\n",   ch->pcdata->security		);

	if ( is_clan( ch ) )
	    fprintf( fp, "PClan       %2d %s~\n",
		    ch->pcdata->rank,
		    ch->pcdata->clan->name );

	if ( ch->pcdata->pkills )
	    fprintf( fp, "PKills      %d\n", ch->pcdata->pkills	);
	if ( ch->pcdata->pdeaths )
	    fprintf( fp, "PDeaths     %d\n", ch->pcdata->pdeaths	);
	if ( ch->pcdata->illegal_pk )
	    fprintf( fp, "IllegalPK   %d\n", ch->pcdata->illegal_pk	);

	fprintf( fp, "MKills      %d\n",   ch->pcdata->mkills		);
	fprintf( fp, "MDeaths     %d\n",   ch->pcdata->mdeaths		);

	fprintf( fp, "Pglen       %d\n",   ch->pcdata->pagelen		);

        for ( alias = ch->pcdata->alias_list; alias; alias = alias->next )
	    fprintf( fp, "NAlias      '%s' %s~\n", alias->cmd,
	    					   fix_string( alias->subst ) );

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name && ch->pcdata->learned[sn] > 0 )
	    {
		fprintf( fp, "Skll        %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}
    }

    for ( paf = ch->affected; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;

	fprintf( fp, "Affect     %18s~ %3d %3d %s~ %s~\n",
		skill_table[ paf->type ].name,
		paf->duration,
		paf->modifier,
		flag_strings( apply_flags, paf->location ),
		write_xbv( affect_flags, paf->bitvector ) );
    }

    fprintf( fp, "End\n\n" );
    return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    AFFECT_DATA      *paf;
    EXTRA_DESCR_DATA *ed;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content )
	fwrite_obj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     */
    if ( ch->level < obj->level
	|| IS_OBJ_STAT( obj, ITEM_NOSAVE )
	|| obj->deleted )
	return;

    fprintf( fp, "#OBJ\n" );
    fprintf( fp, "Nest         %d\n",	iNest			     );
    fprintf( fp, "Name         %s~\n",	obj->name		     );
    fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr	     );
    fprintf( fp, "Description  %s~\n",	obj->description	     );
    fprintf( fp, "Vnum         %d\n",	obj->pIndexData->vnum	     );

    if ( obj->spec_fun )
      fprintf( fp, "Special      %s\n",	spec_obj_string( obj->spec_fun ) );

    fprintf( fp, "ExtraFlags   %s~\n",
                          	write_xbv( extra_flags, obj->extra_flags ) );
    fprintf( fp, "WearFlags    %d\n",	obj->wear_flags		     );
    fprintf( fp, "WearLoc      %d\n",	obj->wear_loc		     );
    fprintf( fp, "ItemType     %d\n",	obj->item_type		     );
    fprintf( fp, "Weight       %d\n",	obj->weight		     );
    fprintf( fp, "Level        %d\n",	obj->level		     );
    fprintf( fp, "Timer        %d\n",	obj->timer		     );
    fprintf( fp, "Cost         %d\n",	obj->cost		     );
    fprintf( fp, "Values       %d %d %d %d %d\n",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
							obj->value[4]   );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1      '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2      '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	if ( obj->value[4] > 0 )
	{
	    fprintf( fp, "Spell 4      '%s'\n", 
		skill_table[obj->value[4]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf; paf = paf->next )
    {
	fprintf( fp, "Affect       %d %d %d %d %s~\n",
		paf->type,
		paf->duration,
		paf->modifier,
		paf->location,
		write_xbv( affect_flags, paf->bitvector ) );
    }

    for ( ed = obj->extra_descr; ed; ed = ed->next )
    {
	fprintf( fp, "ExtraDescr   %s~ %s~\n",
		ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    tail_chain();
    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    FILE      *fp;
    CHAR_DATA *ch;
    char       strsave [ MAX_INPUT_LENGTH ];
    bool       found;
    char       sorry_player [] =
      "********************************************************\n\r"
      "** One or more of the critical fields in your player  **\n\r"
      "** file were corrupted since you last played.  Please **\n\r"
      "** contact an administrator or programmer to	     **\n\r"
      "** investigate the recovery of your characters.       **\n\r"
      "********************************************************\n\r";
    char       sorry_object [] =
      "********************************************************\n\r"
      "** One or more of the critical fields in your player  **\n\r"
      "** file were corrupted leading to the loss of one or  **\n\r"
      "** more of your possessions.			     **\n\r"
      "********************************************************\n\r";
    int        i;


    ch					= new_character( TRUE );

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->pcdata->prompt                  = str_dup( daPrompt );
    ch->act				= new_xbv ( PLR_BLANK,
						    PLR_COMBINE,
						    PLR_PROMPT,
						    PLR_PAGER, -1 );

    ch->pcdata->board			= &board_table[0];

    for ( i = 0; i < MAX_BOARD; i++ )
	ch->pcdata->last_note[i]	= 0;

    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->pwdnew			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->immskll			= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->perm_str			= 13;
    ch->perm_int			= 13; 
    ch->perm_wis			= 13;
    ch->perm_dex			= 13;
    ch->perm_con			= 13;
    ch->pcdata->condition[COND_THIRST]	= 48;
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->pagelen                 = 20;
    ch->pcdata->security		= 0;
    ch->pcdata->rank			= 0;
    ch->pcdata->clan	                = NULL;
    ch->pcdata->wiznet			= 0;

    ch->pcdata->switched                = FALSE;

    for ( i = 0; i < MAX_SKILL; i++ )
	ch->pcdata->learned[i]		= 0;

    found = FALSE;
    fclose( fpReserve );

    /* parsed player file directories by Yaz of 4th Realm */
    /* decompress if .gz file exists - Thx Alander */
    sprintf( strsave, "%s%s%s%s%s", PLAYER_DIR, initial( ch->name ),
	    DIR_SEPARATOR, capitalize( name ), ".gz" );
    if ( ( fp = fopen( strsave, "r" ) ) )
    {
#if defined( USE_GZIP )
        char       buf     [ MAX_STRING_LENGTH ];
#endif

	fclose( fp );
#if defined( USE_GZIP )
	sprintf( buf, "gzip -dfq %s", strsave );
	system( buf );
#endif
    }

    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( ch->name ),
	    DIR_SEPARATOR, capitalize( name ) );

    if ( ( fp = fopen( strsave, "r" ) ) )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char *word;
	    int   letter;
	    int   status;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bugf( "Load_char_obj: # not found." );
		break;
	    }

	    word = fread_word( fp, &status );

	    if ( !str_cmp( word, "PLAYER" ) )
	    {
	        if ( fread_char ( ch, fp ) )
		{
		    bugf( "Load_char_obj:  %s section PLAYER corrupt.", name );
		    write_to_buffer( d, sorry_player, 0 );

		    /* 
		     * In case you are curious,
		     * it is ok to leave ch alone for close_socket
		     * to free.
		     * We want to now kick the bad character out as
		     * what we are missing are MANDATORY fields.  -Kahn
		     */
		    xSET_BIT( ch->act, PLR_DENY );
		    found = TRUE;
		    break;
		}
	    }
	    else if ( !str_cmp( word, "OBJECT" ) )
	    {
	        if ( !envy_fread_obj  ( ch, fp ) )
		{
		    bugf( "Load_char_obj:  %s section OBJECT corrupt.", name );
		    write_to_buffer( d, sorry_object, 0 );
		    found = FALSE;
		    break;
		}
	    }
	    else if ( !str_cmp( word, "OBJ" ) )
	    {
	        if ( !fread_obj  ( ch, fp ) )
		{
		    bugf( "Load_char_obj:  %s section OBJ corrupt.", name );
		    write_to_buffer( d, sorry_object, 0 );
		    found = FALSE;
		    break;
		}
	    }
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bugf( "Load_char_obj:  %s bad section %s.", name, word );
		break;
	    }
	} /* for */

	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return found;
}



/*
 * Read in a char.
 */

int fread_char( CHAR_DATA *ch, FILE *fp )
{
    char        *word;
    char        buf [ MAX_STRING_LENGTH ];
    AFFECT_DATA *paf;
    int         sn;
    int         i;
    int         j;
    int         error_count = 0;
    int         status;
    int         status1;
    char        *p;
    int         tmpi;
    int         num_keys;
    int         last_key = 0;

    char        def_sdesc  [] = "Your short description was corrupted.";
    char        def_ldesc  [] = "Your long description was corrupted.";
    char        def_desc   [] = "Your description was corrupted.";
    char        def_title  [] = "Your title was corrupted.";

    struct key_data key_tab [] = {
      { "ShtDsc", TRUE,  (uiptr) &def_sdesc,	{ &ch->short_descr,   NULL } },
      { "LngDsc", TRUE,  (uiptr) &def_ldesc,	{ &ch->long_descr,    NULL } },
      { "Dscr",   TRUE,  (uiptr) &def_desc,	{ &ch->description,   NULL } },
      { "Prmpt",  TRUE,  (uiptr) &daPrompt,	{ &ch->pcdata->prompt,NULL } },
      { "Sx",     FALSE, SEX_MALE,		{ &ch->sex,           NULL } },
      { "Lvl",    FALSE, MAND,			{ &ch->level,         NULL } },
      { "Trst",   FALSE, 0,			{ &ch->trust,         NULL } },
      { "Playd",  FALSE, 0,			{ &ch->played,        NULL } },
      { "HpMnMv", FALSE, MAND,			{ &ch->hit,
						  &ch->max_hit,
						  &ch->mana,
						  &ch->max_mana,
						  &ch->move,
						  &ch->max_move,      NULL } },
      { "Gold",   FALSE, 0,			{ &ch->gold,          NULL } },
      { "Exp",    FALSE, MAND,			{ &ch->exp,           NULL } },
      { "Res",    FALSE, 0,			{ &ch->resistant,     NULL } },
      { "Imm",    FALSE, 0,			{ &ch->immune,        NULL } },
      { "Susc",   FALSE, 0,			{ &ch->susceptible,   NULL } },
      { "Langs",  FALSE, 0,			{ &ch->speaks,
						  &ch->speaking,      NULL } },
      { "Pos",    FALSE, POS_STANDING, 		{ &ch->position,      NULL } },
      { "Prac",   FALSE, MAND,			{ &ch->practice,      NULL } },
      { "SavThr", FALSE, MAND,			{ &ch->saving_throw,  NULL } },
      { "Align",  FALSE, 0,			{ &ch->alignment,     NULL } },
      { "Hit",    FALSE, MAND,			{ &ch->hitroll,       NULL } },
      { "Dam",    FALSE, MAND,			{ &ch->damroll,       NULL } },
      { "Armr",   FALSE, MAND,			{ &ch->armor,         NULL } },
      { "Wimp",   FALSE, 10,			{ &ch->wimpy,         NULL } },
      { "Deaf",   FALSE, 0,			{ &ch->deaf,          NULL } },
      { "Paswd",  TRUE,  MAND,			{ &ch->pcdata->pwd,   NULL } },
      { "Bmfin",  TRUE,  DEFLT,			{ &ch->pcdata->bamfin,
						                      NULL } },
      { "Bmfout", TRUE,  DEFLT,			{ &ch->pcdata->bamfout,
						                      NULL } },
      { "Immskll",TRUE,  DEFLT,			{ &ch->pcdata->immskll,
						                      NULL } },
      { "Wiznet", FALSE,  0,			{ &ch->pcdata->wiznet,
						                      NULL } },
      { "Ttle",   TRUE,  (uiptr) &def_title,	{ &ch->pcdata->title, NULL } },
      { "AtrPrm", FALSE, MAND,			{ &ch->perm_str,
						  &ch->perm_int,
						  &ch->perm_wis,
						  &ch->perm_dex,
						  &ch->perm_con,
						                      NULL } },
      { "AtrMd",  FALSE, MAND,			{ &ch->mod_str,
						  &ch->mod_int,
						  &ch->mod_wis,
						  &ch->mod_dex,
						  &ch->mod_con,
						                      NULL } },
      { "Cond",   FALSE, DEFLT,			{ &ch->pcdata->condition [0],
						  &ch->pcdata->condition [1],
						  &ch->pcdata->condition [2],
						                      NULL } },
      { "Security", FALSE, DEFLT,             { &ch->pcdata->security,
								      NULL } },
      { "PKills",	FALSE, 0,		{ &ch->pcdata->pkills,
						                      NULL } },
      { "PDeaths",	FALSE, 0,		{ &ch->pcdata->pdeaths,
						                      NULL } },
      { "IllegalPK",	FALSE, 0,		{ &ch->pcdata->illegal_pk,
						                      NULL } },
      { "MKills",	FALSE, 0,		{ &ch->pcdata->mkills,
						                      NULL } },
      { "MDeaths",	FALSE, 0,		{ &ch->pcdata->mdeaths,
						                      NULL } },
      { "Pglen",  FALSE, 20,			{ &ch->pcdata->pagelen,
						                      NULL } },
      { "\0",     FALSE, 0                                                 } };


    for ( num_keys = 0; *key_tab [num_keys].key; )
        num_keys++;

    for ( ;!feof( fp ) ; )
    {

        word = fread_word( fp, &status );

        if ( !word )
	{
	    switch ( status )
	    {
	    case  2:
		bugf( "Fread_char:  %s: EOF.", ch->name );
		break;
	    default:
		bugf( "Fread_char:  %s: EOF?", ch->name );
		fread_to_eol( fp );
		break;
	    }
            break;
	}

                /* This little diddy searches for the keyword
                   from the last keyword found */

        for ( i = last_key;
              i < last_key + num_keys &&
                str_cmp (key_tab [i % num_keys].key, word); )
            i++;

        i = i % num_keys;

        if ( !str_cmp (key_tab [i].key, word) )
            last_key = i;
        else
            i = num_keys;

        if ( *key_tab [i].key )         /* Key entry found in key_tab */
	{
            if ( key_tab [i].string == SPECIFIED )
                bugf( "Key '%s' already specified.", key_tab[i].key );

                        /* Entry is a string */

            else
	      if ( key_tab [i].string )
	      {
                  if ( ( p = fread_string( fp, &status ) ) && !status )
		  {
		      free_string ( *(char **)key_tab [i].ptrs [0] );
		      *(char **)key_tab [i].ptrs [0] = p;
		  }
	      }

                        /* Entry is an integer */
            else
                for ( j = 0; key_tab [i].ptrs [j]; j++ )
		{
                    tmpi = fread_number ( fp, &status );
                    if ( !status )
                        *(int *)key_tab [i].ptrs [j] = tmpi;
		}

            if ( status )
	    {
                fread_to_eol( fp );
                continue;
	    }
	    else
                key_tab [i].string = SPECIFIED;
	}

        else if ( *word == '*' || !str_cmp( word, "Nm" ) )
            fread_to_eol( fp );

        else if ( !str_cmp( word, "End" ) )
            break;

        else if ( !str_cmp( word, "Act" ) )
	  {
	      ch->act = fread_exbv( fp, &status );
	  }

        else if ( !str_cmp( word, "ActF" ) )
	  {
	      ch->act = fread_xbv( fp, plr_flags );
	  }

        else if ( !str_cmp( word, "AffdBy" ) )
	  {
	      ch->affected_by = fread_exbv( fp, &status );
	  }

        else if ( !str_cmp( word, "AffectedBy" ) )
	  {
	      ch->affected_by = fread_xbv( fp, affect_flags );
	  }

        else if ( !str_cmp( word, "Board" ) )
	  {
	      i  = fread_number( fp, &status );
	      sn = board_lookup( fread_word( fp, &status1 ) );

	      if ( status || status1 )
	      {
		  bugf( "Fread_char: error reading board." );
		  fread_to_eol( fp );
		  continue;
	      }

	      if ( sn == -1 )
                  bugf( "Fread_char: unknown board." );
	      else
                  ch->pcdata->last_note[sn] = i;
	  }

        else if ( !str_cmp( word, "Class" ) )
	  {
	      for ( i = 0; i < MAX_MULTICLASS; i++ )
		ch->cclass[i] = class_lookup( temp_fread_string( fp, &status ) );
	  }

        else if ( !str_cmp( word, "Room" ) )
	  {
	      ch->in_room = get_room_index( fread_number( fp, &status ) );
	      if ( !ch->in_room )
                  ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
	  }

	else if ( !str_cmp( word, "Race" ) )
	  {
	      char *race_name;

	      race_name = fread_string( fp, &status );

	      if ( status )
	      {
		  bugf( "Fread_char: error reading Race." );
		  fread_to_eol( fp );
		  continue;
	      }

	      i  = race_lookup( race_name );

	      if ( i == -1 )
		  bugf( "Fread_char: unknown Race: %s.", race_name );
	      else
		  ch->race = i;
	  }

        else if ( !str_cmp( word, "PClan" ) )
	  {
	      char *clan_name;

	      ch->pcdata->rank      = fread_number( fp, &status  );
	      clan_name             = fread_string( fp, &status1 );

	      if ( status || status1 )
	      {
		  bugf( "Fread_char: error reading PClan." );
		  fread_to_eol( fp );
		  continue;
	      }

	      if ( !clan_name
		  || !( ch->pcdata->clan = get_clan( clan_name ) ) )
	      {
		  bugf( "Fread_char: unknown PClan: %s.", clan_name );
		  ch->pcdata->rank      = 0;
	      }
	  }

	else if ( !str_cmp( word, "NAlias" ) )
	  {
	      ALIAS_DATA *alias;
	      int         num;

	      num = 0;

	      for ( alias = ch->pcdata->alias_list; alias; alias = alias->next )
		  num++;

	      if ( num >= MAX_ALIAS )
		  continue;

	      alias                  = (ALIAS_DATA *) alloc_mem( sizeof( ALIAS_DATA ) );
	      alias->cmd             = str_dup( fread_word( fp, &status1 ) );
	      alias->subst           = fread_string( fp, &status );

	      if ( status || status1 )
	      {
		  bugf( "Fread_char: Error reading NAlias." );
		  fread_to_eol( fp );
		  continue;
	      }

	      alias->next            = ch->pcdata->alias_list;
	      ch->pcdata->alias_list = alias;
	  }

        else if ( !str_cmp( word, "Skll" ) )
	  {
	      char *sname;

              i     = fread_number( fp, &status );
	      sname = fread_word  ( fp, &status1 );
	      
	      if ( status || status1 )
	      {
		  bugf( "Fread_char: Error reading skill." );
		  fread_to_eol( fp );
		  continue;
	      }

	      sn = skill_lookup( sname );

	      if ( sn == -1 )
                  bugf( "Fread_char: unknown skill: %s.", sname );
	      else
                  ch->pcdata->learned[sn] = i;
	  }

	else if ( !str_cmp ( word, "Afft" ) )
	  {
	      int   status;
	      char *buf1;

	      paf	= new_affect();
	      buf1	= temp_fread_string( fp, &status );
	      paf->type	= affect_lookup( buf1 );

	      if ( paf->type < 0 )
	      {
		  paf->next	  = affect_free;
		  affect_free	  = paf;

		  bugf( "Fread_char: Error reading Afft %s.", buf1 );

		  fread_to_eol( fp );
		  continue;
	      }

	      paf->duration       = fread_number( fp, &status );
	      paf->modifier       = fread_number( fp, &status );
	      paf->location       = fread_number( fp, &status );
	      paf->bitvector	  = fread_exbv( fp, &status );
	      paf->deleted        = FALSE;
	      paf->next           = ch->affected;
	      ch->affected        = paf;
	  }

	else if ( !str_cmp ( word, "Affect" ) )
	  {
	      int   status;
	      char *buf1;

	      paf	= new_affect();
	      buf1	= temp_fread_string( fp, &status );
	      paf->type	= affect_lookup( buf1 );

	      if ( paf->type < 0 )
	      {
		  paf->next	  = affect_free;
		  affect_free	  = paf;

		  bugf( "Fread_char: Error reading Affect %s.", buf1 );

		  fread_to_eol( fp );
		  continue;
	      }

	      paf->duration       = fread_number( fp, &status );
	      paf->modifier       = fread_number( fp, &status );
	      paf->location       = fread_flag( fp, apply_flags );
	      paf->bitvector      = fread_xbv( fp, affect_flags );
	      paf->deleted        = FALSE;
	      paf->next           = ch->affected;
	      ch->affected        = paf;
	  }

        else
	{
	    bugf( "Fread_char: Unknown key '%s' in pfile.", word );
	    fread_to_eol( fp );
	}
	
    }

                /* Require all manditory fields, set defaults */

    for ( i = 0; *key_tab [i].key; i++ )
    {

        if ( key_tab [i].string == SPECIFIED ||
             key_tab [i].deflt == DEFLT )
            continue;

        if ( key_tab [i].deflt == MAND )
	{
            bugf( "Field '%s' missing from %s.", key_tab[i].key, ch->name );
            error_count++;
            continue;
	}

               /* This if/else sets default strings and numbers */

        if ( key_tab [i].string && key_tab [i].deflt )
	{
	    free_string( *(char **)key_tab [i].ptrs [0] );
            *(char **)key_tab [i].ptrs [0] =
	      str_dup( (char *)key_tab [i].deflt );
	}
        else
            for ( j = 0; key_tab [i].ptrs [j]; j++ )
	        *(int *)key_tab [i].ptrs [j] = key_tab [i].deflt;
    }

                /* Fixups */

    if ( ch->pcdata->title && isalnum ( *ch->pcdata->title ) )
    {
        sprintf( buf, " %s", ch->pcdata->title );
        free_string( ch->pcdata->title );
        ch->pcdata->title = str_dup( buf );
    }

    return error_count;

}

void recover( FILE *fp, long fpos )
{
    char        buf[ MAX_STRING_LENGTH ];

    fseek( fp, fpos, 0 );

    while ( !feof ( fp ) )
    {
        fpos = ftell( fp );

        if ( !fgets( buf, MAX_STRING_LENGTH, fp ) )
            return;

        if ( !strncmp( buf, "#OBJECT", 7 ) ||
	     !strncmp( buf, "#OBJ", 4 ) ||
             !strncmp( buf, "#END", 4 ) )
	{
            fseek( fp, fpos, 0 );
            return;
	}
    }
}

int envy_fread_obj( CHAR_DATA *ch, FILE *fp )
{
    EXTRA_DESCR_DATA *ed;
    OBJ_DATA         obj;
    OBJ_DATA         *new_obj;
    AFFECT_DATA      *paf;
    char             *spell_name = NULL;
    char             *p          = NULL;
    char             *word;
    char             *tmp_ptr;
    bool              fNest;
    bool              fVnum;
    long              fpos;
    int               iNest;
    int               iValue;
    int               status;
    int               sn;
    int               vnum;
    int               num_keys;
    int               last_key   = 0;
    int               i, j, tmpi;

    char              corobj [] = "This object was corrupted.";

    struct key_data key_tab [] =
      {
	{ "Name",        TRUE,  MAND,             { &obj.name,        NULL } },
	{ "ShortDescr",  TRUE,  (uiptr) &corobj,  { &obj.short_descr, NULL } },
	{ "Description", TRUE,  (uiptr) &corobj,  { &obj.description, NULL } },
	{ "ExtraFlags",  FALSE, MAND,             { &obj.extra_flags, NULL } },
	{ "WearFlags",   FALSE, MAND,             { &obj.wear_flags,  NULL } },
	{ "WearLoc",     FALSE, MAND,             { &obj.wear_loc,    NULL } },
	{ "ItemType",    FALSE, MAND,             { &obj.item_type,   NULL } },
	{ "Weight",      FALSE, 10,               { &obj.weight,      NULL } },
	{ "Level",       FALSE, ch->level,        { &obj.level,       NULL } },
	{ "Timer",       FALSE, 0,                { &obj.timer,       NULL } },
	{ "Cost",        FALSE, 300,              { &obj.cost,        NULL } },
	{ "Values",      FALSE, MAND,             { &obj.value [0],
						    &obj.value [1],
						    &obj.value [2],
						    &obj.value [3],   NULL } },
	{ "\0",          FALSE, 0                                          } };

    memset( &obj, 0, sizeof( OBJ_DATA ) );

    obj.name        = str_dup( "" );
    obj.short_descr = str_dup( "" );
    obj.description = str_dup( "" );
    obj.deleted     = FALSE;

    fNest           = FALSE;
    fVnum           = TRUE;
    iNest           = 0;

    new_obj = new_object ();

    for ( num_keys = 0; *key_tab [num_keys].key; )
        num_keys++;

    for ( fpos = ftell( fp ) ; !feof( fp ) ; )
    {

        word = fread_word( fp, &status );

        for ( i = last_key;
              i < last_key + num_keys &&
                str_cmp( key_tab [i % num_keys].key, word ); )
            i++;

        i = i % num_keys;

        if ( !str_cmp( key_tab [i].key, word ) )
            last_key = i + 1;
        else
            i = num_keys;

        if ( *key_tab [i].key )         /* Key entry found in key_tab */
	{
            if ( key_tab [i].string == SPECIFIED )
                bugf( "Key '%s' already specified.", key_tab[i].key );

                        /* Entry is a string */

            else if ( key_tab [i].string )
	    {
                if ( ( p = fread_string( fp, &status ) ) && !status )
		{
                   free_string ( * (char **) key_tab [i].ptrs [0] );
                   * (char **) key_tab [i].ptrs [0] = p;
		}
	    }

                        /* Entry is an integer */
            else
                for ( j = 0; key_tab [i].ptrs [j]; j++ )
		{
                    tmpi = fread_number( fp, &status );
                    if ( !status )
                        * (int *) key_tab [i].ptrs [j] = tmpi;
		}

            if ( status )
	    {
                fread_to_eol( fp );
                continue;
	    }
	    else
                key_tab [i].string = SPECIFIED;
	}

        else if ( *word == '*' )
            fread_to_eol( fp );

        else if ( !str_cmp( word, "End" ) )
	{
            if ( !fNest || !fVnum )
	    {
                bugf( "Fread_obj: incomplete object." );

		recover    ( fp, fpos        );
		free_string( obj.name        );
		free_string( obj.short_descr );
		free_string( obj.description );
		extract_obj( new_obj         );

		return FALSE;
	    }
            break;
	}

        else if ( !str_cmp( word, "Nest" ) )
	{

            iNest = fread_number( fp, &status );

            if ( status )       /* Losing track of nest level is bad */
                iNest = 0;      /* This makes objs go to inventory */

            else if ( iNest < 0 || iNest >= MAX_NEST )
                bugf( "Fread_obj: bad nest %d.", iNest );

            else
	    {
                rgObjNest[iNest] = new_obj;
                fNest = TRUE;
	    }
	}

        else if ( !str_cmp( word, "Spell" ) )
	{

            iValue = fread_number( fp, &status );

            if ( !status )
                spell_name = fread_word( fp, &status );

            if ( status )       /* Recover is to skip spell */
	    {
                fread_to_eol( fp );
                continue;
	    }

            sn = skill_lookup( spell_name );

            if ( iValue < 0 || iValue > 3 )
                bugf( "Fread_obj: bad iValue %d.", iValue );

            else if ( sn == -1 )
                bugf( "Fread_obj: unknown spell: %s.", spell_name );

            else
                obj.value[iValue] = sn;
	}

        else if ( !str_cmp( word, "Vnum" ) )
	{

            vnum = fread_number( fp, &status );

            if ( status )               /* Can't live without vnum */
	    {
		recover    ( fp, fpos        );
		free_string( obj.name        );
		free_string( obj.short_descr );
		free_string( obj.description );
		extract_obj( new_obj         );
		return FALSE;
	    }

            if ( !( obj.pIndexData = get_obj_index( vnum ) ) )
                bugf( "Fread_obj: bad vnum %d.", vnum );
            else
                fVnum = TRUE;
	}

                /* The following keys require extra processing */

        if ( !str_cmp( word, "Affect" ) )
	{
            paf = new_affect ();

	    paf->type       = fread_number( fp, &status );
	    paf->duration   = fread_number( fp, &status );
	    paf->modifier   = fread_number( fp, &status );
	    paf->location   = fread_number( fp, &status );
	    paf->bitvector  = fread_exbv( fp, &status );

            paf->next = obj.affected;
            obj.affected = paf;
	}

        else if ( !str_cmp( word, "ExtraDescr" ) )
	{
	    tmp_ptr = fread_string( fp, &status );

            if ( !status )
                p = fread_string( fp, &status );

            if ( status )
	    {
		recover    ( fp, fpos        );
		free_string( obj.name        );
		free_string( obj.short_descr );
		free_string( obj.description );
		extract_obj( new_obj         );
		return FALSE;
	    }

            ed = new_extra_descr ();

            ed->keyword     = tmp_ptr;
            ed->description = p;
            ed->next        = obj.extra_descr;
            obj.extra_descr = ed;
	}
    }
                /* Require all manditory fields, set defaults */

    for ( i = 0; *key_tab [i].key; i++ )
    {

        if ( key_tab [i].string == SPECIFIED ||
             key_tab [i].deflt == DEFLT )
            continue;

        if ( key_tab [i].deflt == MAND )
	{
            bugf( "Obj field '%s' missing from %s.", key_tab[i].key, ch->name );

	    recover    ( fp, fpos        );
	    free_string( obj.name        );
	    free_string( obj.short_descr );
	    free_string( obj.description );
	    extract_obj( new_obj         );

	    return FALSE;
	}

                /* This if/else sets default strings and numbers */

        if ( key_tab [i].string && key_tab [i].deflt )
            * (char **) key_tab [i].ptrs [0] =
                        str_dup ( (char *) key_tab [i].deflt );
        else
            for ( j = 0; key_tab [i].ptrs [j]; j++ )
                * (int *) key_tab [i].ptrs [j] = key_tab [i].deflt;
    }

    memcpy( new_obj, &obj, sizeof( OBJ_DATA ) );

    new_obj->next = object_list;
    object_list   = new_obj;

    new_obj->pIndexData->count++;
    if ( iNest == 0 || !rgObjNest[iNest] )
        obj_to_char( new_obj, ch );
    else
        obj_to_obj( new_obj, rgObjNest[iNest-1] );

    return TRUE;
}

int fread_obj( CHAR_DATA *ch, FILE *fp )
{
    EXTRA_DESCR_DATA *ed;
    OBJ_DATA         obj;
    OBJ_DATA         *new_obj;
    AFFECT_DATA      *paf;
    char             *spell_name = NULL;
    char             *p          = NULL;
    char             *word;
    char             *tmp_ptr;
    char             *special;
    bool              fNest;
    bool              fVnum;
    long              fpos;
    int               iNest;
    int               iValue;
    int               status;
    int               sn;
    int               vnum;
    int               num_keys;
    int               last_key   = 0;
    int               i, j, tmpi;

    char              corobj [] = "This object was corrupted.";

    struct key_data key_tab [] =
      {
	{ "Name",        TRUE,  MAND,             { &obj.name,        NULL } },
	{ "ShortDescr",  TRUE,  (uiptr) &corobj,  { &obj.short_descr, NULL } },
	{ "Description", TRUE,  (uiptr) &corobj,  { &obj.description, NULL } },
	{ "WearFlags",   FALSE, MAND,             { &obj.wear_flags,  NULL } },
	{ "WearLoc",     FALSE, MAND,             { &obj.wear_loc,    NULL } },
	{ "ItemType",    FALSE, MAND,             { &obj.item_type,   NULL } },
	{ "Weight",      FALSE, 10,               { &obj.weight,      NULL } },
	{ "Level",       FALSE, ch->level,        { &obj.level,       NULL } },
	{ "Timer",       FALSE, 0,                { &obj.timer,       NULL } },
	{ "Cost",        FALSE, 300,              { &obj.cost,        NULL } },
	{ "Values",      FALSE, MAND,             { &obj.value [0],
						    &obj.value [1],
						    &obj.value [2],
						    &obj.value [3],
						    &obj.value [4],   NULL } },
	{ "\0",          FALSE, 0                                          } };

    memset( &obj, 0, sizeof( OBJ_DATA ) );

    obj.name        = str_dup( "" );
    obj.short_descr = str_dup( "" );
    obj.description = str_dup( "" );
    obj.deleted     = FALSE;

    fNest           = FALSE;
    fVnum           = TRUE;
    iNest           = 0;

    new_obj = new_object ();

    for ( num_keys = 0; *key_tab [num_keys].key; )
        num_keys++;

    for ( fpos = ftell( fp ) ; !feof( fp ) ; )
    {

        word = fread_word( fp, &status );

        for ( i = last_key;
              i < last_key + num_keys &&
                str_cmp( key_tab [i % num_keys].key, word ); )
            i++;

        i = i % num_keys;

        if ( !str_cmp( key_tab [i].key, word ) )
            last_key = i + 1;
        else
            i = num_keys;

        if ( *key_tab [i].key )         /* Key entry found in key_tab */
	{
            if ( key_tab [i].string == SPECIFIED )
                bugf( "Key '%s' already specified.", key_tab[i].key );

                        /* Entry is a string */

            else if ( key_tab [i].string )
	    {
                if ( ( p = fread_string( fp, &status ) ) && !status )
		{
                   free_string ( * (char **) key_tab [i].ptrs [0] );
                   * (char **) key_tab [i].ptrs [0] = p;
		}
	    }

                        /* Entry is an integer */
            else
                for ( j = 0; key_tab [i].ptrs [j]; j++ )
		{
                    tmpi = fread_number( fp, &status );
                    if ( !status )
                        * (int *) key_tab [i].ptrs [j] = tmpi;
		}

            if ( status )
	    {
                fread_to_eol( fp );
                continue;
	    }
	    else
                key_tab [i].string = SPECIFIED;
	}

        else if ( *word == '*' )
            fread_to_eol( fp );

        else if ( !str_cmp( word, "End" ) )
	{
            if ( !fNest || !fVnum )
	    {
                bugf( "Fread_obj: incomplete object." );

		recover    ( fp, fpos        );
		free_string( obj.name        );
		free_string( obj.short_descr );
		free_string( obj.description );
		extract_obj( new_obj         );

		return FALSE;
	    }
            break;
	}

        else if ( !str_cmp( word, "Nest" ) )
	{

            iNest = fread_number( fp, &status );

            if ( status )       /* Losing track of nest level is bad */
                iNest = 0;      /* This makes objs go to inventory */

            else if ( iNest < 0 || iNest >= MAX_NEST )
                bugf( "Fread_obj: bad nest %d.", iNest );

            else
	    {
                rgObjNest[iNest] = new_obj;
                fNest = TRUE;
	    }
	}

        else if ( !str_cmp( word, "Special" ) )
	{
	    special = fread_word( fp, &status );

	    if ( !status )
		obj.spec_fun = spec_obj_lookup( special );

            if ( status )
	    {
                fread_to_eol( fp );
                continue;
	    }

	}

        else if ( !str_cmp( word, "Spell" ) )
	{
            iValue = fread_number( fp, &status );

            if ( !status )
                spell_name = fread_word( fp, &status );

            if ( status )       /* Recover is to skip spell */
	    {
                fread_to_eol( fp );
                continue;
	    }

            sn = skill_lookup( spell_name );

            if ( iValue < 0 || iValue > 4 )
                bugf( "Fread_obj: bad iValue %d.", iValue );

            else if ( sn == -1 )
                bugf( "Fread_obj: unknown spell: %s.", spell_name );

            else
                obj.value [iValue] = sn;
	}

        else if ( !str_cmp( word, "Vnum" ) )
	{

            vnum = fread_number( fp, &status );

            if ( status )               /* Can't live without vnum */
	    {
		recover    ( fp, fpos        );
		free_string( obj.name        );
		free_string( obj.short_descr );
		free_string( obj.description );
		extract_obj( new_obj         );
		return FALSE;
	    }

            if ( !( obj.pIndexData = get_obj_index( vnum ) ) )
                bugf( "Fread_obj: bad vnum %d.", vnum );
            else
                fVnum = TRUE;
	}

                /* The following keys require extra processing */

        if ( !str_cmp( word, "Affect" ) )
	{
            paf = new_affect ();

	    paf->type       = fread_number( fp, &status );
	    paf->duration   = fread_number( fp, &status );
	    paf->modifier   = fread_number( fp, &status );
	    paf->location   = fread_number( fp, &status );
	    paf->bitvector  = fread_xbv( fp, affect_flags );
            paf->next       = obj.affected;
            obj.affected    = paf;
	}

        else if ( !str_cmp( word, "ExtraFlags" ) )
          {
              obj.extra_flags = fread_xbv( fp, extra_flags );
          }

	else if ( !str_cmp( word, "ExtraDescr" ) )
	{
	    tmp_ptr = fread_string( fp, &status );

            if ( !status )
                p = fread_string( fp, &status );

            if ( status )
	    {
		recover    ( fp, fpos        );
		free_string( obj.name        );
		free_string( obj.short_descr );
		free_string( obj.description );
		extract_obj( new_obj         );
		return FALSE;
	    }

            ed = new_extra_descr ();

            ed->keyword     = tmp_ptr;
            ed->description = p;
            ed->next        = obj.extra_descr;
            obj.extra_descr = ed;
	}
    }
                /* Require all manditory fields, set defaults */

    for ( i = 0; *key_tab [i].key; i++ )
    {

        if ( key_tab [i].string == SPECIFIED ||
             key_tab [i].deflt == DEFLT )
            continue;

        if ( key_tab [i].deflt == MAND )
	{
            bugf( "Obj field '%s' missing from %s.", key_tab[i].key, ch->name );

	    recover    ( fp, fpos        );
	    free_string( obj.name        );
	    free_string( obj.short_descr );
	    free_string( obj.description );
	    extract_obj( new_obj         );

	    return FALSE;
	}

                /* This if/else sets default strings and numbers */

        if ( key_tab [i].string && key_tab [i].deflt )
            * (char **) key_tab [i].ptrs [0] =
                        str_dup ( (char *) key_tab [i].deflt );
        else
            for ( j = 0; key_tab [i].ptrs [j]; j++ )
                * (int *) key_tab [i].ptrs [j] = key_tab [i].deflt;
    }

    memcpy( new_obj, &obj, sizeof( OBJ_DATA ) );

    new_obj->next = object_list;
    object_list   = new_obj;

    new_obj->pIndexData->count++;
    if ( iNest == 0 || !rgObjNest[iNest] )
        obj_to_char( new_obj, ch );
    else
        obj_to_obj( new_obj, rgObjNest[iNest-1] );

    return TRUE;
}
