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
#include <string.h>
#include <time.h>
#include "merc.h"



const char *act_xbv( const FLAG_TYPE *table, const XBV vector )
{
    static char buf [ MAX_STRING_LENGTH ];
    int         pos;

    buf[0] = '\0';

    for ( pos = 0; *table[pos].name; pos++ )
    {
	if ( xIS_SET( vector, table[pos].bit ) )
	{
	    strcat( buf, " " );
	    strcat( buf, table[pos].name );
	}
    }
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}


const char *write_xbv( const FLAG_TYPE *table, const XBV vector )
{
    static char buf [ MAX_STRING_LENGTH ];
    int         pos;

    buf[0] = '\0';

    for ( pos = 0; *table[pos].name; pos++ )
    {
	if ( !table[pos].settable )
	    continue;

	if ( xIS_SET( vector, table[pos].bit ) )
	{
	    strcat( buf, " " );
	    strcat( buf, table[pos].name );
	}
    }
    return ( buf[0] != '\0' ) ? buf+1 : "";
}



struct flag_stat_type
{
    const FLAG_TYPE	*structure;
    bool		 stat;
};

const	struct	flag_stat_type	flag_stat_table	[ ]	=
{
/*      structure		stat	*/
    {	connected_flags, 	TRUE		},
    {	area_flags, 		FALSE		},
    {	sex_flags, 		TRUE		},
    {	size_flags, 		TRUE		},
    {	exit_flags, 		FALSE		},
    {	door_resets, 		TRUE		},
    {	room_flags, 		FALSE		},
    {	sector_flags, 		TRUE		},
    {	type_flags, 		TRUE		},
    {	extra_flags, 		FALSE		},
    {	wear_flags, 		FALSE		},
    {	act_flags, 		TRUE  	 	},
    {	plr_flags, 		TRUE   		},
    {	affect_flags, 		TRUE   		},
    {	apply_flags, 		TRUE  	 	},
    {	wear_loc_flags, 	TRUE   		},
    {	wear_loc_strings, 	TRUE   		},
    {	weapon_flags, 		TRUE   		},
    {	container_flags, 	FALSE		},
    {	liquid_flags, 		TRUE		},
    {	mp_flags, 		TRUE		},
    {	portal_door_flags, 	FALSE		},
    {	portal_flags,	 	FALSE		},
    {	mana_flags,	 	FALSE		},
    {	rank_flags, 		TRUE		},
    {	clan_flags, 		TRUE		},
    {	ris_flags, 		FALSE		},
    {	position_flags, 	TRUE		},
    {	range_type_flags, 	TRUE   		},
    {	0, 			0		}
};


bool is_stat( const FLAG_TYPE *flag_table )
{
    int flag;

    for ( flag = 0; flag_stat_table[flag].structure; flag++ )
    {
	if ( flag_stat_table[flag].structure == flag_table
	    && flag_stat_table[flag].stat )
	    return TRUE;
    }
    return FALSE;
}


int flag_lookup( const char *name, const FLAG_TYPE *flag_table )
{
    int flag;

    for ( flag = 0; *flag_table[flag].name; flag++ )
    {
	if ( !str_cmp( name, flag_table[flag].name )
	    && flag_table[flag].settable )
	    return flag_table[flag].bit;
    }

    return NO_FLAG;
}


int flag_slookup( const char *name, const FLAG_TYPE *flag_table )
{
    int flag;

    for ( flag = 0; *flag_table[flag].name; flag++ )
    {
	if ( !str_cmp( name, flag_table[flag].name ) )
	    return flag_table[flag].bit;
    }

    return NO_FLAG;
}


int flag_value( const FLAG_TYPE *flag_table, char *argument )
{
    char word [ MAX_INPUT_LENGTH ];
    int  bit;
    int  marked	= 0;
    bool found	= FALSE;

    if ( is_stat( flag_table ) )
    {
	one_argument( argument, word );

	bit = flag_slookup( word, flag_table );
	return bit;
    }

    for ( ; ; )
    {
	argument = one_argument( argument, word );

	if ( word[0] == '\0' )
	    break;

	if ( ( bit = flag_lookup( word, flag_table ) ) != NO_FLAG )
	{
	    SET_BIT( marked, bit );
	    found = TRUE;
	}
    }

    if ( found )
	return marked;
    else
	return NO_FLAG;
}


int fread_flag( FILE *fp, const FLAG_TYPE *flag_table )
{
    char   word [ MAX_INPUT_LENGTH  ];
    char  *buf;
    char  *argument;
    int    bit;
    int    marked	= 0;
    int    stat;
/*    bool   found	= FALSE;*/

    buf = temp_fread_string( fp, &stat );
    argument = &buf[0];

    if ( is_stat( flag_table ) )
    {
	one_argument( argument, word );

	bit = flag_slookup( word, flag_table );
	return bit;
    }

    for ( ; ; )
    {
	argument = one_argument( argument, word );

	if ( word[0] == '\0' )
	    break;

	if ( ( bit = flag_slookup( word, flag_table ) ) != NO_FLAG )
	{
	    SET_BIT( marked, bit );
/*	    found = TRUE;*/
	}
    }

    return marked;
}


/*
 * Coded by Zen.
 */
