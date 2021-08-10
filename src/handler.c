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
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "merc.h"



/*
 * Local functions.
 */
void    affect_modify          args( ( CHAR_DATA *ch, AFFECT_DATA *paf,
				      bool fAdd ) );



/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    if ( ch->desc && ch->desc->original )
	ch = ch->desc->original;

    if ( ch->trust != 0 )
	return ch->trust;

    if ( IS_NPC( ch ) && ch->level >= LEVEL_HERO )
	return LEVEL_HERO - 1;
    else
	return ch->level;
}



/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return 17 + ( ch->played + (int) ( current_time - ch->logon ) ) / 428400;

    /* 428400 assumes 30 secs/mud hour * 24 hours/day * 35 days/month *
       17 months/year - Kahn */
}



/*
 * Retrieve character's current strength.
 */
int get_curr_str( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].str_mod;

    value = 13 + mod;

    if ( IS_NPC ( ch ) || ch->cclass[0]->attr_prime == APPLY_STR )
	max = UMIN( 25, 25 + mod );
    else
	max = UMIN( 22 + mod, 25 );

    return URANGE( 3, ch->perm_str + ch->mod_str, max );
}



/*
 * Retrieve character's maximum strength.
 */
int get_max_str( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].str_mod;
    value = 13 + mod;

    if ( IS_NPC ( ch ) || ch->cclass[0]->attr_prime == APPLY_STR )
	max = UMIN( 25, 25 + mod );
    else
	max = UMIN( 22 + mod, 25 );

    return max;
}



/*
 * Retrieve character's current intelligence.
 */
int get_curr_int( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].int_mod;
    value = 13 + mod;

    if ( IS_NPC ( ch ) || ch->cclass[0]->attr_prime == APPLY_INT )
	max = UMIN( 25, 25 + mod );
    else
	max = UMIN( 22 + mod, 25 );

    return URANGE( 3, ch->perm_int + ch->mod_int, max );
}



/*
 * Retrieve character's maximum intelligence.
 */
int get_max_int( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].int_mod;
    value = 13 + mod;

    if ( IS_NPC ( ch ) || ch->cclass[0]->attr_prime == APPLY_INT )
	max = UMIN( 25, 25 + mod );
    else
	max = UMIN( 22 + mod, 25 );

    return max;
}



/*
 * Retrieve character's current wisdom.
 */
int get_curr_wis( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].wis_mod;
    value = 13 + mod;

    if ( IS_NPC ( ch ) || ch->cclass[0]->attr_prime == APPLY_WIS )
	max = UMIN( 25, 25 + mod );
    else
	max = UMIN( 22 + mod, 25 );

    return URANGE( 3, ch->perm_wis + ch->mod_wis, max );
}



/*
 * Retrieve character's maximum wisdom.
 */
int get_max_wis( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].wis_mod;
    value = 13 + mod;

    if ( IS_NPC ( ch ) || ch->cclass[0]->attr_prime == APPLY_WIS )
	max = UMIN( 25, 25 + mod );
    else
	max = UMIN( 22 + mod, 25 );

    return max;
}



/*
 * Retrieve character's current dexterity.
 */
int get_curr_dex( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].dex_mod;
    value = 13 + mod;

    if ( IS_NPC ( ch ) || ch->cclass[0]->attr_prime == APPLY_DEX )
	max = UMIN( 25, 25 + mod );
    else
	max = UMIN( 22 + mod, 25 );

    return URANGE( 3, ch->perm_dex + ch->mod_dex, max );
}



/*
 * Retrieve character's maximum dexterity.
 */
int get_max_dex( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].dex_mod;
    value = 13 + mod;

    if ( IS_NPC ( ch ) || ch->cclass[0]->attr_prime == APPLY_DEX )
	max = UMIN( 25, 25 + mod );
    else
	max = UMIN( 22 + mod, 25 );

    return max;
}



/*
 * Retrieve character's current constitution.
 */
int get_curr_con( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].con_mod;
    value = 13 + mod;

    if ( IS_NPC ( ch ) || ch->cclass[0]->attr_prime == APPLY_CON )
	max = UMIN( 25, 25 + mod );
    else
	max = UMIN( 22 + mod, 25 );

    return URANGE( 3, ch->perm_con + ch->mod_con, max );
}



/*
 * Retrieve character's maximum constitution.
 */
int get_max_con( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].con_mod;
    value = 13 + mod;

    if ( IS_NPC ( ch ) || ch->cclass[0]->attr_prime == APPLY_CON )
	max = UMIN( 25, 25 + mod );
    else
	max = UMIN( 22 + mod, 25 );

    return max;
}



/*
 * Retrieve character's current hitroll for given weapon location
 */
int get_hitroll( CHAR_DATA *ch, int wpn )
{
    OBJ_DATA    *other_wield;
    AFFECT_DATA *paf;
    int          other_wpn;
    int          hitroll;

    if ( wpn == WEAR_WIELD)
        other_wpn = WEAR_WIELD_2;
    else if ( wpn == WEAR_WIELD_2 )
        other_wpn = WEAR_WIELD;
    else
    {
        char buf [ MAX_STRING_LENGTH ];
        sprintf( buf, "get_hitroll: Invalid weapon location %d on %s.",
		wpn, ch->name );
        bug( buf, 0 );
        return 0;
    }

    hitroll = ch->hitroll + str_app[get_curr_str( ch )].tohit;
    if ( !( other_wield = get_eq_char( ch, other_wpn ) ) )
        return hitroll;

    for( paf = other_wield->pIndexData->affected; paf; paf = paf->next )
        if ( paf->location == APPLY_HITROLL )
            hitroll -= paf->modifier;
    for( paf = other_wield->affected; paf; paf = paf->next )
        if ( paf->location == APPLY_HITROLL )
            hitroll -= paf->modifier;

    return hitroll;
}



/*
 * Retrieve character's current damroll for given weapon location
 */
