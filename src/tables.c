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

extern  int     _filbuf	        args( (FILE *) );

#if !defined( ultrix ) && !defined( apollo )
#include <memory.h>
#endif



#if defined( KEY )
#undef KEY
#endif

#define KEY( literal, field, value )                   \
                if ( !str_cmp( word, literal ) )       \
                {                                      \
                    field  = value;                    \
                    fMatch = TRUE;                     \
                    break;                             \
                }

#define SKEY( string, field )                           \
                if ( !str_cmp( word, string ) )         \
                {                                       \
                    free_string( field );               \
                    field  = fread_string( fp, &stat ); \
                    fMatch = TRUE;                      \
                    break;                              \
                }

#define	GET_TOKEN( fp, token, string )				\
		if ( !( token = fread_word( fp, &stat ) ) )	\
		{						\
		    token = string;				\
		}



/*
 * Globals.
 */

/* The social table.  New socials contributed by Katrina and Binky */
SOC_INDEX_DATA *	soc_index_hash	[ MAX_WORD_HASH ];

CLAN_DATA *             clan_first;
CLAN_DATA *             clan_last;

CLASS_TYPE *		class_first;
CLASS_TYPE *		class_last;

extern FILE *		fpArea;
extern char		strArea		[ ];

/*
 * New code for loading classes from file.
 */
bool fread_class( char *filename )
{
           FILE        *fp;
    static CLASS_TYPE   class_zero;
           CLASS_TYPE  *cclass;
     const char        *word;
           char         buf [ MAX_STRING_LENGTH ];
           bool         fMatch;
           int          stat;
           int          level;
           int          i;

    sprintf( buf, "%s%s", CLASS_DIR, filename );
    if ( !( fp = fopen( buf, "r" ) ) )
    {
        perror( buf );
        return FALSE;
    }

    strcpy( strArea, filename );
    fpArea = fp;

    cclass = (CLASS_TYPE *) alloc_mem ( sizeof( CLASS_TYPE ) );

    *cclass = class_zero;

    cclass->skill_level	 = (int *) alloc_mem( sizeof( int ) * MAX_SKILL );
    cclass->skill_adept	 = (int *) alloc_mem( sizeof( int ) * MAX_SKILL );
    cclass->skill_rating = (int *) alloc_mem( sizeof( int ) * MAX_SKILL );

    /* Initialize MAX_SPELL marker so noone can use it. */
    cclass->skill_level[MAX_SPELL] = MAX_LEVEL+1;

    for ( i = 0; i < MAX_SKILL; i++ )
    {
	cclass->skill_level[i] = L_APP;
	cclass->skill_adept[i] = 0;
    }

    for ( i = 0; i <= MAX_LEVEL; i++ )
    {
	cclass->title[i][0] = str_dup( "" );
	cclass->title[i][1] = str_dup( "" );
    }
	
    for ( i = 0; i < MAX_POSE; i++ )
    {
	cclass->pose[i][0] = str_dup( "" );
	cclass->pose[i][1] = str_dup( "" );
    }

    for ( ; ; )
    {
    	GET_TOKEN( fp, word, "End" );
	fMatch = FALSE;

	switch ( UPPER( word[0] ) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
            KEY( "AtrPrm", cclass->attr_prime, fread_number( fp, &stat ) );
	    break;

	case 'C':
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		fclose( fp );

		if ( !class_first )
		    class_first      = cclass;

		if ( class_last )
		    class_last->next = cclass;

		class_last           = cclass;

		fpArea = NULL;
		return TRUE;
	    }
	    break;

	case 'G':
            KEY( "Guild", cclass->guild, fread_number( fp, &stat ) );
	    break;

	case 'H':
            KEY( "Hpmin", cclass->hp_min, fread_number( fp, &stat ) );
            KEY( "Hpmax", cclass->hp_max, fread_number( fp, &stat ) );
	    break;

	case 'M':
            KEY( "Mana", cclass->fMana, fread_number( fp, &stat ) );
	    break;

	case 'N':
            SKEY( "Nm", cclass->name );
	    break;

	case 'P':
	    if ( !str_cmp( word, "Pose" ) )
	    {
		level = fread_number( fp, &stat );
		i     = fread_number( fp, &stat );

		if ( level < MAX_POSE )
		{
                    free_string( cclass->pose[level][i] );
                    cclass->pose[level][i] = fread_string( fp, &stat );
		}
		else
		    bugf( "Fread_class: invalid pose." );
		fMatch = TRUE;
	    }

	    break;

	case 'S':
            KEY( "SkllAdpt", cclass->max_adept, fread_number( fp, &stat ) );

	    if ( !str_cmp( word, "Skll" ) )
	    {
		int   sn;
		int   value1;
		int   value2;
		int   value3;

		value1 = fread_number( fp, &stat );
		value2 = fread_number( fp, &stat );
		value3 = fread_number( fp, &stat );
		word   = fread_word( fp, &stat );
		sn     = skill_lookup( word );
		if ( sn == -1 )
		{
		    bugf( "Fread_class: unknown skill: %s.", word );
		}
		else
		{
		    cclass->skill_level [sn] = value1;
		    cclass->skill_rating[sn] = value2;
		    cclass->skill_adept [sn] = value3;
		}
		fMatch = TRUE;
	    }

	    break;

	case 'T':
	    KEY( "Thac0", cclass->thac0_00, fread_number( fp, &stat ) );
	    KEY( "Thac47", cclass->thac0_47, fread_number( fp, &stat ) );

	    if ( !str_cmp( word, "Ttle" ) )
	    {
		i  = fread_number( fp, &stat );

		if ( i <= MAX_LEVEL )
		{
                    free_string( cclass->title[i][0] );
                    free_string( cclass->title[i][1] );
                    cclass->title[i][0] = fread_string( fp, &stat );
                    cclass->title[i][1] = fread_string( fp, &stat );
		}
		else
		    bugf( "Fread_class: too many titles." );
		fMatch = TRUE;
	    }

	    break;

	case 'W':
	    SKEY( "WhoNm", cclass->who_name );
	    KEY( "Wpn", cclass->weapon, fread_number( fp, &stat ) );
	    break;
	}

	if ( !fMatch )
	{
            bugf( "load_class_file: no match: %s", word );
	}
    }

    return FALSE;
}