XBV fread_xbv( FILE *fp, const FLAG_TYPE *table )
{
    static char  arg      [MAX_INPUT_LENGTH];
           char *buf;
	   int   status;
           int   bit;
    static XBV   vector;

    xCLEAR_BITS( vector );
    buf = temp_fread_string( fp, &status );

    for ( ; ; )
    {
	buf = one_argument( buf, arg );

	if ( *arg == '\0' )
	    break;

	if ( ( bit = flag_slookup( arg, table ) ) != NO_FLAG )
	    xSET_BIT( vector, bit );
	else
	   bugf ("Fread_xbv: Unknown bit '%s'", arg);
    }

    return vector;
}


const char *flag_string( const FLAG_TYPE *flag_table, int bits )
{
    static char buf [ MAX_STRING_LENGTH ];
    int         flag;

    buf[0] = '\0';

    for ( flag = 0; *flag_table[flag].name; flag++ )
    {
	if ( !is_stat( flag_table ) && IS_SET( bits, flag_table[flag].bit ) )
	{
	    strcat( buf, " " );
	    strcat( buf, flag_table[flag].name );
	}
	else if ( flag_table[flag].bit == bits )
	{
	    strcat( buf, " " );
	    strcat( buf, flag_table[flag].name );
	    break;
	}
    }
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}


const char *flag_strings( const FLAG_TYPE *flag_table, int bits )
{
    static char buf [ MAX_STRING_LENGTH ];
    int         flag;

    buf[0] = '\0';

    for ( flag = 0; *flag_table[flag].name; flag++ )
    {
	if ( !flag_table[flag].settable )
	    continue;

	if ( !is_stat( flag_table ) && IS_SET( bits, flag_table[flag].bit ) )
	{
	    strcat( buf, " " );
	    strcat( buf, flag_table[flag].name );
	}
	else if ( flag_table[flag].bit == bits )
	{
	    strcat( buf, " " );
	    strcat( buf, flag_table[flag].name );
	    break;
	}
    }
    return ( buf[0] != '\0' ) ? buf+1 : "";
}



const	FLAG_TYPE		connected_flags	[ ]	=
{
    {	"PLAYING",		CON_PLAYING,			FALSE	},
    {	"GET_NAME",		CON_GET_NAME,			FALSE	},
    {	"GET_OLD_PASSWORD",	CON_GET_OLD_PASSWORD,		FALSE	},
    {	"CONFIRM_NEW_NAME",	CON_CONFIRM_NEW_NAME,		FALSE	},
    {	"GET_NEW_PASSWORD",	CON_GET_NEW_PASSWORD,		FALSE	},
    {	"CONFIRM_NEW_PASSWORD", CON_CONFIRM_NEW_PASSWORD,	FALSE	},
    {	"GET_COLOUR",		CON_GET_COLOUR,			FALSE	},
    {	"DISPLAY_RACE", 	CON_DISPLAY_RACE,		FALSE	},
    {	"GET_NEW_RACE", 	CON_GET_NEW_RACE,		FALSE	},
    {	"CONFIRM_NEW_RACE",	CON_CONFIRM_NEW_RACE,		FALSE	},
    {	"GET_NEW_SEX",		CON_GET_NEW_SEX,		FALSE	},
    {	"DISPLAY_1ST_CLASS",	CON_DISPLAY_1ST_CLASS,		FALSE	},
    {	"GET_1ST_CLASS",	CON_GET_1ST_CLASS,		FALSE	},
    {	"CONFIRM_1ST_CLASS",	CON_CONFIRM_1ST_CLASS,		FALSE	},
    {	"DEFAULT_CHOICE",	CON_DEFAULT_CHOICE,		FALSE	},
    {	"DISPLAY_2ND_CLASS",	CON_DISPLAY_2ND_CLASS,		FALSE	},
    {	"GET_2ND_CLASS",	CON_GET_2ND_CLASS,		FALSE	},
    {	"CONFIRM_2ND_CLASS",	CON_CONFIRM_2ND_CLASS,		FALSE	},
    {	"SHOW_MOTD",		CON_SHOW_MOTD,			FALSE	},
    {	"READ_MOTD",		CON_READ_MOTD,			FALSE	},
    {	"PASSWD_GET_OLD",	CON_PASSWD_GET_OLD,		FALSE	},
    {	"PASSWD_GET_NEW",	CON_PASSWD_GET_NEW,		FALSE	},
    {	"PASSWD_CONFIRM_NEW",	CON_PASSWD_CONFIRM_NEW, 	FALSE	},
    {	"RETIRE_GET_PASSWORD",	CON_RETIRE_GET_PASSWORD,	FALSE	},
    {	"RETIRE_CONFIRM",	CON_RETIRE_CONFIRM,		FALSE	},
    {	"AEDITOR",		CON_AEDITOR,			FALSE	},
    {	"REDITOR",		CON_REDITOR,			FALSE	},
    {	"MEDITOR",		CON_MEDITOR,			FALSE	},
    {	"OEDITOR",		CON_OEDITOR,			FALSE	},
    {	"MPEDITOR",		CON_MPEDITOR,			FALSE	},
    {	"", 			0,				0	}
};


const	FLAG_TYPE		area_flags	[ ]	=
{
    {	"none", 		AREA_NONE, 	FALSE	},
    {	"changed", 		AREA_CHANGED,	FALSE	},
    {	"added", 		AREA_ADDED, 	FALSE	},
    {	"loading", 		AREA_LOADING, 	FALSE	},
    {	"verbose", 		AREA_VERBOSE, 	FALSE	},
    {	"proto", 		AREA_PROTO,	TRUE	},
    {	"", 			0, 		0	}
};