int get_damroll( CHAR_DATA *ch, int wpn )
{
    OBJ_DATA    *other_wield;
    AFFECT_DATA *paf;
    int          other_wpn;
    int          damroll;

    if ( wpn == WEAR_WIELD)
        other_wpn = WEAR_WIELD_2;
    else if ( wpn == WEAR_WIELD_2 )
        other_wpn = WEAR_WIELD;
    else
    {
        char buf [ MAX_STRING_LENGTH ];
        sprintf( buf, "get_damroll: Invalid weapon location %d on %s.",
		wpn, ch->name );
        bug( buf, 0 );
        return 0;
    }

    damroll = ch->damroll + str_app[get_curr_str( ch )].todam;
    if ( !( other_wield = get_eq_char( ch, other_wpn ) ) )
        return damroll;

    for( paf = other_wield->pIndexData->affected; paf; paf = paf->next )
        if ( paf->location == APPLY_DAMROLL )
            damroll -= paf->modifier;
    for( paf = other_wield->affected; paf; paf = paf->next )
        if ( paf->location == APPLY_DAMROLL )
            damroll -= paf->modifier;

    return damroll;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_PET ) )
	return 0;

    return MAX_WEAR + 2 * get_curr_dex( ch ) / 2;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
	return 1000000;

    if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_PET ) )
	return 0;

    return str_app[get_curr_str( ch )].carry;
}



/*
 * See if a string is one of the names of an object.
 * New is_name sent in by Alander.
 */
bool is_name( const char *str, char *namelist )
{
    char name [ MAX_INPUT_LENGTH ];

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
	if ( !str_cmp( str, name ) )
	    return TRUE;
    }
}


/*
 * See if a string is prefix of one of the names of an object.
 */
bool is_name_prefix( const char *str, char *namelist )
{
    char name [ MAX_INPUT_LENGTH ];

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
	if ( !str_prefix( str, name ) )
	    return TRUE;
    }
}


/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    OBJ_DATA *wield2;
    char      buf [ MAX_STRING_LENGTH ];
    int       mod;

    mod = paf->modifier;

    if ( fAdd )
        xSET_BITS( ch->affected_by, paf->bitvector );
    else
    {
        xREMOVE_BITS( ch->affected_by, paf->bitvector );
	switch ( paf->location )
	{
	case APPLY_RESISTANT:	REMOVE_BIT( ch->resistant, mod );	return;
	case APPLY_IMMUNE:	REMOVE_BIT( ch->immune, mod );		return;
	case APPLY_SUSCEPTIBLE:	REMOVE_BIT( ch->susceptible, mod );	return;
	}
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
        sprintf( buf, "Affect_modify: unknown location %d on %s.",
		paf->location, ch->name );
	bug ( buf, 0 );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:           ch->mod_str		+= mod; break;
    case APPLY_DEX:           ch->mod_dex		+= mod; break;
    case APPLY_INT:           ch->mod_int		+= mod; break;
    case APPLY_WIS:           ch->mod_wis		+= mod; break;
    case APPLY_CON:           ch->mod_con		+= mod; break;
    case APPLY_SEX:           ch->sex                   += mod; break;
    case APPLY_RACE:          ch->race                  += mod; break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana              += mod; break;
    case APPLY_HIT:           ch->max_hit               += mod; break;
    case APPLY_MOVE:          ch->max_move              += mod; break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:            ch->armor                 += mod; break;
    case APPLY_HITROLL:       ch->hitroll               += mod; break;
    case APPLY_DAMROLL:       ch->damroll               += mod; break;
    case APPLY_SAVING_PARA:   ch->saving_throw          += mod; break;
    case APPLY_SAVING_ROD:    ch->saving_throw          += mod; break;
    case APPLY_SAVING_PETRI:  ch->saving_throw          += mod; break;
    case APPLY_SAVING_BREATH: ch->saving_throw          += mod; break;
    case APPLY_SAVING_SPELL:  ch->saving_throw          += mod; break;
    case APPLY_RESISTANT:     SET_BIT( ch->resistant, mod );    break;
    case APPLY_IMMUNE:        SET_BIT( ch->immune, mod );       break;
    case APPLY_SUSCEPTIBLE:   SET_BIT( ch->susceptible, mod );  break;
    }

    /* Remove the excess general stats */
    ch->hit  = UMIN( ch->hit, ch->max_hit );
    ch->mana = UMIN( ch->mana, ch->max_mana );
    ch->move = UMIN( ch->move, ch->max_move );

    if ( IS_NPC( ch ) )
        return;

    /*
     * Check for PC weapon wielding.
     * Guard against recursion (for weapons with affects).
     * If more than one weapon, drop weapon 2 first, then recheck.
     * And yes, it does work.  :)  --- Thelonius (Monk)
     */
    if ( ( wield  = get_eq_char( ch, WEAR_WIELD ) ) )
    {
	if ( ( wield2 = get_eq_char( ch, WEAR_WIELD_2 ) ) )
	{
	    if ( ( ( get_obj_weight( wield ) + get_obj_weight( wield2 ) )
		  > str_app[get_curr_str( ch )].wield )
		|| !IS_SET( race_table[ ch->race ].race_abilities,
			   RACE_WEAPON_WIELD ) )
	    {
		static int depth;

		if ( depth == 0 )
		{
		    depth++;
		    act( "You drop $p.", ch, wield2, NULL, TO_CHAR );
		    act( "$n drops $p.", ch, wield2, NULL, TO_ROOM );
		    obj_from_char( wield2 );
		    obj_to_room( wield2, ch->in_room );
		    depth--;
		}

	    }
	}
	else
	if ( ( get_obj_weight( wield ) > str_app[get_curr_str( ch )].wield )
	    || !IS_SET( race_table[ ch->race ].race_abilities,
		       RACE_WEAPON_WIELD ) )
	{
	    static int depth;

	    if ( depth == 0 )
	    {
		depth++;
		act( "You drop $p.", ch, wield, NULL, TO_CHAR );
		act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
		obj_from_char( wield );
		obj_to_room( wield, ch->in_room );
		depth--;
	    }

	}
    }
    else if ( ( wield2 = get_eq_char( ch, WEAR_WIELD_2 ) )
             && ( get_obj_weight( wield2 ) > str_app[get_curr_str( ch )].wield
		 || !IS_SET( race_table[ ch->race ].race_abilities,
			    RACE_WEAPON_WIELD ) ) )
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield2, NULL, TO_CHAR );
	    act( "$n drops $p.", ch, wield2, NULL, TO_ROOM );
	    obj_from_char( wield2 );
	    obj_to_room( wield2, ch->in_room );
	    depth--;
	}

    }

    return;
}



