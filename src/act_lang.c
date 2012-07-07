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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

typedef	struct	syl_type		SYL_TYPE;

struct syl_type
{
     SYL_TYPE *		next;
     char *		old_s;
     char *		new_s;
};


struct	syl_type	*syl_table	[ MAX_LANGUAGE ][ MAX_WORD_HASH ];



const	struct	lang_type	lang_table	[MAX_LANGUAGE]	=
{
    {  "common",	LANG_COMMON	},  /*  0   */
    {  "elven",		LANG_ELVEN	},  /*  1   */
    {  "dwarven",	LANG_DWARVEN	},
    {  "pixie",		LANG_PIXIE	},
    {  "ogre",		LANG_OGRE	},
    {  "orcish",	LANG_ORCISH	},  /*  5   */
    {  "trollese",	LANG_TROLLESE	},
    {  "rodent",	LANG_RODENT	},
    {  "insectoid",	LANG_INSECTOID	},
    {  "mammal",	LANG_MAMMAL	},
    {  "reptile",	LANG_REPTILE	},  /*  10   */
    {  "dragon",	LANG_DRAGON	},
    {  "spiritual",	LANG_SPIRITUAL	},
    {  "magical",	LANG_MAGICAL	},
    {  "goblin",	LANG_GOBLIN	},
    {  "god",		LANG_GOD	},  /*  15   */
    {  "halfling",	LANG_HALFLING	},
    {  "gith",		LANG_GITH	},
    {  "drow",		LANG_DROW	},
    {  "kobold",	LANG_KOBOLD	},
    {  "gnomish",	LANG_GNOMISH	},  /*  20   */
    {  "plant",		LANG_PLANT	},
    {  "fish",		LANG_FISH	},
    {  "clan",		LANG_CLAN	}
};



/*
 * Lookup a language by name.
 */
int syl_lookup( const char *name )
{
    int ln;

    if ( !name )
	return -1;

    for ( ln = 0; ln < MAX_LANGUAGE; ln++ )
    {
	if ( !lang_table[ln].name )
	    break;
	if ( LOWER( name[0] ) == LOWER( lang_table[ln].name[0] )
	    && !str_prefix( name, lang_table[ln].name ) )
	    return ln;
    }

    return -1;
}