const	FLAG_TYPE		sex_flags	[ ]	=
{
    {	"male", 		SEX_MALE, 	TRUE	},
    {	"female", 		SEX_FEMALE, 	TRUE	},
    {	"neutral", 		SEX_NEUTRAL, 	TRUE	},
    {	"", 			0, 		0	}
};


const	FLAG_TYPE		size_flags	[ ]	=
{
    {	"any",			SIZE_ANY,	TRUE	},
    {	"minute", 		SIZE_MINUTE,	TRUE	},
    {	"small", 		SIZE_SMALL,	TRUE	},
    {	"petite", 		SIZE_PETITE,	TRUE	},
    {	"average", 		SIZE_AVERAGE,	TRUE	},
    {	"medium", 		SIZE_MEDIUM,	TRUE	},
    {	"large", 		SIZE_LARGE,	TRUE	},
    {	"huge", 		SIZE_HUGE,	TRUE	},
    {	"titanic", 		SIZE_TITANIC,	TRUE	},
    {	"gargantuan", 		SIZE_GARGANTUAN,TRUE	},
    {	"", 			0, 		0	}
};


const	FLAG_TYPE		exit_flags	[ ]	=
{
    {	"door", 		EX_ISDOOR, 	TRUE	},
    {	"closed", 		EX_CLOSED, 	TRUE	},
    {	"locked", 		EX_LOCKED, 	TRUE	},
    {	"bashed", 		EX_BASHED, 	FALSE	},
    {	"bashproof", 		EX_BASHPROOF, 	TRUE	},
    {	"pickproof", 		EX_PICKPROOF, 	TRUE	},
    {	"passproof", 		EX_PASSPROOF, 	TRUE	},
    {	"eat_key", 		EX_EAT_KEY, 	TRUE	},
    {	"", 			0, 		0	}
};


const	FLAG_TYPE		door_resets	[ ]	=
{
    {	"open and unlocked", 	0, 		TRUE	},
    {	"closed and unlocked", 	1, 		TRUE	},
    {	"closed and locked", 	2, 		TRUE	},
    {	"", 			0, 		0	}
};


const	FLAG_TYPE		room_flags	[ ]	=
{
    {	"dark", 		ROOM_DARK, 		TRUE	},
    {	"no_mob", 		ROOM_NO_MOB, 		TRUE	},
    {	"indoors", 		ROOM_INDOORS, 		TRUE	},
    {	"private", 		ROOM_PRIVATE, 		TRUE	},
    {	"safe", 		ROOM_SAFE, 		TRUE	},
    {	"solitary", 		ROOM_SOLITARY, 		TRUE	},
    {	"pet_shop", 		ROOM_PET_SHOP, 		TRUE	},
    {	"no_recall", 		ROOM_NO_RECALL, 	TRUE	},
    {	"cone_of_silence", 	ROOM_CONE_OF_SILENCE, 	TRUE	},
    {	"arena",	 	ROOM_ARENA, 		TRUE	},
    {	"marked",	 	ROOM_MARKED, 		FALSE	},
    {	"", 			0, 			0	}
};


const	FLAG_TYPE		sector_flags	[ ]	=
{
    {	"inside", 		SECT_INSIDE, 		TRUE	},
    {	"city", 		SECT_CITY, 		TRUE	},
    {	"field", 		SECT_FIELD, 		TRUE	},
    {	"forest", 		SECT_FOREST, 		TRUE	},
    {	"hills", 		SECT_HILLS, 		TRUE	},
    {	"mountain", 		SECT_MOUNTAIN, 		TRUE	},
    {	"water_swim", 		SECT_WATER_SWIM, 	TRUE	},
    {	"water_noswim", 	SECT_WATER_NOSWIM, 	TRUE	},
    {	"underwater", 		SECT_UNDERWATER, 	TRUE	},
    {	"air", 			SECT_AIR, 		TRUE	},
    {	"desert", 		SECT_DESERT, 		TRUE	},
    {	"dunno", 		SECT_DUNNO, 		TRUE	},
    {	"iceland", 		SECT_ICELAND, 		TRUE	},
    {	"", 			0, 			0	}
};


const	FLAG_TYPE		type_flags	[ ]	=
{
    {	"light", 		ITEM_LIGHT, 		TRUE	},
    {	"scroll", 		ITEM_SCROLL, 		TRUE	},
    {	"wand", 		ITEM_WAND, 		TRUE	},
    {	"staff", 		ITEM_STAFF, 		TRUE	},
    {	"weapon", 		ITEM_WEAPON, 		TRUE	},
    {	"treasure", 		ITEM_TREASURE, 		TRUE	},
    {	"armor", 		ITEM_ARMOR, 		TRUE	},
    {	"potion", 		ITEM_POTION, 		TRUE	},
    {	"furniture", 		ITEM_FURNITURE, 	TRUE	},
    {	"trash", 		ITEM_TRASH, 		TRUE	},
    {	"container", 		ITEM_CONTAINER, 	TRUE	},
    {	"drink_con",		ITEM_DRINK_CON, 	TRUE	},
    {	"key", 			ITEM_KEY, 		TRUE	},
    {	"food", 		ITEM_FOOD, 		TRUE	},
    {	"money", 		ITEM_MONEY, 		TRUE	},
    {	"boat", 		ITEM_BOAT, 		TRUE	},
    {	"corpse_npc", 		ITEM_CORPSE_NPC, 	TRUE	},
    {	"corpse_pc", 		ITEM_CORPSE_PC, 	TRUE	},
    {	"fountain", 		ITEM_FOUNTAIN, 		TRUE	},
    {	"pill", 		ITEM_PILL, 		TRUE	},
    {	"portal", 		ITEM_PORTAL, 		TRUE	},
    {	"warp_stone", 		ITEM_WARP_STONE, 	TRUE	},
    {	"clothing", 		ITEM_CLOTHING, 		TRUE	},
    {	"ranged_weapon", 	ITEM_RANGED_WEAPON, 	TRUE	},
    {	"magazine", 		ITEM_MAGAZINE, 		TRUE	},
    {	"gem",	 		ITEM_GEM, 		TRUE	},
    {	"vehicle", 		ITEM_VEHICLE, 		TRUE	},
    {	"", 			0, 			0	}
};