/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new		= new_affect();

    *paf_new		= *paf;
    paf_new->deleted    = FALSE;
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}



/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    if ( !ch->affected )
    {
	bug( "Affect_remove: no affect.", 0 );
	return;
    }

    affect_modify( ch, paf, FALSE );

    paf->deleted = TRUE;

    return;
}



/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;

    for ( paf_old = ch->affected; paf_old; paf_old = paf_old->next )
    {
        if ( paf_old->deleted )
	    continue;
	if ( paf_old->type == paf->type )
	{
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}



/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    CHAR_DATA **last;
    OBJ_DATA   *obj;

    if ( !ch->in_room )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if ( !IS_NPC( ch ) )
	--ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) )
	&& obj->item_type == ITEM_LIGHT
	&& obj->value[2] != 0
	&& ch->in_room->light > 0 )
	--ch->in_room->light;

    for ( last = &ch->in_room->people; *last; last = &(*last)->next_in_room )
    {
	if ( (*last) == ch )
	{
	    *last = ch->next_in_room;
	    break;
	}
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;

    if ( ch->riding )
	char_from_room( ch->riding );

    return;
}



/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if ( !pRoomIndex )
    {
	bug( "Char_to_room: NULL.", 0 );
	return;
    }

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

    if ( !IS_NPC( ch ) )
	++ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) )
	&& obj->item_type == ITEM_LIGHT
	&& obj->value[2] != 0 )
	++ch->in_room->light;

    if ( ch->riding )
	char_to_room( ch->riding, pRoomIndex );

    return;
}



/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    OBJ_DATA  **last;
    CHAR_DATA  *ch;

    if ( !( ch = obj->carried_by ) )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    for ( last = &ch->carrying; *last; last = &(*last)->next_content )
    {
	if ( (*last) == obj )
	{
	    *last = obj->next_content;
	    break;
	}
    }

    obj->carried_by      = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );
    return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear )
{
    if ( obj->item_type != ITEM_ARMOR && obj->item_type != ITEM_CLOTHING )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:     return 3 * obj->value[0];
    case WEAR_HEAD:	return 2 * obj->value[0];
    case WEAR_LEGS:	return 2 * obj->value[0];
    case WEAR_FEET:	return     obj->value[0];
    case WEAR_HANDS:	return     obj->value[0];
    case WEAR_ARMS:	return     obj->value[0];
    case WEAR_SHIELD:	return     obj->value[0];
    case WEAR_FINGER_L:	return     obj->value[0];
    case WEAR_FINGER_R: return     obj->value[0];
    case WEAR_NECK_1:	return     obj->value[0];
    case WEAR_NECK_2:	return     obj->value[0];
    case WEAR_ABOUT:	return 2 * obj->value[0];
    case WEAR_WAIST:	return     obj->value[0];
    case WEAR_WRIST_L:	return     obj->value[0];
    case WEAR_WRIST_R:	return     obj->value[0];
    case WEAR_HOLD:	return     obj->value[0];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
        if ( obj->deleted )
	    continue;
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    char         buf [ MAX_STRING_LENGTH ];

    if ( get_eq_char( ch, iWear ) )
    {
        sprintf( buf, "Equip_char: %s already equipped at %d.",
		ch->name, iWear );
	bug( buf, 0 );
	return;
    }

    if (   ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL   ) && IS_EVIL   ( ch ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD   ) && IS_GOOD   ( ch ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL( ch ) ) )
    {
	/*
	 * Thanks to Morgenes for the bug fix here!
	 */
	act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
	act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	return;
    }

    ch->armor      	-= apply_ac( obj, iWear );
    obj->wear_loc	 = iWear;

    for ( paf = obj->pIndexData->affected; paf; paf = paf->next )
	affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf; paf = paf->next )
	affect_modify( ch, paf, TRUE );

    if ( obj->item_type == ITEM_LIGHT
	&& obj->value[2] != 0
	&& ch->in_room )
	++ch->in_room->light;

    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf;
    char         buf [ MAX_STRING_LENGTH ];

    if ( obj->wear_loc == WEAR_NONE )
    {
        sprintf( buf, "Unequip_char: %s already unequipped with %d.",
		ch->name, obj->pIndexData->vnum );
	bug( buf, 0 );
	return;
    }

    ch->armor		+= apply_ac( obj, obj->wear_loc );
    obj->wear_loc	 = -1;

    for ( paf = obj->pIndexData->affected; paf; paf = paf->next )
	affect_modify( ch, paf, FALSE );
    for ( paf = obj->affected; paf; paf = paf->next )
	affect_modify( ch, paf, FALSE );

    if ( obj->item_type == ITEM_LIGHT
	&& obj->value[2] != 0
	&& ch->in_room
	&& ch->in_room->light > 0 )
	--ch->in_room->light;

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int       nMatch;

    nMatch = 0;
    for ( obj = list; obj; obj = obj->next_content )
    {
        if ( obj->deleted )
	    continue;
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA  *in_room;
    OBJ_DATA        **last;

    if ( !( in_room = obj->in_room ) )
    {
	bug( "obj_from_room: NULL.", 0 );
	return;
    }

    for ( last = &in_room->contents; *last; last = &(*last)->next_content )
    {
	if ( (*last) == obj )
	{
	    *last = obj->next_content;
	    break;
	}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    if ( obj_to->deleted )
    {
	bug( "Obj_to_obj:  Obj_to already deleted", 0 );
        return;
    }

    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;

    for ( ; obj_to; obj_to = obj_to->in_obj )
    {
        if ( obj_to->deleted )
	    continue;
	if ( obj_to->carried_by )
	{
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj );
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA  *obj_from;
    OBJ_DATA **last;

    if ( !( obj_from = obj->in_obj ) )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    for ( last = &obj_from->contains; *last; last = &(*last)->next_content )
    {
	if ( (*last) == obj )
	{
	    *last = obj->next_content;
	    break;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from; obj_from = obj_from->in_obj )
    {
        if ( obj_from->deleted )
	    continue;
	if ( obj_from->carried_by )
	{
	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj );
	}
    }

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
           OBJ_DATA *obj_content;
           OBJ_DATA *obj_next;
    extern bool      delete_obj;

    if ( obj->deleted )
    {
	bug( "Extract_obj:  Obj already deleted", 0 );
	return;
    }

         if ( obj->in_room    )
	obj_from_room( obj );
    else if ( obj->carried_by )
	obj_from_char( obj );
    else if ( obj->in_obj     )
	obj_from_obj( obj  );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
        obj_next = obj_content->next_content;
	if( obj_content->deleted )
	    continue;
	extract_obj( obj_content );
    }

    obj->deleted = TRUE;

    delete_obj   = TRUE;
    return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
           CHAR_DATA *wch;
           OBJ_DATA  *obj;
           OBJ_DATA  *obj_next;
    extern bool       delete_char;

    if ( !ch->in_room )
    {
	bug( "Extract_char: NULL.", 0 );
	return;
    }

    if ( ch->fighting )
        stop_fighting( ch, TRUE );

    if ( fPull )
    {
	char* name;

	if ( IS_NPC ( ch ) )
	    name = ch->short_descr;
	else
	    name = ch->name;

	die_follower( ch, name );

	/* Get rid of weapons _first_ 
	   - from Erwin Andreasen <erwin@pip.dknet.dk> */

	{
	    OBJ_DATA *obj, *obj2;

	    obj  = get_eq_char( ch, WEAR_WIELD   );
	    obj2 = get_eq_char( ch, WEAR_WIELD_2 );

	    if ( obj )
	        extract_obj( obj );

	    /* Now kill obj2 if it exists no matter if on body or floor */
	    if ( obj2 )
	        extract_obj( obj2 );

	}

	for ( obj = ch->carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->deleted )
	      continue;
	    extract_obj( obj );
	}
     }
    
    char_from_room( ch );

    if ( !fPull )
    {
        ROOM_INDEX_DATA *location;

	if ( !( location = get_room_index( ROOM_VNUM_PURGATORY_A ) ) )
	  {
	    bug( "Purgatory A does not exist!", 0 );
	    char_to_room( ch, get_room_index( ROOM_VNUM_ALTAR ) );
	  }
	else
	    char_to_room( ch, location );
	return;
    }

    if ( IS_NPC( ch ) )
	--ch->pIndexData->count;

    if ( ch->desc && ch->desc->original )
	do_return( ch, "" );

    for ( wch = char_list; wch; wch = wch->next )
    {
	if ( wch->reply == ch )
	    wch->reply = NULL;
    }

    ch->deleted = TRUE;

    if ( ch->desc )
	ch->desc->character = NULL;

    delete_char = TRUE;
    return;
}