/*
 * Load in all the class files.
 */ 
void load_classes( void )
{
    FILE       *fpList;
    const char *filename;
    char        fname     [ MAX_STRING_LENGTH ];
    char        classlist [ MAX_STRING_LENGTH ];
    int         stat;

    log_string( "Loading classes" );

    sprintf( classlist, "%s%s", CLASS_DIR, CLASS_LIST );
    if ( !( fpList = fopen( classlist, "r" ) ) )
    {
        perror( classlist );
        exit( 1 );
    }

    for ( ; ; )
    {
    	GET_TOKEN( fpList, filename, "$" );
	strcpy( fname, filename );
        if ( fname[0] == '$' )
          break;

        if ( fread_class( fname ) )
	    fputc( '.', stderr );
	else
	    bugf( "Cannot load class file: %s", fname );
    }
    fclose( fpList );

    fputc( '\n', stderr );
    return;
}

void save_class( const CLASS_TYPE *cclass )
{
    FILE                    *fp;
    char                     buf  	[ MAX_STRING_LENGTH ];
    char                     filename	[ MAX_INPUT_LENGTH  ];
    int                      level;
    int                      pose;
    int                      sn;

    sprintf( filename, "%s.cls", cclass->who_name );

    filename[0] = LOWER( filename[0] );

    sprintf( buf, "%s%s", CLASS_DIR, filename );

    fclose( fpReserve );

    if ( !( fp = fopen( buf, "w" ) ) )
    {
        bugf( "Cannot open: %s for writing", filename );
    }
    else
    {
	fprintf( fp, "Nm          %s~\n",	cclass->name		);
	fprintf( fp, "WhoNm       %s~\n",	cclass->who_name	);
	fprintf( fp, "AtrPrm      %d\n",	cclass->attr_prime	);
	fprintf( fp, "Wpn         %d\n",	cclass->weapon		);
	fprintf( fp, "Guild       %d\n",	cclass->guild		);
	fprintf( fp, "Sklladpt    %d\n",	cclass->max_adept	);
	fprintf( fp, "Thac0       %d\n",	cclass->thac0_00	);
	fprintf( fp, "Thac47      %d\n",	cclass->thac0_47	);
	fprintf( fp, "Hpmin       %d\n",	cclass->hp_min		);
	fprintf( fp, "Hpmax       %d\n",	cclass->hp_max		);
	fprintf( fp, "Mana        %d\n",	cclass->fMana		);

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( !skill_table[sn].name )
		break;

	    if ( ( level = cclass->skill_level[sn] ) < LEVEL_IMMORTAL )
		fprintf( fp, "Skll        %3d %3d %3d '%s'\n",
			level,
			cclass->skill_rating[sn],
			cclass->skill_adept [sn],
			skill_table[sn].name );
	}

	for ( level = 0; level <= MAX_LEVEL; level++ )
	    fprintf( fp, "Ttle        %2d %s~ %s~\n",
		level, cclass->title [level] [0], cclass->title [level] [1] );

	for ( pose = 0; pose < MAX_POSE; pose++ )
	{
	    fprintf( fp, "Pose        %2d %1d %s~\n",
		pose, 0, cclass->pose[pose][0] );
	    fprintf( fp, "Pose        %2d %1d %s~\n",
		pose, 1, cclass->pose[pose][1] );
	}
	fprintf( fp, "End\n" );

	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void save_classes( void )
{
    CLASS_TYPE *cclass;

    for ( cclass = class_first; cclass; cclass = cclass->next )
	save_class( cclass );

    return;
}



/*
 * Add a social to the social index table                       - Thoric
 * Hashed and insert sorted.
 */
void add_social( SOC_INDEX_DATA *social )
{
    SOC_INDEX_DATA *tmp;
    SOC_INDEX_DATA *prev;
    int             hash;
    int             x;

    if ( !social )
    {
	bug( "Add_social: NULL social", 0 );
	return;
    }

    if ( !social->name )
    {
	bug( "Add_social: NULL social->name", 0 );
	return;
    }

    if ( !social->char_no_arg )
    {
	bug( "Add_social: NULL social->char_no_arg", 0 );
	return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; social->name[x] != '\0'; x++ )
	social->name[x] = LOWER( social->name[x] );

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
	hash = 0;
    else
	hash = ( social->name[0] - 'a' ) + 1;

    if ( !( prev = tmp = soc_index_hash[hash] ) )
    {
	social->next = soc_index_hash[hash];
	soc_index_hash[hash] = social;
	return;
    }

    for ( ; tmp; tmp = tmp->next )
    {
	if ( !( x = strcmp( social->name, tmp->name ) ) )
	{
	    bug( "Add_social: trying to add duplicate name to bucket %d", hash);
	    free_social( social );
	    return;
	}
	else
	if ( x < 0 )
	{
	    if ( tmp == soc_index_hash[hash] )
	    {
		social->next = soc_index_hash[hash];
		soc_index_hash[hash] = social;
		return;
	    }
	    prev->next = social;
	    social->next = tmp;
	    return;
	}
	prev = tmp;
    }

    /* add to end */
    prev->next = social;
    social->next = NULL;
    return;
}

/*
 * Save the social_table_tables to disk. -Toric
 */
void save_socials( void )
{
    FILE           *fpout;
    SOC_INDEX_DATA *social;
    int             x;
    char            strsave [ MAX_INPUT_LENGTH ];

    fclose( fpReserve );

    sprintf( strsave, "%s%s", SYSTEM_DIR, SOCIAL_FILE );

    if ( !( fpout = fopen( strsave, "w" ) ) )
    {
	bug( "Cannot open SOCIALS.TXT for writting", 0 );
	perror( SOCIAL_FILE );
	return;
    }

    for ( x = 0; x < 27; x++ )
    {
	for ( social = soc_index_hash[x]; social; social = social->next )
	{
	    if ( !social->name || social->name[0] == '\0' )
	    {
		bug( "Save_socials: blank social in hash bucket %d", x );
		continue;
	    }
	    fprintf( fpout, "#SOCIAL\n" );
	    fprintf( fpout, "Name        %s~\n",	social->name );
	    if ( social->char_no_arg )
		fprintf( fpout, "CharNoArg   %s~\n",	social->char_no_arg );
	    else
	        bug( "Save_socials: NULL char_no_arg in hash bucket %d", x );
	    if ( social->others_no_arg )
		fprintf( fpout, "OthersNoArg %s~\n",	social->others_no_arg );
	    if ( social->char_found )
		fprintf( fpout, "CharFound   %s~\n",	social->char_found );
	    if ( social->others_found )
		fprintf( fpout, "OthersFound %s~\n",	social->others_found );
	    if ( social->vict_found )
		fprintf( fpout, "VictFound   %s~\n",	social->vict_found );
	    if ( social->char_auto )
		fprintf( fpout, "CharAuto    %s~\n",	social->char_auto );
	    if ( social->others_auto )
		fprintf( fpout, "OthersAuto  %s~\n",	social->others_auto );
	    fprintf( fpout, "End\n\n" );
	}
    }

    fprintf( fpout, "#END\n" );
    fclose( fpout );

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Clear a new social.
 */
void clear_social( SOC_INDEX_DATA *soc )
{
    static SOC_INDEX_DATA soc_zero;

    *soc			= soc_zero;
    soc->name			= NULL;
    soc->char_no_arg		= NULL;
    soc->others_no_arg		= NULL;
    soc->char_found		= NULL;
    soc->others_found		= NULL;
    soc->vict_found		= NULL;
    soc->char_auto		= NULL;
    soc->others_auto		= NULL;
    return;
}

/*
 * Take a social data from the free list and clean it out.
 */
SOC_INDEX_DATA *new_social( void )
{
    SOC_INDEX_DATA *soc;

    soc		= (SOC_INDEX_DATA *) alloc_perm( sizeof( SOC_INDEX_DATA ) );

    clear_social( soc );

    return soc;
}

/*
 * Remove a social from it's hash index                         - Thoric
 */
void unlink_social( SOC_INDEX_DATA *social )
{
    SOC_INDEX_DATA *tmp;
    SOC_INDEX_DATA *tmp_next;
    int             hash;

    if ( !social )
    {
        bug( "Unlink_social: NULL social", 0 );
        return;
    }

    if ( !islower( social->name[0] ) )
        hash = 0;
    else
        hash = ( social->name[0] - 'a' ) + 1;

    if ( social == ( tmp = soc_index_hash[hash] ) )
    {
        soc_index_hash[hash] = tmp->next;
        return;
    }

    for ( ; tmp; tmp = tmp_next )
    {
        tmp_next = tmp->next;
        if ( social == tmp_next )
        {
            tmp->next = tmp_next->next;
            return;
        }
    }

    return;
}

/*
 * Free a social structure
 */
void free_social( SOC_INDEX_DATA *social )
{
    free_string( social->name          );
    free_string( social->char_no_arg   );
    free_string( social->others_no_arg );
    free_string( social->char_found    );
    free_string( social->others_found  );
    free_string( social->vict_found    );
    free_string( social->char_auto     );
    free_string( social->others_auto   );

    free_mem( social, sizeof( SOC_INDEX_DATA ) );
    return;
}

void fread_social( FILE *fp )
{
    const char     *word;
    SOC_INDEX_DATA *social;
    bool            fMatch;
    int             stat;

    social = new_social( );

    for ( ; ; )
    {
    	GET_TOKEN( fp, word, "End" );
	fMatch = FALSE;

	switch ( UPPER( word[0] ) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'C':
	    SKEY( "CharNoArg", social->char_no_arg );
	    SKEY( "CharFound", social->char_found  );
	    SKEY( "CharAuto",  social->char_auto   );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !social->name )
		{
		    bugf( "Fread_social: Name not found" );
		    free_social( social );
		    return;
		}
		if ( !social->char_no_arg )
		{
		    bugf( "Fread_social: CharNoArg not found" );
		    free_social( social );
		    return;
		}
		add_social( social );
		return;
	    }
	    break;

	case 'N':
	    SKEY( "Name", social->name );
	    break;

	case 'O':
	    SKEY( "OthersNoArg", social->others_no_arg );
	    SKEY( "OthersFound", social->others_found  );
	    SKEY( "OthersAuto",	 social->others_auto   );
	    break;

	case 'V':
	    SKEY( "VictFound", social->vict_found );
	    break;
	}
	
	if ( !fMatch )
	{
            bugf( "Fread_social: no match: %s. Skipping to next line.", word );
	    fread_to_eol( fp );
	}
    }

    return;
}