const	FLAG_TYPE		extra_flags	[ ]	=
{
    {	"glow", 		ITEM_GLOW, 		TRUE	},
    {	"hum", 			ITEM_HUM, 		TRUE	},
    {	"dark", 		ITEM_DARK, 		TRUE	},
    {	"lock", 		ITEM_LOCK, 		TRUE	},
    {	"evil", 		ITEM_EVIL, 		TRUE	},
    {	"invis", 		ITEM_INVIS, 		TRUE	},
    {	"magic", 		ITEM_MAGIC, 		TRUE	},
    {	"nodrop", 		ITEM_NODROP, 		TRUE	},
    {	"bless", 		ITEM_BLESS, 		TRUE	},
    {	"anti-good", 		ITEM_ANTI_GOOD, 	TRUE	},
    {	"anti-evil", 		ITEM_ANTI_EVIL, 	TRUE	},
    {	"anti-neutral", 	ITEM_ANTI_NEUTRAL, 	TRUE	},
    {	"noremove", 		ITEM_NOREMOVE, 		TRUE	},
    {	"inventory", 		ITEM_INVENTORY, 	TRUE	},
    {	"poisoned", 		ITEM_POISONED, 		TRUE	},
    {	"vampire_bane", 	ITEM_VAMPIRE_BANE, 	TRUE	},
    {	"holy", 		ITEM_HOLY, 		TRUE	},
    {	"visible_death", 	ITEM_VIS_DEATH, 	TRUE	},
    {	"nosave",	 	ITEM_NOSAVE,	 	TRUE	},
    {	"", 			0, 			0	}
};


const	FLAG_TYPE		wear_flags	[ ]	=
{
    {	"take", 		ITEM_TAKE, 		TRUE	},
    {	"finger", 		ITEM_WEAR_FINGER, 	TRUE	},
    {	"neck", 		ITEM_WEAR_NECK, 	TRUE	},
    {	"body", 		ITEM_WEAR_BODY, 	TRUE	},
    {	"head", 		ITEM_WEAR_HEAD, 	TRUE	},
    {	"legs", 		ITEM_WEAR_LEGS, 	TRUE	},
    {	"feet",			ITEM_WEAR_FEET, 	TRUE	},
    {	"hands", 		ITEM_WEAR_HANDS, 	TRUE	},
    {	"arms", 		ITEM_WEAR_ARMS, 	TRUE	},
    {	"shield", 		ITEM_WEAR_SHIELD, 	TRUE	},
    {	"about", 		ITEM_WEAR_ABOUT, 	TRUE	},
    {	"waist", 		ITEM_WEAR_WAIST, 	TRUE	},
    {	"wrist", 		ITEM_WEAR_WRIST, 	TRUE	},
    {	"wield", 		ITEM_WIELD, 		TRUE	},
    {	"hold", 		ITEM_HOLD, 		TRUE	},
    {	"missile", 		ITEM_MISSILE_WIELD, 	TRUE	},
    {	"", 			0, 			0	}
};


const	FLAG_TYPE		act_flags	[ ]	=
{
    {	"npc", 			ACT_IS_NPC, 		FALSE	},
    {	"sentinel", 		ACT_SENTINEL, 		TRUE	},
    {	"scavenger", 		ACT_SCAVENGER, 		TRUE	},
    {	"aggressive", 		ACT_AGGRESSIVE, 	TRUE	},
    {	"stay_area", 		ACT_STAY_AREA, 		TRUE	},
    {	"wimpy", 		ACT_WIMPY, 		TRUE	},
    {	"pet", 			ACT_PET, 		TRUE	},
    {	"train", 		ACT_TRAIN, 		TRUE	},
    {	"practice", 		ACT_PRACTICE, 		TRUE	},
    {	"mobinvis", 		ACT_MOBINVIS, 		TRUE	},
    {	"nokill",		ACT_NOKILL,		TRUE	},
    {	"", 			0, 			0	}
};


const	FLAG_TYPE		npc_flags	[ ]	=
{
    {	"bondable", 		ACT_BONDABLE, 		TRUE	},
    {	"", 			0, 			0	}
};