/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char       arg [ MAX_INPUT_LENGTH ];
    int        number;
    int        count;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !str_cmp( arg, "self" ) )
	return ch;
    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
	    continue;
	if ( ++count == number )
	    return rch;
    }

    count  = 0;
    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( !can_see( ch, rch ) || !is_name_prefix( arg, rch->name ) )
	    continue;
	if ( ++count == number )
	    return rch;
    }

    return NULL;
}




/*
 * Find a char in an area.
 */
CHAR_DATA *get_char_area( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *ach;
    char       arg [ MAX_INPUT_LENGTH ];
    int        number;
    int        count;

    if ( ( ach = get_char_room( ch, argument ) ) )
	return ach;

    number = number_argument( argument, arg );
    count  = 0;
    for ( ach = char_list; ach ; ach = ach->next )
    {
	if ( !ach->in_room || !ch->in_room
	    || ach->in_room->area != ch->in_room->area
	    || !can_see( ch, ach )
	    || !is_name( arg, ach->name ) )
	    continue;
	if ( ++count == number )
	    return ach;
    }

    count  = 0;
    for ( ach = char_list; ach ; ach = ach->next )
    {
	if ( !ach->in_room || !ch->in_room
	    || ach->in_room->area != ch->in_room->area
	    || !can_see( ch, ach )
	    || !is_name_prefix( arg, ach->name ) )
	    continue;
	if ( ++count == number )
	    return ach;
    }

    return NULL;
}