void load_socials( void )
{
    FILE *fp;
    int   stat;
    char  strsave [ MAX_INPUT_LENGTH ];

    fclose( fpReserve );

    sprintf( strsave, "%s%s", SYSTEM_DIR, SOCIAL_FILE );

    if ( !( fp = fopen( strsave, "r" ) ) )
    {
	bug( "Cannot open SOCIALS.TXT", 0 );
	exit( 0 );
    }

    for ( ; ; )
    {
	int   letter;
	char *word;

	letter = fread_letter( fp );

	if ( letter == '*' )
	{
	    fread_to_eol( fp );
	    continue;
	}

	if ( letter != '#' )
	{
	    bug( "Load_socials: # not found.", 0 );
	    break;
	}

	word = fread_word( fp, &stat );
	if ( !str_cmp( word, "SOCIAL" ) )
	{
	    fread_social( fp );
	    continue;
	}
	else
	if ( !str_cmp( word, "END" ) )
	    break;
	else
	{
	    bug( "Load_socials: bad section.", 0 );
	    continue;
	}
    }
    
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Get pointer to clan structure from clan name. -Toric
 */
CLAN_DATA *get_clan( const char *name )
{
    CLAN_DATA *clan;
    
    for ( clan = clan_first; clan; clan = clan->next )
       if ( !str_cmp( name, clan->name ) )
         return clan;
    return NULL;
}

/*
 * New code for loading clans from file.
 */
bool fread_clan( CLAN_DATA *clan, FILE *fp )
{
    const char *word;
    bool        fMatch;
    int         stat;

    for ( ; ; )
    {
    	GET_TOKEN( fp, word, "End" );
	fMatch = FALSE;

	switch ( UPPER( word[0] ) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'C':
            SKEY( "Chieftain",   clan->chieftain );
             if ( !str_cmp( word, "Class" ) )
             {
		clan->cclass = class_lookup( temp_fread_string( fp, &stat ) );
             	fMatch = TRUE;
             	break;
             }
            KEY( "ClanHeros",    clan->clanheros, fread_number( fp, &stat ) );
            KEY( "ClanType",     clan->clan_type, fread_number( fp, &stat ) );
            KEY( "ClanObjOne",   clan->clanobj1,  fread_number( fp, &stat ) );
            KEY( "ClanObjTwo",   clan->clanobj2,  fread_number( fp, &stat ) );
            KEY( "ClanObjThree", clan->clanobj3,  fread_number( fp, &stat ) );
	    break;

	case 'D':
            SKEY( "Desc",    clan->description );
            KEY( "Donation", clan->donation,  fread_number( fp, &stat ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
		return TRUE;
	    break;

	case 'I':
            KEY( "IllegalPK",   clan->illegal_pk,  fread_number( fp, &stat ) );
	    break;

	case 'M':
            KEY( "Members",     clan->members, fread_number( fp, &stat ) );
            KEY( "MKills",      clan->mkills,  fread_number( fp, &stat ) );
            KEY( "MDeaths",     clan->mdeaths, fread_number( fp, &stat ) );
            SKEY( "Motto",      clan->motto );
	    break;

	case 'N':
            SKEY( "Name",      clan->name    );
	    break;

	case 'O':
            SKEY( "Overlord", clan->overlord );
	    break;

	case 'P':
            KEY( "PKills",  clan->pkills,  fread_number( fp, &stat ) );
            KEY( "PDeaths", clan->pdeaths, fread_number( fp, &stat ) );
	    break;

	case 'R':
            KEY( "Recall",  clan->recall,  fread_number( fp, &stat ) );
	    break;

	case 'S':
            KEY( "Score",     clan->score,     fread_number( fp, &stat ) );
            KEY( "Subchiefs", clan->subchiefs, fread_number( fp, &stat ) );
	    break;

	case 'W':
            SKEY( "WhoName",   clan->who_name    );
	    break;

	}

	if ( !fMatch )
	{
            bugf( "Load_clan_file: no match: %s", word );
	}
    }

    return FALSE;
}

bool load_clan_file( char *filename )
{
    CLAN_DATA *clan;
    FILE      *fp;
    int        stat;
    char       buf [ MAX_STRING_LENGTH ];

    sprintf( buf, "%s%s", CLAN_DIR, filename );
    if ( !( fp = fopen( buf, "r" ) ) )
    {
        perror( buf );
        return FALSE;
    }

    clan = (CLAN_DATA *) alloc_mem ( sizeof( CLAN_DATA ) );
    clan->filename = str_dup( filename );

    for ( ; ; )
    {
	char *word;
	int   letter;

	letter = fread_letter( fp );
	if ( letter == '*' )
	{
	    fread_to_eol( fp );
	    continue;
	}

	if ( letter != '#' )
	{
	    bug( "Load_clan_file: # not found.", 0 );
	    free_mem( clan, sizeof( CLAN_DATA ) );
	    break;
	}

	word = fread_word( fp, &stat );

	if ( !str_cmp( word, "CLAN" ) )
	{
	    fread_clan( clan, fp );

	    if ( !clan_first )
		clan_first	= clan;
	    else
		clan_last->next	= clan;
	    clan->next		= NULL;
	    clan_last		= clan;

	    break;
	}
	else if ( !str_cmp( word, "END"  ) )				break;
	else
	{
	    bugf( "Load_clan_file: bad section: %s.", word );
	    free_mem( clan, sizeof( CLAN_DATA ) );
	    break;
	}
    }
    fclose( fp );

    return TRUE;
}

/*
 * Load in all the clan files.
 */ 
void load_clans( void )
{
    FILE       *fpList;
    const char *filename;
    char        fname		[ MAX_STRING_LENGTH ];
    char        clanslist	[ MAX_STRING_LENGTH ];
    int         stat;
    
    clan_first  = NULL;
    clan_last   = NULL;

    log_string( "Loading clans" );

    sprintf( clanslist, "%s%s", CLAN_DIR, CLANS_LIST );
    if ( !( fpList = fopen( clanslist, "r" ) ) )
    {
        perror( clanslist );
        exit( 1 );
    }

    for ( ; ; )
    {
    	GET_TOKEN( fpList, filename, "$" );
	strcpy( fname, filename );
        if ( fname[0] == '$' )
	    break;

        if ( load_clan_file( fname ) )
	    fputc( '.', stderr );
	else
	    bugf( "Cannot load clan file: %s", fname );

    }
    fclose( fpList );

    fputc( '\n', stderr );
    return;
}

void save_clan_list( void )
{
    FILE      *fp;
    CLAN_DATA *clan;
    char       clanslist	[ MAX_STRING_LENGTH ];

    sprintf( clanslist, "%s%s", CLAN_DIR, CLANS_LIST );

    fclose( fpReserve );

    if ( !( fp = fopen( clanslist, "w" ) ) )
    {
        bug( "Save_clan_list: fopen", 0 );
        perror( clanslist );
	return;
    }

    for ( clan = clan_first; clan; clan = clan->next )
	fprintf( fp, "%s\n", clan->filename );

    fprintf( fp, "$\n" );
    fclose( fp );

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void clan_update( void )
{
    CLAN_DATA *clan;

    for ( clan = clan_first; clan; clan = clan->next )
	save_clan( clan );
    return;
}

/*
 * New code for writing a clan to a file.
 */
void save_clan( CLAN_DATA *clan )
{
    FILE                    *fp;
    char                     buf	[ MAX_STRING_LENGTH ];

    if ( !clan->filename )
	return;
    
    sprintf( buf, "%s%s", CLAN_DIR, clan->filename );

    fclose( fpReserve );

    if ( !( fp = fopen( buf, "w" ) ) )
    {
        bugf( "Cannot open: %s for writing", clan->filename );
    }
    else
    {
	fprintf( fp, "#CLAN\n"						    );
	fprintf( fp, "WhoName       %s~\n",        clan->who_name	    );
	fprintf( fp, "Name          %s~\n",        clan->name		    );
	fprintf( fp, "Motto         %s~\n",        clan->motto		    );
	fprintf( fp, "Desc          %s~\n", fix_string( clan->description ) );
	fprintf( fp, "Overlord      %s~\n",        clan->overlord	    );
	fprintf( fp, "Chieftain     %s~\n",        clan->chieftain	    );
	fprintf( fp, "PKills        %d\n",         clan->pkills		    );
	fprintf( fp, "PDeaths       %d\n",         clan->pdeaths	    );
	fprintf( fp, "MKills        %d\n",         clan->mkills		    );
	fprintf( fp, "MDeaths       %d\n",         clan->mdeaths	    );
	fprintf( fp, "IllegalPK     %d\n",         clan->illegal_pk	    );
	fprintf( fp, "Score         %d\n",         clan->score		    );
	fprintf( fp, "ClanType      %d\n",         clan->clan_type	    );
	fprintf( fp, "Clanheros     %d\n",         clan->clanheros	    );
	fprintf( fp, "Subchiefs     %d\n",         clan->subchiefs	    );
	fprintf( fp, "Members       %d\n",         clan->members	    );
	fprintf( fp, "ClanObjOne    %d\n",         clan->clanobj1	    );
	fprintf( fp, "ClanObjTwo    %d\n",         clan->clanobj2	    );
	fprintf( fp, "ClanObjThree  %d\n",         clan->clanobj3	    );
	fprintf( fp, "Recall        %d\n",         clan->recall 	    );
	fprintf( fp, "Donation      %d\n",         clan->donation	    );

	if ( clan->cclass )
	    fprintf( fp, "Class         %s~\n",    clan->cclass->name  	    );

	fprintf( fp, "End\n"						    );
	fprintf( fp, "#END\n"						    );

	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );

    return;
}



void do_makeclan( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CLAN_DATA *clan;
    char       filename [ MAX_STRING_LENGTH ];
    char       who_name [ MAX_STRING_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "makeclan" ) )
        return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: makeclan <clan name>\n\r", ch );
        return;
    }

    one_argument( argument, who_name );
    who_name[14] = '\0';

    sprintf( filename, "%s.cln", who_name );

    clan = (CLAN_DATA *) alloc_mem ( sizeof( CLAN_DATA ) );
    if ( !clan_first )
	clan_first		= clan;
    else
	clan_last->next		= clan;
	clan->next		= NULL;
	clan_last		= clan;

    clan->filename		= str_dup( filename );
    clan->who_name		= str_dup( who_name );
    clan->name			= str_dup( argument );
    clan->motto			= str_dup( "" );
    clan->description		= str_dup( "" );
    clan->overlord		= str_dup( "" );
    clan->chieftain		= str_dup( "" );
    clan->subchiefs		= 0;
    clan->clanheros		= 0;
    clan->members		= 0;
    clan->recall		= 3001;
    clan->donation		= 0;
    clan->cclass		= 0;
    clan->mkills		= 0;
    clan->mdeaths		= 0;
    clan->pkills		= 0;
    clan->pdeaths		= 0;
    clan->illegal_pk		= 0;
    clan->score			= 0;
    clan->clan_type		= 0;
    clan->clanobj1		= 0;
    clan->clanobj2		= 0;
    clan->clanobj3		= 0;

    return;
}




/*
 * Save a single board.
 */
void save_board( BOARD_DATA *board )
{
    FILE      *fp;
    char       strsave	[ MAX_INPUT_LENGTH ];
    NOTE_DATA *note;

    fclose( fpReserve );

    sprintf( strsave, "%s%s", NOTE_DIR, board->short_name );

    if ( !( fp = fopen( strsave, "w" ) ) )
    {
	perror( board->short_name );
    }
    else
    {
	for ( note = board->note_first; note; note = note->next )
	    append_note( fp, note );
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Snarf a board.
 */
void load_board( BOARD_DATA *board )
{
    FILE      *fp;
    FILE      *fpArch;
    NOTE_DATA *pnotelast;
    char       strsave	[ MAX_INPUT_LENGTH ];

    sprintf( strsave, "%s%s", NOTE_DIR, board->short_name );

    if ( !( fp = fopen( strsave, "r" ) ) )
	return;	    
    	    
    pnotelast = NULL;
    for ( ; ; )
    {
	NOTE_DATA *pnote;
	int        letter;
	int        stat;

	do
	{
	    letter = getc( fp );
	    if ( feof( fp ) )
	    {
		fclose( fp );
		return;
	    }
	}
	while ( isspace( letter ) );
	ungetc( letter, fp );

	pnote		  = (NOTE_DATA *) alloc_mem( sizeof( *pnote ) );

	if ( str_cmp( fread_word( fp, &stat ), "sender" ) )
	    break;
	pnote->sender     = fread_string( fp, &stat );

	if ( str_cmp( fread_word( fp, &stat ), "date" ) )
	    break;
	pnote->date       = fread_string( fp, &stat );

	if ( str_cmp( fread_word( fp, &stat ), "stamp" ) )
	    break;
	pnote->date_stamp = fread_number( fp, &stat );

	if ( str_cmp( fread_word( fp, &stat ), "expire" ) )
	    break;
	pnote->expire     = fread_number( fp, &stat );

	if ( str_cmp( fread_word( fp, &stat ), "to" ) )
	    break;
	pnote->to_list    = fread_string( fp, &stat );

	if ( str_cmp( fread_word( fp, &stat ), "subject" ) )
	    break;
	pnote->subject    = fread_string( fp, &stat );

	if ( str_cmp( fread_word( fp, &stat ), "text" ) )
	    break;
	pnote->text       = fread_string( fp, &stat );

	pnote->next = NULL;


        /*
	 * Should this note be archived right now?
	 */
        if ( pnote->expire < current_time )
        {
	    sprintf( strsave, "%s%s.old", NOTE_DIR, board->short_name );

	    if ( !( fpArch = fopen( strsave, "a" ) ) )
		bug( "Load_board: couldn't open arch boards for writing.", 0 );
	    else
	    {
		append_note( fpArch, pnote );
		fclose( fpArch );
	    }

	    free_note( pnote );
	    board->changed = TRUE;
	    continue;
        }

        if ( !board->note_first )
	    board->note_first	= pnote;
	else
	    pnotelast->next	= pnote;

        pnotelast		= pnote;
    }

    strcpy( strArea, board->short_name );
    fpArea = fp;

    bug( "Load_board: bad key word.", 0 );
    return;
}



/*
 * Initialize structures.  Load all boards.
 */
void load_notes( void )
{
    int i;

    for ( i = 0; i < MAX_BOARD; i++ )
	load_board( &board_table[i] );
    return;
}



/*
 * Save changed boards.
 */
void notes_update( void )
{
    int i;

    for ( i = 0; i < MAX_BOARD; i++ )
	if ( board_table[i].changed )	save_board( &board_table[i] );
    return;
}