const	FLAG_TYPE		plr_flags	[ ]	=
{
    {	"npc", 			PLR_IS_NPC,		FALSE	},
    {	"bought_pet", 		PLR_BOUGHT_PET,		TRUE	},
    {	"register", 		PLR_REGISTER,		TRUE	},
    {	"autoexit", 		PLR_AUTOEXIT,		TRUE	},
    {	"autoloot", 		PLR_AUTOLOOT,		TRUE	},
    {	"autosac", 		PLR_AUTOSAC,		TRUE	},
    {	"blank", 		PLR_BLANK,		TRUE	},
    {	"brief", 		PLR_BRIEF,		TRUE	},
    {	"combine", 		PLR_COMBINE,		TRUE	},
    {	"prompt", 		PLR_PROMPT,		TRUE	},
    {	"telnet_ga", 		PLR_TELNET_GA,		TRUE	},
    {	"holylight", 		PLR_HOLYLIGHT,		TRUE	},
    {	"wizinvis", 		PLR_WIZINVIS,		TRUE	},
    {	"wizbit", 		PLR_WIZBIT,		TRUE	},
    {	"silence", 		PLR_SILENCE,		TRUE	},
    {	"no_emote", 		PLR_NO_EMOTE,		TRUE	},
    {	"moved", 		PLR_MOVED,		TRUE	},
    {	"no_tell", 		PLR_NO_TELL,		TRUE	},
    {	"log", 			PLR_LOG,		TRUE	},
    {	"deny", 		PLR_DENY,		TRUE	},
    {	"freeze", 		PLR_FREEZE,		TRUE	},
    {	"thief", 		PLR_THIEF,		TRUE	},
    {	"killer", 		PLR_KILLER,		TRUE	},
    {	"autogold", 		PLR_AUTOGOLD,		TRUE	},
    {	"afk", 			PLR_AFK,		TRUE	},
    {	"colour", 		PLR_COLOUR,		TRUE	},
    {	"edit_info", 		PLR_EDIT_INFO,		TRUE	},
    {	"pager", 		PLR_PAGER,		TRUE	},
    {	"", 			0, 			0	}
};


const	FLAG_TYPE		affect_flags	[ ]	=
{
    {	"blind", 		AFF_BLIND, 		TRUE	},
    {	"invisible", 		AFF_INVISIBLE, 		TRUE	},
    {	"detect_evil", 		AFF_DETECT_EVIL, 	TRUE	},
    {	"detect_invis", 	AFF_DETECT_INVIS, 	TRUE	},
    {	"detect_magic", 	AFF_DETECT_MAGIC, 	TRUE	},
    {	"detect_hidden", 	AFF_DETECT_HIDDEN, 	TRUE	},
    {	"hold", 		AFF_HOLD, 		TRUE	},
    {	"sanctuary", 		AFF_SANCTUARY, 		TRUE	},
    {	"faerie_fire", 		AFF_FAERIE_FIRE, 	TRUE	},
    {	"infrared", 		AFF_INFRARED, 		TRUE	},
    {	"curse", 		AFF_CURSE, 		TRUE	},
    {	"change_sex", 		AFF_CHANGE_SEX, 	FALSE	},
    {	"poison", 		AFF_POISON, 		TRUE	},
    {	"protect_evil",		AFF_PROTECT_EVIL, 	TRUE	},
    {	"polymorph",		AFF_POLYMORPH, 		FALSE	},
    {	"sleep", 		AFF_SLEEP, 		TRUE	},
    {	"sneak", 		AFF_SNEAK, 		TRUE	},
    {	"hide", 		AFF_HIDE, 		TRUE	},
    {	"charm", 		AFF_CHARM, 		TRUE	},
    {	"flying", 		AFF_FLYING, 		TRUE	},
    {	"pass_door", 		AFF_PASS_DOOR, 		TRUE	},
    {	"mute", 		AFF_MUTE, 		TRUE	},
    {	"gills", 		AFF_GILLS, 		TRUE	},
    {	"vamp_bite", 		AFF_VAMP_BITE, 		TRUE	},
    {	"ghoul", 		AFF_GHOUL, 		FALSE	},
    {	"flame_shield", 	AFF_FLAME_SHIELD, 	TRUE	},
    {	"frost_shield", 	AFF_FROST_SHIELD, 	TRUE	},
    {	"shock_shield", 	AFF_SHOCK_SHIELD, 	TRUE	},
    {	"waterwalk", 		AFF_WATERWALK, 		TRUE	},
    {	"summoned", 		AFF_SUMMONED, 		FALSE	},
    {	"detect_good", 		AFF_DETECT_GOOD, 	TRUE	},
    {	"protect_good",		AFF_PROTECT_GOOD, 	TRUE	},
    {	"plague",		AFF_PLAGUE, 		TRUE	},
    {	"cammo", 		AFF_CAMMO, 		TRUE	},
    {	"", 			0, 			0	}
};


/*
 * Used when adding an affect to tell where it goes.
 */