/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *wch;
    char       arg [ MAX_INPUT_LENGTH ];
    int        number;
    int        count;

    if ( ( wch = get_char_room( ch, argument ) ) )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch ; wch = wch->next )
    {
	if ( !can_see( ch, wch ) || !is_name( arg, wch->name ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    count  = 0;
    for ( wch = char_list; wch ; wch = wch->next )
    {
	if ( !can_see( ch, wch ) || !is_name_prefix( arg, wch->name ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj; obj = obj->next )
    {
        if ( obj->deleted )
	    continue;

	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       number;
    int       count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    count  = 0;
    for ( obj = list; obj; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) && is_name_prefix( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       number;
    int       count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	    && can_see_obj( ch, obj )
	    && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    count  = 0;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	    && can_see_obj( ch, obj )
	    && is_name_prefix( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       number;
    int       count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	    && can_see_obj( ch, obj )
	    && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    count  = 0;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	    && can_see_obj( ch, obj )
	    && is_name_prefix( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       number;
    int       count;

    if ( ( obj = get_obj_here( ch, argument ) ) )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    count  = 0;
    for ( obj = object_list; obj; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name_prefix( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int amount )
{
    OBJ_DATA *obj;

    if ( amount <= 0 )
    {
	bug( "Create_money: zero or negative money %d.", amount );
	amount = 1;
    }

    if ( amount == 1 )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE  ), 0 );
    }
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
    }

    obj->value[0]		= amount;
    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;

    number = 0;
    if ( obj->item_type == ITEM_CONTAINER )
        for ( obj = obj->contains; obj; obj = obj->next_content )
	{
	    if ( obj->deleted )
	        continue;
	    number += get_obj_number( obj );
	}
    else
	number = 1;

    return number;
}



/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;

    weight = obj->weight;
    for ( obj = obj->contains; obj; obj = obj->next_content )
    {
	if ( obj->deleted )
	    continue;
	weight += get_obj_weight( obj );
    }

    return weight;
}



/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if ( pRoomIndex->light > 0 )
	return FALSE;

    for ( obj = pRoomIndex->contents; obj; obj = obj->next_content )
    {
	if ( obj->deleted )
	    continue;
	if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	    return FALSE;
    }

    if ( IS_SET( pRoomIndex->room_flags, ROOM_DARK ) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
	|| pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
	|| weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}



/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int        count;

    count = 0;
    for ( rch = pRoomIndex->people; rch; rch = rch->next_in_room )
    {
	if ( rch->deleted )
	    continue;

	count++;
    }

    if ( IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE  ) && count >= 2 )
	return TRUE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) && count >= 1 )
	return TRUE;

    return FALSE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( victim->deleted )
        return FALSE;

    if ( ch == victim )
	return TRUE;

    /* All mobiles cannot see wizinvised immorts */
    if ( IS_NPC( ch )
	&& !IS_NPC( victim ) && xIS_SET( victim->act, PLR_WIZINVIS ) )
        return FALSE;
	
    if ( !IS_NPC( victim )
	&& xIS_SET( victim->act, PLR_WIZINVIS )
	&& get_trust( ch ) < get_trust( victim ) )
	return FALSE;

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
	return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
	return FALSE;

    if ( room_is_dark( ch->in_room )
	&& !IS_SET( race_table[ ch->race ].race_abilities, RACE_INFRAVISION )
	&& !IS_AFFECTED( ch, AFF_INFRARED ) )
	return FALSE;

    if ( victim->position == POS_DEAD )
        return TRUE;

    if ( IS_AFFECTED( victim, AFF_INVISIBLE )
	&& !IS_SET( race_table[ ch->race ].race_abilities, RACE_DETECT_INVIS )
	&& !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
	return FALSE;

    if ( IS_AFFECTED( victim, AFF_HIDE )
	&& !IS_SET( race_table[ ch->race ].race_abilities, RACE_DETECT_HIDDEN )
	&& !IS_AFFECTED( ch, AFF_DETECT_HIDDEN )
	&& !victim->fighting )
	return FALSE;

    return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( obj->deleted )
        return FALSE;

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
	return TRUE;

    if ( xIS_SET( obj->extra_flags,ITEM_VIS_DEATH ) )
        return FALSE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( room_is_dark( ch->in_room )
	&& !IS_SET( race_table[ ch->race ].race_abilities, RACE_INFRAVISION )
	&& !IS_AFFECTED( ch, AFF_INFRARED ) )
	return FALSE;

    if ( xIS_SET( obj->extra_flags, ITEM_INVIS )
	&& !IS_SET( race_table[ ch->race ].race_abilities, RACE_DETECT_INVIS )
	&& !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !xIS_SET( obj->extra_flags, ITEM_NODROP ) )
	return TRUE;

    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;

    return FALSE;
}



/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA *obj )
{
    OBJ_DATA *in_obj;
    char      buf [ MAX_STRING_LENGTH ];

    switch ( obj->item_type )
    {
    case ITEM_LIGHT:		return "light";
    case ITEM_SCROLL:		return "scroll";
    case ITEM_WAND:		return "wand";
    case ITEM_STAFF:		return "staff";
    case ITEM_WEAPON:		return "weapon";
    case ITEM_TREASURE:		return "treasure";
    case ITEM_ARMOR:		return "armor";
    case ITEM_POTION:		return "potion";
    case ITEM_FURNITURE:	return "furniture";
    case ITEM_TRASH:		return "trash";
    case ITEM_CONTAINER:	return "container";
    case ITEM_DRINK_CON:	return "drink container";
    case ITEM_KEY:		return "key";
    case ITEM_FOOD:		return "food";
    case ITEM_MONEY:		return "money";
    case ITEM_BOAT:		return "boat";
    case ITEM_CORPSE_NPC:	return "npc corpse";
    case ITEM_CORPSE_PC:        return "pc corpse";
    case ITEM_FOUNTAIN:		return "fountain";
    case ITEM_PILL:		return "pill";
    case ITEM_PORTAL:		return "portal";
    case ITEM_WARP_STONE:	return "warp stone";
    case ITEM_CLOTHING:		return "clothing";
    case ITEM_RANGED_WEAPON:	return "ranged weapon";
    case ITEM_AMMO:		return "ammo";
    case ITEM_GEM:		return "gem";
    case ITEM_VEHICLE:	      	return "vehicle";
    }

    for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj )
      ;

    if ( in_obj->carried_by )
      sprintf( buf, "Item_type_name: unknown type %d from %s owned by %s.",
	      obj->item_type, obj->name, obj->carried_by->name );
    else
      sprintf( buf,
	      "Item_type_name: unknown type %d from %s owned by (unknown).",
	      obj->item_type, obj->name );

    bug( buf, 0 );
    return "(unknown)";
}



/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:		return "none";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_SEX:		return "sex";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_AGE:		return "age";
    case APPLY_HEIGHT:          return "height";
    case APPLY_WEIGHT:          return "weight";
    case APPLY_MANA:		return "mana";
    case APPLY_HIT:		return "hp";
    case APPLY_MOVE:		return "moves";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:		return "damage roll";
    case APPLY_SAVING_PARA:	return "save vs paralysis";
    case APPLY_SAVING_ROD:	return "save vs rod";
    case APPLY_SAVING_PETRI:	return "save vs petrification";
    case APPLY_SAVING_BREATH:	return "save vs breath";
    case APPLY_SAVING_SPELL:	return "save vs spell";
    case APPLY_RACE:		return "race";
    case APPLY_RESISTANT:	return "resistant";
    case APPLY_IMMUNE:		return "immune";
    case APPLY_SUSCEPTIBLE:	return "susceptible";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}