const char *lang_bit_name( int vector )
{
    static char buf [ 512 ];
           int  ln;

    buf[0] = '\0';
    for ( ln = 0; ln < MAX_LANGUAGE; ln++ )
    {
	if ( IS_SET( vector, lang_table[ln].value ) )
	{
	    strcat( buf, " " );
	    strcat( buf, lang_table[ln].name );
	}
    }

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

void fread_language( FILE *fp )
{
    char       *str1;
    char       *str2;
    const char *word;
    SYL_TYPE   *syl;
    int         status1;
    int         status2;
    int         hash;
    int         ln        = 0;
    
    for ( ; ; )
    {
    	if ( !( word = fread_word( fp, &status1 ) ) )
	    word = "$";

	switch ( UPPER( word[0] ) )
	{
	case 'D':
	    str1 = fread_string( fp, &status1 );
	    str2 = fread_string( fp, &status2 );

	    if ( status1 || status2 )
	    {
		bug( "Fread_language: Error!", 0 );
		return;
	    }

	    syl = (SYL_TYPE *) alloc_perm( sizeof( SYL_TYPE ) );

	    syl->old_s = str1;
	    syl->new_s = str2;

	    if ( LOWER( str1[0] ) < 'a' || LOWER( str1[0] ) > 'z' )
		hash = 0;
	    else
		hash = ( LOWER( str1[0] ) - 'a' ) + 1;

	    syl->next           = syl_table[ln][hash];
	    syl_table[ln][hash] = syl;
	    break;

        case 'L':
	    if ( ( ln = syl_lookup( fread_word( fp, &status1 ) ) ) == -1 )
	    {
		bug( "Fread_language: unknown language.", 0 );
		return;
	    }
	    break;

        case '$':
	    return;
	    break;

        case '*':
	    fread_to_eol( fp );
	    break;
	}
    }

    return;
}

/*
 * Load in all the language dictionaries.
 */ 
void load_languages( void )
{
    FILE       *fp;
    FILE       *fpList;
    const char *word;
    char        strsave	[ MAX_STRING_LENGTH ];
    int         stat;
    
    log_string( "Loading languages" );

    sprintf( strsave, "%s%s", DICT_DIR, LANG_LIST );
    if ( !( fpList = fopen( strsave, "r" ) ) )
    {
        perror( strsave );
        exit( 1 );
    }

    for ( ; ; )
    {
    	if ( !( word = fread_word( fpList, &stat ) ) )
	    word = "$";

        if ( word[0] == '$' )
          break;

	sprintf( strsave, "%s%s", DICT_DIR, word );
	if ( !( fp = fopen( strsave, "r" ) ) )
	{
	    perror( strsave );
	    exit( 1 );
	}

	fprintf( stderr, "." );
	fread_language( fp );
	fclose( fp );
    }
    fclose( fpList );

    fprintf( stderr, "\n" );
    return;
}

void save_language( int ln )
{
    FILE      *fp;
    char       strsave  [ MAX_STRING_LENGTH ];
    int        i;
    SYL_TYPE  *iSyl;

    sprintf( strsave, "%s%s.dic", DICT_DIR, lang_table[ln].name );

    fclose( fpReserve );

    if ( !( fp = fopen( strsave, "w" ) ) )
    {
        bugf( "Cannot open: %s.dic for writing", lang_table[ln].name );
    }
    else
    {
	fprintf( fp, "L %s\n",		lang_table[ln].name	);

	for ( i = 0; i < MAX_WORD_HASH; i++ )
	    for ( iSyl = syl_table[ln][i]; iSyl; iSyl = iSyl->next )
		fprintf( fp, "D %s~ %s~\n",	iSyl->old_s,
						iSyl->new_s	);

	fprintf( fp, "$\n" );

	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void save_languages( void )
{
    int ln;

    for ( ln = 0; ln < MAX_LANGUAGE; ln++ )
	save_language( ln );

    return;
}

char *translate( char *str, int ln, CHAR_DATA *vch, CHAR_DATA *ch )
{
	   char       buf     [ MAX_STRING_LENGTH ];
           SYL_TYPE  *iSyl;
           int        hash;
           int        length;
           int        fMatch;

    buf[0]	= '\0';
    length	= 0;

    for ( ; *str != '\0'; str += length )
    {
	if ( LOWER( str[0] ) < 'a' || LOWER( str[0] ) > 'z' )
	    hash = 0;
	else
    	    hash = ( LOWER( str[0] ) - 'a' ) + 1;
	    
	fMatch = FALSE;

	if ( !knows_language( vch, ln, ch ) )
	    for ( iSyl = syl_table[ln][hash]; iSyl; iSyl = iSyl->next )
	    {
	    	length = strlen( iSyl->old_s );

	    	if ( !str_prefix( iSyl->old_s, str ) )
	    	{
		    fMatch = TRUE;
		    strcat( buf, iSyl->new_s );
		    break;
	    	}
	    }

	if ( !fMatch )
	{
	    sprintf( buf + strlen( buf ), "%c", str[0] );
	    length = 1;
	    continue;
	}

	if ( length == 0 )
	    length = 1;
    }
    buf[MAX_INPUT_LENGTH-1] = '\0';

    strcpy( str, buf );
    return str;
}

void do_speak( CHAR_DATA *ch, char *argument )
{
    int   ln;
    char  buf [ MAX_STRING_LENGTH ];

    if ( ( ln = syl_lookup( argument ) ) == -1
	|| !speaks_language( ch, ln, ch )
	|| ( lang_table[ln].value == LANG_CLAN
	    && !is_clan( ch )
	    && !IS_NPC ( ch ) ) )
    {
 	send_to_char( "You don't know that language.\n\r", ch );
	return;
    }

    ch->speaking = ln;

    sprintf( buf, "You now speak %s.\n\r", lang_table[ln].name );
    send_to_char( buf, ch );
    return;
}

bool knows_language( CHAR_DATA *vch, int language, CHAR_DATA *ch )
{
    if ( IS_SET( race_table[vch->race].language, lang_table[language].value ) )
      	return TRUE;
    if ( IS_SET( vch->speaks, lang_table[language].value ) )
	return TRUE;
    if ( IS_SET( lang_table[language].value, LANG_COMMON ) )
	return TRUE;
    if ( IS_SET( lang_table[language].value, LANG_CLAN ) )
    {
    	if ( IS_NPC( vch ) || IS_NPC( ch ) )
    	    return TRUE;
	return is_same_clan( vch, ch );
    }
    if ( !IS_NPC( ch ) )
    {
	int sn;

	if ( IS_SET( vch->speaks, lang_table[language].value ) )
	{
	    if ( ( sn = skill_blookup( lang_table[language].name,
			MAX_SPELL + 1, MAX_SKILL ) ) != -1
		&& number_percent( ) < ch->pcdata->learned[sn] )
	    {
      	      	return TRUE;
	    }
	}
    }

    return FALSE;
}

/*
 * Langconv by Aioros.
 */
void langconv( char *buf1, const char *txt, CHAR_DATA *ch )
{
    int i = 0, j = 0;
    int lang = -1;
    int size;
    char *str, buf2[ MAX_STRING_LENGTH ];

    buf1[0] = '\0';
    buf2[0] = '\0';
    size = strlen( txt );

    while ( j <= size )
    {
    	if ( txt[j] == '^' )
    	{
    	    switch ( txt[++j] )
    	    {
    	    case 'c': lang = syl_lookup( "common" );	break;
    	    case 'e': lang = syl_lookup( "elven" ); 	break;
    	    case 'd': lang = syl_lookup( "dwarven" );	break;
    	    case 'p': lang = syl_lookup( "pixie" ); 	break;
    	    case 'o': lang = syl_lookup( "ogre" );  	break;
    	    case 'O': lang = syl_lookup( "orcish" );	break;
    	    case 't': lang = syl_lookup( "trollese" );	break;
    	    case 'r': lang = syl_lookup( "rodent" );	break;
    	    case 'i': lang = syl_lookup( "insectoid" ); break;
    	    case 'm': lang = syl_lookup( "mammal" );	break;
    	    case 'R': lang = syl_lookup( "reptile" );	break;
    	    case 'D': lang = syl_lookup( "dragon" );	break;
    	    case 's': lang = syl_lookup( "spiritual" ); break;
    	    case 'M': lang = syl_lookup( "magical" );	break;
    	    case 'g': lang = syl_lookup( "goblin" );	break;
    	    case 'G': lang = syl_lookup( "god" );   	break;
    	    case 'h': lang = syl_lookup( "halfling" );	break;
    	    case 'H': lang = syl_lookup( "gith" );  	break;
    	    case 'w': lang = syl_lookup( "drow" );  	break;
    	    case 'k': lang = syl_lookup( "kobold" );	break;
    	    case 'n': lang = syl_lookup( "gnomish" );	break;
    	    case 'P': lang = syl_lookup( "plant" ); 	break;
    	    case 'f': lang = syl_lookup( "fish" );  	break;
    	    case 'C': lang = syl_lookup( "clan" );  	break;
    	    case 'x':
    	    default:  lang = -1;    	    	    	break;
    	    }
    	    j++;
    	    continue;
    	}
    	else if ( lang != -1 )
    	{
    	    int y;

    	    for( y = 0; txt[j] != '^' && txt[j] != '\0'; j++, y++ )
    	    	buf2[y] = txt[j];

    	    buf2[y] = '\0';

    	    str = translate( buf2, lang, ch, NULL );

    	    strcat( buf1, str );

    	    i += y;

    	    lang = -1;
    	}
    	else
    	{
    	    buf1[i] = txt[j];
    	    j++;
    	    i++;
    	    buf1[i] = '\0';
    	}
    }
}

bool speaks_language( CHAR_DATA *vch, int language, CHAR_DATA *ch )
{
    if ( IS_SET( race_table[vch->race].language, lang_table[language].value ) )
    	return TRUE;
    if ( IS_SET( vch->speaks, lang_table[language].value ) )
    	return TRUE;
    if ( IS_SET( lang_table[language].value, LANG_COMMON ) )
    	return TRUE;
    if ( IS_SET( lang_table[language].value, LANG_CLAN ) )
    {
    	if ( IS_NPC( vch ) || IS_NPC( ch ) )
    	    return TRUE;
    	if ( vch->pcdata->clan )
    	    return TRUE;
    }
 
    return FALSE;
}