const	FLAG_TYPE		apply_flags	[ ]	=
{
    {	"none", 		APPLY_NONE, 		TRUE	},
    {	"strength", 		APPLY_STR, 		TRUE	},
    {	"dexterity", 		APPLY_DEX, 		TRUE	},
    {	"intelligence", 	APPLY_INT, 		TRUE	},
    {	"wisdom", 		APPLY_WIS, 		TRUE	},
    {	"constitution", 	APPLY_CON, 		TRUE	},
    {	"sex", 			APPLY_SEX, 		TRUE	},
    {	"class", 		APPLY_CLASS, 		TRUE	},
    {	"level", 		APPLY_LEVEL, 		TRUE	},
    {	"age", 			APPLY_AGE, 		TRUE	},
    {	"height", 		APPLY_HEIGHT, 		TRUE	},
    {	"weight", 		APPLY_WEIGHT, 		TRUE	},
    {	"mana", 		APPLY_MANA, 		TRUE	},
    {	"hp", 			APPLY_HIT, 		TRUE	},
    {	"move", 		APPLY_MOVE, 		TRUE	},
    {	"gold", 		APPLY_GOLD, 		TRUE	},
    {	"experience", 		APPLY_EXP, 		TRUE	},
    {	"ac", 			APPLY_AC, 		TRUE	},
    {	"hitroll", 		APPLY_HITROLL, 		TRUE	},
    {	"damroll", 		APPLY_DAMROLL, 		TRUE	},
    {	"saving-para", 		APPLY_SAVING_PARA, 	TRUE	},
    {	"saving-rod", 		APPLY_SAVING_ROD, 	TRUE	},
    {	"saving-petri", 	APPLY_SAVING_PETRI, 	TRUE	},
    {	"saving-breath", 	APPLY_SAVING_BREATH, 	TRUE	},
    {	"saving-spell", 	APPLY_SAVING_SPELL, 	TRUE	},
    {	"race", 		APPLY_RACE, 		TRUE	},
    {	"resistant", 		APPLY_RESISTANT, 	TRUE	},
    {	"immune", 		APPLY_IMMUNE, 		TRUE	},
    {	"susceptible", 		APPLY_SUSCEPTIBLE,	TRUE	},
    {	"", 			0, 			0	}
};


/*
 * What is seen.
 */
const	FLAG_TYPE		wear_loc_strings	[ ]	=
{
    {	"in the inventory", 	WEAR_NONE, 		TRUE	},
    {	"as a light", 		WEAR_LIGHT, 		TRUE	},
    {	"on the left finger", 	WEAR_FINGER_L, 		TRUE	},
    {	"on the right finger", 	WEAR_FINGER_R, 		TRUE	},
    {	"around the neck (1)", 	WEAR_NECK_1, 		TRUE	},
    {	"around the neck (2)", 	WEAR_NECK_2, 		TRUE	},
    {	"on the body", 		WEAR_BODY, 		TRUE	},
    {	"over the head", 	WEAR_HEAD, 		TRUE	},
    {	"on the legs", 		WEAR_LEGS, 		TRUE	},
    {	"on the feet", 		WEAR_FEET, 		TRUE	},
    {	"on the hands", 	WEAR_HANDS, 		TRUE	},
    {	"on the arms", 		WEAR_ARMS, 		TRUE	},
    {	"as a shield", 		WEAR_SHIELD, 		TRUE	},
    {	"about the shoulders", 	WEAR_ABOUT, 		TRUE	},
    {	"around the waist", 	WEAR_WAIST, 		TRUE	},
    {	"on the left wrist", 	WEAR_WRIST_L, 		TRUE	},
    {	"on the right wrist", 	WEAR_WRIST_R, 		TRUE	},
    {	"primary wield", 	WEAR_WIELD, 		TRUE	},
    {	"held in the hands", 	WEAR_HOLD, 		TRUE	},
    {	"second wield", 	WEAR_WIELD_2, 		TRUE	},
    {	"missile wield", 	WEAR_MISSILE_WIELD, 	TRUE	},
    {	"", 			0,			0	}
};


/*
 * What is typed.
 * neck2 should not be settable for loaded mobiles.
 */
const	FLAG_TYPE		wear_loc_flags	[ ]	=
{
    {	"none", 		WEAR_NONE, 		TRUE	},
    {	"light", 		WEAR_LIGHT, 		TRUE	},
    {	"lfinger", 		WEAR_FINGER_L, 		TRUE	},
    {	"rfinger", 		WEAR_FINGER_R, 		TRUE	},
    {	"neck1", 		WEAR_NECK_1, 		TRUE	},
    {	"neck2", 		WEAR_NECK_2, 		TRUE	},
    {	"body", 		WEAR_BODY, 		TRUE	},
    {	"head", 		WEAR_HEAD, 		TRUE	},
    {	"legs", 		WEAR_LEGS, 		TRUE	},
    {	"feet", 		WEAR_FEET, 		TRUE	},
    {	"hands", 		WEAR_HANDS, 		TRUE	},
    {	"arms", 		WEAR_ARMS, 		TRUE	},
    {	"shield", 		WEAR_SHIELD, 		TRUE	},
    {	"about", 		WEAR_ABOUT, 		TRUE	},
    {	"waist", 		WEAR_WAIST, 		TRUE	},
    {	"lwrist", 		WEAR_WRIST_L, 		TRUE	},
    {	"rwrist", 		WEAR_WRIST_R, 		TRUE	},
    {	"wielded", 		WEAR_WIELD, 		TRUE	},
    {	"hold", 		WEAR_HOLD, 		TRUE	},
    {	"sec_wield", 		WEAR_WIELD_2, 		TRUE	},
    {	"mis_wield", 		WEAR_MISSILE_WIELD, 	TRUE	},
    {	"", 			0, 			0	}
};