/*
 * Return ascii name of extra flags vector.
 */
const char *extra_bit_name( XBV vector )
{
    return act_xbv( extra_flags, vector );
}



/*
 * Return ascii name of parts vector.
 */
const char *parts_bit_name( int vector )
{
    static char buf [ 512 ];

    buf[0] = '\0';
    if ( vector & PART_HEAD         ) strcat( buf, " head"           );
    if ( vector & PART_ARMS         ) strcat( buf, " arms"           );
    if ( vector & PART_LEGS         ) strcat( buf, " legs"           );
    if ( vector & PART_HEART        ) strcat( buf, " heart"          );
    if ( vector & PART_BRAINS       ) strcat( buf, " brains"         );
    if ( vector & PART_GUTS         ) strcat( buf, " guts"           );
    if ( vector & PART_HANDS        ) strcat( buf, " hands"          );
    if ( vector & PART_FEET         ) strcat( buf, " feet"           );
    if ( vector & PART_FINGERS      ) strcat( buf, " fingers"        );
    if ( vector & PART_EAR          ) strcat( buf, " ear"            );
    if ( vector & PART_EYE          ) strcat( buf, " eye"            );
    if ( vector & PART_LONG_TONGUE  ) strcat( buf, " long_tongue"    );
    if ( vector & PART_EYESTALKS    ) strcat( buf, " eyestalks"      );
    if ( vector & PART_TENTACLES    ) strcat( buf, " tentacles"      );
    if ( vector & PART_FINS         ) strcat( buf, " fins"           );
    if ( vector & PART_WINGS        ) strcat( buf, " wings"          );
    if ( vector & PART_TAIL         ) strcat( buf, " tail"           );
    if ( vector & PART_CLAWS        ) strcat( buf, " claws"          );
    if ( vector & PART_FANGS        ) strcat( buf, " fangs"          );
    if ( vector & PART_HORNS        ) strcat( buf, " horns"          );
    if ( vector & PART_SCALES       ) strcat( buf, " scales"         );
    if ( vector & PART_TUSKS        ) strcat( buf, " tusks"          );

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}



/*
 * Return ascii name of a ris vector.
 */
const char *ris_bit_name( int vector )
{
    static char buf [ 512 ];

    buf[0] = '\0';
    if ( vector & RIS_FIRE          ) strcat( buf, " fire"           );
    if ( vector & RIS_COLD          ) strcat( buf, " cold"           );
    if ( vector & RIS_ELECTRICITY   ) strcat( buf, " electricity"    );
    if ( vector & RIS_ENERGY        ) strcat( buf, " energy"         );
    if ( vector & RIS_ACID          ) strcat( buf, " acid"           );
    if ( vector & RIS_POISON        ) strcat( buf, " poison"         );
    if ( vector & RIS_CHARM         ) strcat( buf, " charm"          );
    if ( vector & RIS_MENTAL        ) strcat( buf, " mental"         );
    if ( vector & RIS_WHITE_MANA    ) strcat( buf, " white_mana"     );
    if ( vector & RIS_BLACK_MANA    ) strcat( buf, " black_mana"     );
    if ( vector & RIS_DISEASE       ) strcat( buf, " disease"        );
    if ( vector & RIS_DROWNING      ) strcat( buf, " drowning"       );
    if ( vector & RIS_LIGHT         ) strcat( buf, " light"          );
    if ( vector & RIS_SOUND         ) strcat( buf, " sound"          );
    if ( vector & RIS_MAGIC         ) strcat( buf, " magic"          );
    if ( vector & RIS_NONMAGIC      ) strcat( buf, " nonmagic"       );
    if ( vector & RIS_SILVER        ) strcat( buf, " silver"         );
    if ( vector & RIS_IRON          ) strcat( buf, " iron"           );
    if ( vector & RIS_WOOD          ) strcat( buf, " wood"           );
    if ( vector & RIS_WEAPON        ) strcat( buf, " weapon"         );
    if ( vector & RIS_BASH          ) strcat( buf, " bash"           );
    if ( vector & RIS_PIERCE        ) strcat( buf, " pierce"         );
    if ( vector & RIS_SLASH         ) strcat( buf, " slash"          );

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}



CHAR_DATA *get_char( CHAR_DATA *ch )
{
    if ( !ch->pcdata )
        return ch->desc->original;
    else
        return ch;
}

bool longstring( CHAR_DATA *ch, char *argument )
{
    if ( strlen( argument) > 60 )
    {
	send_to_char( "No more than 60 characters in this field.\n\r", ch );
	return TRUE;
    }
    else
        return FALSE;
}

bool authorized( CHAR_DATA *ch, char *skllnm )
{

    char buf [ MAX_STRING_LENGTH ];

    if ( ( !IS_NPC( ch ) && str_infix( skllnm, ch->pcdata->immskll ) )
	||  IS_NPC( ch ) )
    {
        sprintf( buf, "Sorry, you are not authorized to use %s.\n\r", skllnm );
	send_to_char( buf, ch );
	return FALSE;
    }

    return TRUE;

}

void end_of_game( void )
{
    DESCRIPTOR_DATA *d;
    DESCRIPTOR_DATA *d_next;

    char buf [ MAX_STRING_LENGTH ];

    char *	const	message		[ 7 ] =
    {
	"{o{yYou feel the ground shake as the end comes near!{x\n\r",
	"{o{yLightning crackles in the sky above!{x\n\r",
	"{o{yCrashes of thunder sound across the land!{x\n\r",
	"{o{yThe sky has suddenly turned midnight black.{x\n\r",
	"{o{yYou notice the life forms around you slowly dwindling away.{x\n\r",
	"{o{yThe seas across the realm have turned frigid.{x\n\r",
	"{o{yYou sense a change in the magical forces surrounding you.{x\n\r"
    };

    if ( auction.item )
    {
	sprintf( buf, "Sale of %s has been stopped by mud.",
		auction.item->short_descr );
	talk_auction( buf );

	obj_to_char( auction.item, auction.seller );
	auction.item = NULL;

	if ( auction.buyer )
	{
	    auction.buyer->gold += auction.bet;
	    send_to_char( "Your money has been returned.\n\r", auction.buyer );
	}
    }

    send_to_all_char( message[ number_range( 0, 6 ) ] );

    for ( d = descriptor_list; d; d = d_next )
    {
	d_next = d->next;
	if ( d->connected == CON_PLAYING )
	{
	    if ( d->character->position == POS_FIGHTING )
	      interpret( d->character, "save" );
	    else
	      interpret( d->character, "quit" );
	}
	else
	    close_socket( d );
    }

    clan_update	( );

    return;

}