const	FLAG_TYPE		weapon_flags	[ ]	=
{
    {	"hit", 			0, 		TRUE	},
    {	"slice", 		1, 		TRUE	},
    {	"stab", 		2, 		TRUE	},
    {	"slash", 		3, 		TRUE	},
    {	"whip", 		4, 		TRUE	},
    {	"claw", 		5, 		TRUE	},
    {	"blast", 		6, 		TRUE	},
    {	"pound", 		7, 		TRUE	},
    {	"crush", 		8, 		TRUE	},
    {	"grep", 		9, 		TRUE	},
    {	"bite", 		10, 		TRUE	},
    {	"pierce", 		11, 		TRUE	},
    {	"suction", 		12, 		TRUE	},
    {	"chop", 		13, 		TRUE	},
    {	"vorpal", 		14, 		TRUE	},
    {	"cleave", 		15, 		TRUE	},
    {	"wail", 		16, 		TRUE	},
    {	"", 			0, 		0	}
};


const	FLAG_TYPE		range_type_flags[ ]	=
{
    {	"bow",			RNG_BOW,	TRUE	},
    {	"crossbow",		RNG_CROSSBOW,	TRUE	},
    {	"catapult",		RNG_CATAPULT,	TRUE	},
    {	"",			0,		0	}
};

const	FLAG_TYPE		ris_flags	[ ]	=
{
    {	"none",	     		RIS_NONE,	FALSE	},
    {	"fire",	     		RIS_FIRE,	TRUE	},
    {	"cold",	     		RIS_COLD,	TRUE	},
    {	"electricity",  	RIS_ELECTRICITY,TRUE	},
    {	"energy",    		RIS_ENERGY,	TRUE	},
    {	"acid",	     		RIS_ACID,	TRUE	},
    {	"poison",    		RIS_POISON,	TRUE	},
    {	"charm",     		RIS_CHARM,	TRUE	},
    {	"mental",    		RIS_MENTAL,	TRUE	},
    {	"white_mana",		RIS_WHITE_MANA, TRUE	},
    {	"black_mana",		RIS_BLACK_MANA, TRUE	},
    {	"disease",   		RIS_DISEASE,	TRUE	},
    {	"drowning",  		RIS_DROWNING,	TRUE	},
    {	"light",     		RIS_LIGHT,	TRUE	},
    {	"sound",     		RIS_SOUND,	TRUE	},
    {	"magic",     		RIS_MAGIC,	TRUE	},
    {	"nonmagic",  		RIS_NONMAGIC,	TRUE	},
    {	"silver",    		RIS_SILVER,	TRUE	},
    {	"iron",	     		RIS_IRON,	TRUE	},
    {	"wood",	     		RIS_WOOD,	TRUE	},
    {	"weapon",    		RIS_WEAPON,	TRUE	},
    {	"bash",	     		RIS_BASH,	TRUE	},
    {	"pierce",    		RIS_PIERCE,	TRUE	},
    {	"slash",     		RIS_SLASH,	TRUE	},
    {	"",			0,		0	}
};


const	FLAG_TYPE		position_flags	[ ]	=
{
    {	"dead",	    		POS_DEAD,	TRUE	},
    {	"mort",			POS_MORTAL,	TRUE	},
    {	"incap",		POS_INCAP,	TRUE	},
    {	"stun",  		POS_STUNNED,	TRUE	},
    {	"sleep", 		POS_SLEEPING,	TRUE	},
    {	"rest",  		POS_RESTING,	TRUE	},
    {	"fight", 		POS_FIGHTING,	TRUE	},
    {	"stand", 		POS_STANDING,	TRUE	},
    {	"",	    		0,		0	}
};


const	FLAG_TYPE		mp_flags	[ ]	=
{
    {	"act",			TRIG_ACT,	TRUE	},
    {	"bribe",		TRIG_BRIBE,	TRUE	},
    {	"death",		TRIG_DEATH,	TRUE	},
    {	"entry",		TRIG_ENTRY,	TRUE	},
    {	"fight",		TRIG_FIGHT,	TRUE	},
    {	"give",			TRIG_GIVE,	TRUE	},
    {	"greet",		TRIG_GREET,	TRUE	},
    {	"grall",		TRIG_GRALL,	TRUE	},
    {	"kill",			TRIG_KILL,	TRUE	},
    {	"hpcnt",		TRIG_HPCNT,	TRUE	},
    {	"random",		TRIG_RANDOM,	TRUE	},
    {	"speech",		TRIG_SPEECH,	TRUE	},
    {	"exit",			TRIG_EXIT,	TRUE	},
    {	"exall",		TRIG_EXALL,	TRUE	},
    {	"delay",		TRIG_DELAY,	TRUE	},
    {	"surr",			TRIG_SURR,	TRUE	},
    {	"",	    		0,		0	}
};


const	FLAG_TYPE		container_flags	[ ]	=
{
    {	"closeable", 		1, 		TRUE	},
    {	"pickproof", 		2, 		TRUE	},
    {	"closed", 		4, 		TRUE	},
    {	"locked", 		8, 		TRUE	},
    {	"", 			0, 		0	}
};


const	FLAG_TYPE		portal_door_flags	[ ] =
{
    {	"closeable", 		1, 		TRUE	},
    {	"pickproof", 		2, 		TRUE	},
    {	"closed", 		4, 		TRUE	},
    {	"locked", 		8, 		TRUE	},
    {	"", 			0, 		0	}
};

const	FLAG_TYPE		portal_flags	[ ]	=
{
    {	"nocursed", 		1, 		TRUE	},
    {	"gowithchar", 		2, 		TRUE	},
    {	"random", 		4, 		TRUE	},
    {	"buggy", 		8, 		TRUE	},
    {	"", 			0, 		0	}
};

const	FLAG_TYPE		mana_flags	[ ]	=
{
    {	"mana_earth", 		MANA_EARTH, 	TRUE	},
    {	"mana_air", 		MANA_AIR, 	TRUE	},
    {	"mana_fire", 		MANA_FIRE, 	TRUE	},
    {	"mana_water", 		MANA_WATER, 	TRUE	},
    {	"", 			0, 		0	}
};

const	FLAG_TYPE		liquid_flags	[ ]	=
{
    {	"water", 		0, 		TRUE	},
    {	"beer", 		1, 		TRUE	},
    {	"wine", 		2, 		TRUE	},
    {	"ale", 			3, 		TRUE	},
    {	"dark-ale", 		4, 		TRUE	},
    {	"whisky", 		5, 		TRUE	},
    {	"lemonade", 		6, 		TRUE	},
    {	"firebreather", 	7, 		TRUE	},
    {	"local-specialty", 	8, 		TRUE	},
    {	"slime-mold-juice", 	9, 		TRUE	},
    {	"milk", 		10, 		TRUE	},
    {	"tea", 			11, 		TRUE	},
    {	"coffee", 		12, 		TRUE	},
    {	"blood", 		13, 		TRUE	},
    {	"salt-water", 		14, 		TRUE	},
    {	"cola", 		15, 		TRUE	},
    {	"white wine", 		16, 		TRUE	},
    {	"root beer", 		17, 		TRUE	},
    {	"champagne", 		18, 		TRUE	},
    {	"vodka", 		19, 		TRUE	},
    {	"absinth", 		20, 		TRUE	},
    {	"brandy", 		21, 		TRUE	},
    {	"schnapps", 		22, 		TRUE	},
    {	"orange juice", 	23, 		TRUE	},
    {	"sherry", 		24, 		TRUE	},
    {	"rum", 			25, 		TRUE	},
    {	"port", 		26, 		TRUE	},
    {	"", 			0, 		0	}
};

const	FLAG_TYPE		clan_flags	[ ]	=
{
    {	"clan",			CLAN_PLAIN,	TRUE	},
    {	"npk_clan",		CLAN_NOKILL,	TRUE	},
    {	"order",		CLAN_ORDER,	TRUE	},
    {	"guild",		CLAN_GUILD,	TRUE	},
    {	"",			0,		0	}
};


const	FLAG_TYPE		rank_flags	[ ]	=
{
    {	"exiled",		RANK_EXILED,	TRUE	},
    {	"clansman",		RANK_CLANSMAN,	TRUE	},
    {	"clanhero",		RANK_CLANHERO,	TRUE	},
    {	"subchief",		RANK_SUBCHIEF,	TRUE	},
    {	"chieftain",		RANK_CHIEFTAIN,	TRUE	},
    {	"overlord",		RANK_OVERLORD,	TRUE	},
    {	"",			0,		0	}
};


struct wear_type
{
    int	wear_loc;
    int	wear_bit;
};

const	struct	wear_type	wear_table	[ ]	=
{
    {	WEAR_NONE, 		ITEM_TAKE		},
    {	WEAR_LIGHT, 		ITEM_LIGHT		},
    {	WEAR_FINGER_L, 		ITEM_WEAR_FINGER	},
    {	WEAR_FINGER_R, 		ITEM_WEAR_FINGER	},
    {	WEAR_NECK_1, 		ITEM_WEAR_NECK		},
    {	WEAR_NECK_2, 		ITEM_WEAR_NECK		},
    {	WEAR_BODY, 		ITEM_WEAR_BODY		},
    {	WEAR_HEAD, 		ITEM_WEAR_HEAD		},
    {	WEAR_LEGS, 		ITEM_WEAR_LEGS		},
    {	WEAR_FEET, 		ITEM_WEAR_FEET		},
    {	WEAR_HANDS, 		ITEM_WEAR_HANDS		},
    {	WEAR_ARMS, 		ITEM_WEAR_ARMS		},
    {	WEAR_SHIELD, 		ITEM_WEAR_SHIELD	},
    {	WEAR_ABOUT, 		ITEM_WEAR_ABOUT		},
    {	WEAR_WAIST, 		ITEM_WEAR_WAIST		},
    {	WEAR_WRIST_L, 		ITEM_WEAR_WRIST		},
    {	WEAR_WRIST_R, 		ITEM_WEAR_WRIST		},
    {	WEAR_WIELD, 		ITEM_WIELD		},
    {	WEAR_HOLD, 		ITEM_HOLD		},
    {	WEAR_MISSILE_WIELD, 	ITEM_MISSILE_WIELD	},
    {	NO_FLAG, 		NO_FLAG			}
};

int wear_loc( int bits, int count )
{
    int flag;

    for ( flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++ )
	if ( IS_SET( bits, wear_table[flag].wear_bit ) && --count < 1 )
	    return wear_table[flag].wear_loc;

    return NO_FLAG;
}


int wear_bit( int loc )
{
    int flag;

    for ( flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++ )
	if ( loc == wear_table[flag].wear_loc )
	    return wear_table[flag].wear_bit;

    return 0;
}