int race_lookup( const char *race )
{
    int index;

    for ( index = 0; index < MAX_RACE; index++ )
        if ( !str_prefix( race, race_table[index].name ) )
	    return index;

    return -1;

}

int race_full_lookup( const char *race )
{
    int index;

    for ( index = 0; index < MAX_RACE; index++ )
        if ( !str_cmp( race, race_table[index].name ) )
            return index;

    return NO_FLAG;

}

int affect_lookup( const char *affectname )
{
    int index;

    for ( index = 0; index < MAX_SKILL; index++ )
	if ( !str_cmp( affectname, skill_table[index].name ) )
	    return index;

    return -1;

}

/*
 * Lookup a clan by name.
 */
CLAN_DATA *clan_lookup( const char *name )
{
    CLAN_DATA *clan;

    for ( clan = clan_first ; clan; clan = clan->next )
    {
	if ( !clan->name )
	    break;
	if ( LOWER( name[0] ) == LOWER( clan->name[0] )
	    && !str_prefix( name, clan->name ) )
	    return clan;
    }

    return NULL;
}

/*
  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.
  
  Erwin S.A.
*/
int advatoi( const char *s )
{
    int number		= 0;
    int multiplier	= 0;

    /*
     * as long as the current character is a digit add to current number.
     */
    while ( isdigit( s[0] ) )
        number = ( number * 10 ) + ( *s++ - '0' );

    switch ( UPPER( s[0] ) )
    {
        case 'K'  : number *= ( multiplier = 1000 );      ++s; break;
        case 'M'  : number *= ( multiplier = 1000000 );   ++s; break;
        case '\0' : break;
        default   : return 0; /* not k nor m nor NULL - return 0! */
    }

    /* if any digits follow k/m, add those too */
    while ( isdigit( s[0] ) && ( multiplier > 1 ) )
    {
        /* the further we get to right, the less the digit 'worth' */
        multiplier /= 10;
        number = number + ( ( *s++ - '0' ) * multiplier );
    }

    /* return 0 if non-digit character was found, other than NULL */
    if ( s[0] != '\0' && !isdigit( s[0] ) )
        return 0;

    /* anything left is likely extra digits (ie: 14k4443  -> 3 is extra) */
    return number;
}

/*
  This function allows the following kinds of bets to be made:

  Absolute bet
  ============

  bet 14k, bet 50m66, bet 100k

  Relative bet
  ============

  These bets are calculated relative to the current bet. The '+' symbol adds
  a certain number of percent to the current bet. The default is 25, so
  with a current bet of 1000, bet + gives 1250, bet +50 gives 1500 etc.
  Please note that the number must follow exactly after the +, without any
  spaces!

  The '*' or 'x' bet multiplies the current bet by the number specified,
  defaulting to 2. If the current bet is 1000, bet x  gives 2000, bet x10
  gives 10,000 etc.
*/
int parsebet( const int currentbet, char *s )
{
    /* check to make sure it's not blank */
    if ( s[0] != '\0' )
    {
	/* if first char is a digit, use advatoi */
	if ( isdigit( s[0] ) )
	    return ( advatoi( s ) );
	if ( s[0] == '+' )		/* add percent (default 25%) */
	{
	    if ( s[1] == '\0' )
		return ( currentbet * 125 ) / 100;
	    return ( currentbet * ( 100 + atoi( s+1 ) ) ) / 100;
	}
	if ( s[0] == '*' || s[0] == 'x' ) /* multiply (default is by 2) */
	{
	    if ( s[1] == '\0' )
		return (currentbet * 2);
	    return ( currentbet * atoi( s+1 ) );
	}
    }

    return 0;
}

void learn( CHAR_DATA *ch, int sn, bool success )
{
    CLASS_TYPE *cclass;
    char        buf    [ MAX_STRING_LENGTH ];
    int         chance;
    int         gain;

    if ( IS_NPC( ch ) )
	return;

    if ( !( cclass = skill_class( ch, sn ) )
	|| ch->level < cclass->skill_level[sn]
	|| cclass->skill_rating[sn] == 0
	|| ch->pcdata->learned[sn] == 0
	|| ch->pcdata->learned[sn] >= cclass->skill_adept[sn] )
	return;

    chance  = int_app[get_curr_int( ch )].learn;
    chance += cclass->skill_rating[sn] * 3;
    chance += ch->level / 2;

    chance *= number_range( 2, 6 );

    if ( number_range( 1, 1000 ) > chance )
	return;

    if ( success )
    {
	chance = URANGE( 5, 100 - ch->pcdata->learned[sn], 95 );

	if ( number_percent( ) > chance )
	    return;

	ch->pcdata->learned[sn]++;

	gain = 2 * cclass->skill_rating[sn];

	if ( ch->pcdata->learned[sn] == cclass->skill_adept[sn] )
	{
	    gain *= 2;

	    sprintf( buf, "{oYou are now an adept of %s!\n\r",
	    	    skill_table[sn].name );
	    send_to_char( buf, ch );
	    sprintf( buf, "{oYou gain %d bonus experience!{x\n\r", gain );
	    send_to_char( buf, ch );

	    gain_exp( ch, gain );
	    return;
	}

	sprintf( buf, "{oYou have become better at %s!\n\r",
		skill_table[sn].name );
	send_to_char( buf, ch );
	sprintf( buf, "{oYou gain %d experience points from your sucess!{x\n\r",
		gain );
	send_to_char( buf, ch );

	gain_exp( ch, gain );
	return;
    }

    chance = URANGE( 5, ch->pcdata->learned[sn] / 2, 30 );

    if ( number_percent( ) > chance )
	return;

    sprintf( buf,
	    "You learn from your mistakes, and you improve at %s.\n\r",
	    skill_table[sn].name );
    send_to_char( buf, ch );

    ch->pcdata->learned[sn] += number_fuzzy( 2 );
    ch->pcdata->learned[sn]  = UMIN( ch->pcdata->learned[sn],
    				      cclass->skill_adept[sn] );
    return;
}



/*
 * Extended bitvector utility functions.
 */
bool xbv_is_empty( XBV *bits )
{
    register int i;

    for ( i = 0; i < XBI; i++ )
	if ( bits->bits[i] != 0 )	return FALSE;

    return TRUE;
}

bool xbv_same_bits( XBV *dest, const XBV *src )
{
    register int i;

    for ( i = 0; i < XBI; i++ )
	if ( dest->bits[i] != src->bits[i] )	return FALSE;

    return TRUE;
}

void xbv_clear_bits( XBV *bits )
{
    register int i;

    for ( i = 0; i < XBI; i++ )
	bits->bits[i] = 0;
}

void xbv_set_bits( XBV *dest, const XBV *src )
{
    register int i;

    for ( i = 0; i < XBI; i++ )
	SET_BIT( dest->bits[i], src->bits[i] );
}

void xbv_remove_bits( XBV *dest, const XBV *src )
{
    register int i;

    for ( i = 0; i < XBI; i++ )
	REMOVE_BIT( dest->bits[i], src->bits[i] );
}

XBV new_xbv (int bit, ...)
{
  static XBV	 bits;
  	 va_list param;
  	 int	 b;
  
  xCLEAR_BITS (bits);
  xSET_BIT (bits, bit);

  va_start (param, bit);

  while ((b=va_arg (param, int)) != -1)
    xSET_BIT( bits, b);
  va_end (param);

  return bits;
}


/*
 * Multiclass support functions.
 */
bool can_use( CHAR_DATA *ch, int sn )
{
    CLASS_TYPE *cclass;

    if ( IS_NPC( ch ) )
	return TRUE;

    if ( !( cclass = skill_class( ch, sn ) ) )
	return FALSE;
    
    return ( ch->level >= cclass->skill_level[sn] );
}


CLASS_TYPE *skill_class( CHAR_DATA *ch, int sn )
{
    int iClass;

    if ( IS_NPC( ch ) )
	return NULL;

    for ( iClass = 0; iClass < MAX_MULTICLASS && ch->cclass[iClass]; iClass++ )
    {
	if ( ch->cclass[iClass]->skill_level[sn] < L_APP )
	    return ch->cclass[iClass];
    }

    return NULL;
}


int skill_level( CHAR_DATA *ch, int sn )
{
    int iClass;
    int level;

    if ( IS_NPC( ch ) )
	return 0;

    level = 0;

    for ( iClass = 0; iClass < MAX_MULTICLASS && ch->cclass[iClass]; iClass++ )
	level += ch->cclass[iClass]->skill_level[sn];

    level /= iClass;

    return level;
}


bool can_prac( CHAR_DATA *ch, int sn )
{
    CLASS_TYPE *cclass;

    if ( IS_NPC( ch ) )
	return FALSE;

    if ( !( cclass = skill_class( ch, sn ) ) )
	return FALSE;
    
    return ( ch->level >= cclass->skill_level[sn] );
}


bool has_spells( CHAR_DATA *ch )
{
    int iClass;

    if ( IS_NPC( ch ) )
	return FALSE;

    for ( iClass = 0; iClass < MAX_MULTICLASS && ch->cclass[iClass]; iClass++ )
    {
	if ( ch->cclass[iClass]->fMana )
	    return TRUE;
    }

    return FALSE;
}


bool is_class( CHAR_DATA *ch, CLASS_TYPE *cclass )
{
    int iClass; 

    if ( IS_NPC( ch ) )
	return FALSE;

    for ( iClass = 0; iClass < MAX_MULTICLASS && ch->cclass[iClass]; iClass++ )
    {
	if ( ch->cclass[iClass] == cclass )
	    return TRUE;
    }

    return FALSE;
}


int number_classes( CHAR_DATA *ch )
{
    int iClass;

    if ( IS_NPC( ch ) )
	return 0;

    for ( iClass = 0; iClass < MAX_MULTICLASS && ch->cclass[iClass]; iClass++ )
	;

    return iClass;
}


char *class_long( CHAR_DATA *ch )
{
    static char buf [ MAX_STRING_LENGTH ];
           int  iClass;

    if ( IS_NPC( ch ) )
	return "Mobile";

    buf[0] = '\0';
    for ( iClass = 0; iClass < MAX_MULTICLASS && ch->cclass[iClass]; iClass++ )
    {
	strcat( buf, "/" );
	strcat( buf, ch->cclass[iClass]->name );
    }

    return buf+1;
}


char *class_short( CHAR_DATA *ch )
{
    static char buf [ MAX_STRING_LENGTH ];
    int         iClass;

    if ( IS_NPC( ch ) )
	return "Mob";

    buf[0] = '\0';
    for ( iClass = 0; iClass < MAX_MULTICLASS && ch->cclass[iClass]; iClass++ )
    {
	strcat( buf, "/" );
	strcat( buf, ch->cclass[iClass]->who_name );
    }

    return buf+1;
}


/*
 * Lookup a class by name.
 */
CLASS_TYPE *class_lookup( const char *name )
{
    CLASS_TYPE *cclass;

    for ( cclass = class_first; cclass; cclass = cclass->next )
    {
	if ( !cclass->name )
            break;
	if ( LOWER( name[0] ) == LOWER( cclass->name[0] )
            && !str_prefix( name, cclass->name ) )
            return cclass;
    }

    return NULL;
}
