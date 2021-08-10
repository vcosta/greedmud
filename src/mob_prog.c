/***************************************************************************
 *  GreedMud 0.99.3 improvements copyright (C) 1997, 1998, 1999            *
 *  by Vasco Costa.                                                        *
 *                                                                         *
 *  Based on MERC 2.2 MOBPrograms by N'Atas-ha.                            *
 *                                                                         *
 *  MOBPrograms for ROM2.4 v0.98g copyright (C) 1996 by Markku Nylander.   *
 *                                                                         *
 *  <markku.nylander@uta.fi>                                               *
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
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"



/*
 * These defines correspond to the entries in fn_keyword[] table.
 * If you add a new if_check, you must also add a #define here.
 */
enum
{ CHK_RAND=0,
  CHK_MOBHERE,
  CHK_OBJHERE,
  CHK_MOBEXISTS,
  CHK_OBJEXISTS,
  CHK_PEOPLE,
  CHK_PLAYERS,
  CHK_MOBS,
  CHK_CLONES,
  CHK_ORDER,
  CHK_HOUR,
  CHK_ISPC,
  CHK_ISNPC,
  CHK_ISGOOD,
  CHK_ISEVIL,
  CHK_ISNEUTRAL,
  CHK_ISIMMORT,
  CHK_ISCHARM,
  CHK_ISFOLLOW,
  CHK_ISACTIVE,
  CHK_ISDELAY,
  CHK_ISVISIBLE,
  CHK_HASTARGET,
  CHK_ISTARGET,
  CHK_EXISTS,
  CHK_AFFECTED,
  CHK_ACT,
  CHK_RES,
  CHK_IMM,
  CHK_SUS,
  CHK_CARRIES,
  CHK_WEARS,
  CHK_HAS,
  CHK_USES,
  CHK_NAME,
  CHK_POS,
  CHK_CLAN,
  CHK_RACE,
  CHK_CLASS,
  CHK_OBJTYPE,
  CHK_VNUM,
  CHK_HPCNT,
  CHK_ROOM,
  CHK_SEX,
  CHK_LEVEL,
  CHK_ALIGN,
  CHK_MONEY,
  CHK_OBJVAL0,
  CHK_OBJVAL1,
  CHK_OBJVAL2,
  CHK_OBJVAL3,
  CHK_OBJVAL4,
  CHK_GRPSIZE
}fv_check;


/*
 * These defines correspond to the entries in fn_evals[] table.
 */
enum
{ EVAL_EQ=0,
  EVAL_GE,
  EVAL_LE,
  EVAL_GT,
  EVAL_LT,
  EVAL_NE
}fv_eval;



/*
 * If-check keywords:
 */
const char	*fn_keyword	[] =
{
    "rand",	 /* if rand 30  	 - if random number < 30             */
    "mobhere",	 /* if mobhere fido	 - is there a 'fido' here            */
    "objhere",	 /* if objhere bottle	 - is there a 'bottle' here          */
		 /* if mobhere 1233	 - is there mob vnum 1233 here       */
		 /* if objhere 1233	 - is there obj vnum 1233 here       */
    "mobexists", /* if mobexists fido	 - is there a fido somewhere         */
    "objexists", /* if objexists sword   - is there a sword somewhere        */

    "people",	 /* if people > 4   - does room contain > 4 people           */
    "players",	 /* if players > 1  - does room contain > 1 pcs              */
    "mobs",	 /* if mobs > 2     - does room contain > 2 mobiles          */
    "clones",	 /* if clones > 3   - are there > 3 mobs of same vnum here   */
    "order",	 /* if order == 0   - is mob the first in room               */
    "hour",	 /* if hour > 11    - is the time > 11 o'clock               */


    "ispc",	 /* if ispc $n  	 - is $n a pc                        */
    "isnpc",	 /* if isnpc $n 	 - is $n a mobile                    */
    "isgood",	 /* if isgood $n	 - is $n good                        */
    "isevil",	 /* if isevil $n	 - is $n evil                        */
    "isneutral", /* if isneutral $n	 - is $n neutral                     */
    "isimmort",	 /* if isimmort $n	 - is $n immortal                    */
    "ischarm",	 /* if ischarm $n	 - is $n charmed                     */
    "isfollow",	 /* if isfollow $n	 - is $n following someone           */
    "isactive",	 /* if isactive $n	 - is $n's position > SLEEPING       */
    "isdelay",	 /* if isdelay $i	 - does $i have mobprog pending      */
    "isvisible", /* if isvisible $n	 - can mob see $n                    */
    "hastarget", /* if hastarget $i	 - does $i have a valid target       */
    "istarget",	 /* if istarget $n	 - is $n mob's target                */
    "exists",	 /* if exists $n	 - does $n exist somewhere           */

    "affected",	 /* if affected $n blind - is $n affected by blind           */
    "act",	 /* if act $i sentinel   - is $i flagged sentinel            */
    "res",       /* if res $i fire	 - is $i resistant to fire           */
    "imm",       /* if imm $i fire	 - is $i immune to fire              */
    "sus",       /* if sus $i fire	 - is $i susceptible to fire         */
    "carries",	 /* if carries $n sword  - does $n have a 'sword'            */
		 /* if carries $n 1233   - does $n have obj vnum 1233        */
    "wears",	 /* if wears $n lantern  - is $n wearing a 'lantern'         */
		 /* if wears $n 1233	 - is $n wearing obj vnum 1233       */
    "has",    	 /* if has $n weapon	 - does $n have obj of type weapon   */
    "uses",	 /* if uses $n armor	 - is $n wearing obj of type armor   */
    "name",	 /* if name $n puff	 - is $n's name 'puff'               */
    "pos",	 /* if pos $n standing   - is $n standing                    */
    "clan",	 /* if clan $n 'whatever'- does $n belong to clan 'whatever' */
    "race",	 /* if race $n dragon	 - is $n of 'dragon' race            */
    "class",	 /* if class $n mage	 - is $n's class 'mage'              */
    "objtype",	 /* if objtype $p scroll - is $p a scroll                    */

    "vnum",	 /* if vnum $i == 1233   - virtual number check              */
    "hpcnt",	 /* if hpcnt $i > 30	 - hit point percent check           */
    "room",	 /* if room $i == 1233   - room virtual number               */
    "sex",	 /* if sex $i == 0	 - sex check                         */
    "level",	 /* if level $n < 5	 - level check                       */
    "align",	 /* if align $n < -1000  - alignment check                   */
    "money",	 /* if money $n */
    "objval0",	 /* if objval0 > 1000	 - object value[] checks 0..4        */
    "objval1",
    "objval2",
    "objval3",
    "objval4",
    "grpsize",	 /* if grpsize $n > 6	 - group size check                  */

    ""
};

const char	*fn_evals	[] =
{
    "==",
    ">=",
    "<=",
    ">",
    "<",
    "!=",
    ""
};



/*
 * Return a valid keyword from a keyword table
 */
int keyword_lookup( const char **table, char *keyword )
{
    register int i;

    for ( i = 0; table[i][0] != '\0'; i++ )
	if( !str_cmp( table[i], keyword ) )	return( i );

    return -1;
}


/*
 * Perform numeric evaluation.
 * Called by cmd_eval()
 */
int num_eval( int lval, int oper, int rval )
{
    switch( oper )
    {
	case EVAL_EQ:
	     return ( lval == rval );
	case EVAL_GE:
	     return ( lval >= rval );
	case EVAL_LE:
	     return ( lval <= rval );
	case EVAL_NE:
	     return ( lval != rval );
	case EVAL_GT:
	     return ( lval > rval  );
	case EVAL_LT:
	     return ( lval < rval  );
	default:
	     bug( "num_eval: invalid oper", 0 );
	     return 0;
    }
}

/*
 * ---------------------------------------------------------------------
 * UTILITY FUNCTIONS USED BY CMD_EVAL()
 * ----------------------------------------------------------------------
 */

/*
 * Get a random PC in the room (for $r parameter)
 */
CHAR_DATA *get_random_char( CHAR_DATA *mob )
{
    CHAR_DATA *vch, *victim = NULL;
    int        now = 0, highest = 0;

    for( vch = mob->in_room->people; vch; vch = vch->next_in_room )
    {
	if ( mob != vch 
	    && !IS_NPC( vch ) 
	    && can_see( mob, vch )
	    && ( now = number_percent( ) ) > highest )
	{
	    victim  = vch;
	    highest = now;
	}
    }
    return victim;
}

/* 
 * How many other players / mobs are there in the room
 * iFlag: 0: all, 1: players, 2: mobiles 3: mobs w/ same vnum 4: same group
 */
int count_people_room( CHAR_DATA *mob, int iFlag )
{
    CHAR_DATA *vch;
    int count;
    for ( count = 0, vch = mob->in_room->people; vch; vch = vch->next_in_room )
	if ( mob != vch 
	&&   (iFlag == 0
	  || (iFlag == 1 && !IS_NPC( vch )) 
	  || (iFlag == 2 &&  IS_NPC( vch ))
	  || (iFlag == 3 &&  IS_NPC( mob ) && IS_NPC( vch )
	     && mob->pIndexData->vnum == vch->pIndexData->vnum )
	  || (iFlag == 4 && is_same_group( mob, vch )) )
	&& can_see( mob, vch ) )
	    count++;
    return ( count );
}

/*
 * Get the order of a mob in the room. Useful when several mobs in
 * a room have the same trigger and you want only the first of them
 * to act 
 */
int get_order( CHAR_DATA *ch )
{
    CHAR_DATA *vch;
    int        i;

    if ( !IS_NPC( ch ) )
	return 0;
    for ( i = 0, vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
	if ( vch == ch )
	    return i;
	if ( IS_NPC( vch ) 
	&&   vch->pIndexData->vnum == ch->pIndexData->vnum )
	    i++;
    }
    return 0;
}

/*
 * Check if ch has a given item or item type
 * vnum: item vnum or -1
 * item_type: item type or -1
 * fWear: TRUE: item must be worn, FALSE: don't care
 */
bool has_item( CHAR_DATA *ch, int vnum, int item_type, bool fWear )
{
    OBJ_DATA *obj;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
	if ( ( vnum < 0 || obj->pIndexData->vnum == vnum )
	&&   ( item_type < 0 || obj->pIndexData->item_type == item_type )
	&&   ( !fWear || obj->wear_loc != WEAR_NONE ) )
	    return TRUE;
    return FALSE;
}

/*
 * Check if there's a mob with given vnum in the room
 */
bool get_mob_vnum_room( CHAR_DATA *ch, int vnum )
{
    CHAR_DATA *mob;

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	if ( IS_NPC( mob ) && mob->pIndexData->vnum == vnum )	return TRUE;
    return FALSE;
}

/*
 * Check if there's an object with given vnum in the room
 */
bool get_obj_vnum_room( CHAR_DATA *ch, int vnum )
{
    OBJ_DATA *obj;

    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	if ( obj->pIndexData->vnum == vnum )	return TRUE;
    return FALSE;
}

/* ---------------------------------------------------------------------
 * CMD_EVAL
 * This monster evaluates an if/or/and statement
 * There are five kinds of statement:
 * 1) keyword and value (no $-code)	    if random 30
 * 2) keyword, comparison and value	    if people > 2
 * 3) keyword and actor		    	    if isnpc $n
 * 4) keyword, actor and value		    if carries $n sword
 * 5) keyword, actor, comparison and value  if level $n >= 10
 *
 *----------------------------------------------------------------------
 */
int cmd_eval( int vnum, char *line, int check, CHAR_DATA *mob, CHAR_DATA *ch,
			   const void *arg1, const void *arg2, CHAR_DATA *rch )
{
  CHAR_DATA *lc   = mob;			/* lval_char */
  CHAR_DATA *vch  = (CHAR_DATA *) arg2;
  OBJ_DATA  *obj1 = (OBJ_DATA *) arg1;
  OBJ_DATA  *obj2 = (OBJ_DATA *) arg2;
  OBJ_DATA  *lo   = NULL;			/* lval_obj  */

  char *original, buf [MAX_INPUT_LENGTH], code;
  int	lval = 0, oper = 0, rval = -1;

  original = line;
  line     = one_argument( line, buf );

  if ( buf[0] == '\0' || !mob || mob->deleted )
    return FALSE;

  /*
   * If this mobile has no target, let's assume our victim is the one
   */
  if ( !mob->mprog_target )
    mob->mprog_target = ch;

  switch ( check )
  {
  /*
   * Case 1: keyword and value
   */
  case CHK_MOBHERE:
    if ( is_number( buf ) )
        return get_mob_vnum_room( mob, atoi( buf ) );
    else
        return get_char_room( mob, buf ) ? TRUE : FALSE;

  case CHK_OBJHERE:
    if ( is_number( buf ) )
        return get_obj_vnum_room( mob, atoi( buf ) );
    else
        return get_obj_here( mob, buf ) ? TRUE : FALSE;

  case CHK_RAND:      return ( atoi( buf ) < number_percent( ) );
  case CHK_MOBEXISTS: return get_char_world( mob, buf ) ? TRUE : FALSE;
  case CHK_OBJEXISTS: return get_obj_world ( mob, buf ) ? TRUE : FALSE;

  /*
   * Case 2 begins here: we sneakily use rval to indicate need
   *			 for numeric eval...
   */
  case CHK_PEOPLE:    rval = count_people_room( mob, 0 );	      break;
  case CHK_PLAYERS:   rval = count_people_room( mob, 1 );	      break;
  case CHK_MOBS:      rval = count_people_room( mob, 2 );	      break;
  case CHK_CLONES:    rval = count_people_room( mob, 3 );	      break;
  case CHK_ORDER:     rval = get_order( mob );  		      break;
  case CHK_HOUR:      rval = time_info.hour;			      break;
  default:							      break;
  }

  /*
   * Case 2 continued: evaluate expression
   */
  if ( rval >= 0 )
  {
    if ( ( oper = keyword_lookup( fn_evals, buf ) ) < 0 )
    {
      bugf( "Cmd_eval: prog %d syntax error(2) '%s'", vnum, original );
      return FALSE;
    }
    one_argument( line, buf );
    lval = rval;
    rval = atoi( buf );
    return num_eval( lval, oper, rval );
  }

  /*
   * Case 3,4,5: Grab actors from $* codes
   */
  if ( buf[0] != '$' || buf[1] == '\0' )
  {
    bugf( "Cmd_eval: prog %d syntax error(3) '%s'", vnum, original );
    return FALSE;
  }
  else
    code = buf[1];

  switch( code )
  {
    case 'i': lc = mob;						break;
    case 't': lc = vch;						break;
    case 'n': lc = ch;						break;
    case 'r': lc = ( rch ? rch : get_random_char( mob ) );	break;
    case 'q': lc = mob->mprog_target;				break;
    case 'o': lo = obj1;					break;
    case 'p': lo = obj2;					break;
    default:
	bugf( "Cmd_eval: prog %d syntax error(4) '%s'", vnum, original );
	return FALSE;
  }

  /*
   * From now on, we need an actor, so if none was found, bail out
   */
  if ( !lc && !lo )
      return FALSE;

  /*
   * Case 3: Keyword, comparison and value
   */
  switch( check )
  {
  case CHK_ISPC:	return ( lc &&     !IS_NPC( lc ) );
  case CHK_ISNPC:	return ( lc &&      IS_NPC( lc ) );
  case CHK_ISGOOD:	return ( lc &&     IS_GOOD( lc ) );
  case CHK_ISEVIL:	return ( lc &&     IS_EVIL( lc ) );
  case CHK_ISNEUTRAL:	return ( lc &&  IS_NEUTRAL( lc ) );
  case CHK_ISIMMORT:	return ( lc && IS_IMMORTAL( lc ) );

  case CHK_ISCHARM:	return ( lc && IS_AFFECTED( lc, AFF_CHARM ) );
  case CHK_ISACTIVE:	return ( lc && lc->position > POS_SLEEPING  );
  case CHK_ISDELAY:	return ( lc && lc->mprog_delay > 0          );

  case CHK_ISFOLLOW:
    return ( lc && lc->master && lc->master->in_room == lc->in_room );

  case CHK_ISVISIBLE:
    switch ( code )
    {
    default:
    case 'i':
    case 'n':
    case 't':
    case 'r':
    case 'q': return ( lc && can_see( mob, lc ) );
    case 'o':
    case 'p': return ( lo && can_see_obj( mob, lo ) );
    }
  case CHK_HASTARGET:
    return ( lc && lc->mprog_target && lc->in_room == lc->mprog_target->in_room );
  case CHK_ISTARGET:
    return ( lc && mob->mprog_target == lc );
  }

  /* 
   * Case 4: Keyword, actor and value
   */
  line = one_argument( line, buf );
  switch ( check )
  {
  case CHK_CLASS:
    return ( lc && is_class( lc, class_lookup( buf ) ) );
  case CHK_AFFECTED:
    return ( lc && IS_AFFECTED( lc, flag_value( affect_flags, buf ) ) );
  case CHK_ACT:
    return ( lc && xIS_SET( lc->act, flag_value( act_flags, buf ) ) );
  case CHK_RES:
    return ( lc && CHECK_RES( lc, flag_value( ris_flags, buf ) ) );
  case CHK_IMM:
    return ( lc && CHECK_IMM( lc, flag_value( ris_flags, buf ) ) );
  case CHK_SUS:
    return ( lc && CHECK_SUS( lc, flag_value( ris_flags, buf ) ) );
  case CHK_CARRIES:
    if ( is_number( buf ) )
      return ( lc && has_item( lc, atoi( buf ), -1, FALSE ) );
    else
      return ( lc && get_obj_carry( lc, buf ) );
  case CHK_WEARS:
    if ( is_number( buf ) )
      return( lc && has_item( lc, atoi( buf ), -1, TRUE ) );
    else
      return( lc && get_obj_wear( lc, buf ) );
  case CHK_HAS:
    return ( lc && has_item( lc, -1, flag_value( type_flags, buf ), FALSE ) );
  case CHK_USES:
    return ( lc && has_item( lc, -1, flag_value( type_flags, buf ), TRUE  ) );
  case CHK_NAME:
    switch( code )
    {
    default:
    case 'i':
    case 'n':
    case 't':
    case 'r':
    case 'q': return ( lc && is_name( buf, lc->name ) );
    case 'o':
    case 'p': return ( lo && is_name( buf, lo->name ) );
    }
  case CHK_POS:
    return ( lc && lc->position == flag_value( position_flags, buf ) );
  case CHK_CLAN:
    return ( lc && is_clan( lc ) && lc->pcdata->clan == clan_lookup( buf ) );
  case CHK_RACE:
    return ( lc && lc->race == race_lookup( buf ) );
  case CHK_OBJTYPE:
    return ( lo && lo->item_type == flag_value( type_flags, buf ) );
  }

  /*
   * Case 5: Keyword, actor, comparison and value
   */
  if ( ( oper = keyword_lookup( fn_evals, buf ) ) < 0 )
  {
      bugf( "Cmd_eval: prog %d syntax error(5): '%s'", vnum, original );
      return FALSE;
  }
  one_argument( line, buf );
  rval = atoi( buf );

  switch ( check )
  {
  case CHK_VNUM:
    switch( code )
    {
    default :
    case 'i':
    case 'n':
    case 't':
    case 'r':
    case 'q': if ( lc && IS_NPC( lc ) ) lval = lc->pIndexData->vnum;	break;
    case 'o':
    case 'p': if ( lo ) lval = lo->pIndexData->vnum;			break;
    }
    break;
  case CHK_HPCNT: if ( lc ) lval = ( lc->hit * 100 / lc->max_hit );	break;

  case CHK_ROOM:  if ( lc && lc->in_room ) lval = lc->in_room->vnum;	break;

  case CHK_SEX:     if ( lc ) lval = lc->sex;				break;
  case CHK_LEVEL:   if ( lc ) lval = lc->level;				break;
  case CHK_ALIGN:   if ( lc ) lval = lc->alignment;			break;
  case CHK_MONEY:   if ( lc ) lval = lc->gold;				break;
  case CHK_GRPSIZE: if ( lc ) lval = count_people_room( lc, 4 );	break;

  case CHK_OBJVAL0: if ( lo ) lval = lo->value[0];			break;
  case CHK_OBJVAL1: if ( lo ) lval = lo->value[1];			break;
  case CHK_OBJVAL2: if ( lo ) lval = lo->value[2];			break;
  case CHK_OBJVAL3: if ( lo ) lval = lo->value[3];			break;
  case CHK_OBJVAL4: if ( lo ) lval = lo->value[4];			break;

  default:							 return FALSE;
  }
  return num_eval( lval, oper, rval );
}

/*
 * ------------------------------------------------------------------------
 * EXPAND_ARG
 * This is a hack of act() in comm.c. I've added some safety guards,
 * so that missing or invalid $-codes do not crash the server
 * ------------------------------------------------------------------------
 */
void expand_arg( char *buf, const char *format, CHAR_DATA *mob, CHAR_DATA *ch,
		const void *arg1, const void *arg2, CHAR_DATA *rch )
{
    static char *    const  he_she  [ ] = { "it",  "he",  "she" };
    static char *    const  him_her [ ] = { "it",  "him", "her" };
    static char *    const  his_her [ ] = { "its", "his", "her" };

                  const char *someone   = "someone";
                  const char *something = "something";
                  const char *someones  = "someone's";
 
           char             fname   [ MAX_INPUT_LENGTH ];

	   CHAR_DATA       *vch  = (CHAR_DATA *) arg2;
	   OBJ_DATA        *obj1 = (OBJ_DATA *) arg1;
	   OBJ_DATA        *obj2 = (OBJ_DATA *) arg2;

    const  char            *str;
    const  char            *i;
           char            *point;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( !format || format[0] == '\0' )
	return;

    if ( !mob || mob->deleted )
	return;

    point   = buf;
    str     = format;
    while ( *str != '\0' )
    {
	if ( *str != '$' )
	{
	    *point++ = *str++;
	    continue;
	}
	++str;

	switch ( *str )
	{
	default:  bug( "Expand_arg: bad code %d.", *str );
		  i = " <@@@> ";					break;
        case 'i':
		  one_argument( mob->name, fname );
		  i = fname;						break;
        case 'I': i = mob->short_descr;					break;
        case 'n': 
	    i = someone;
	    if ( ch && can_see( mob, ch ) )
	    {
		one_argument( ch->name, fname );
		i = capitalize( fname );
	    }								break;
	case 'N': i = PERS( ch, mob );					break;
	case 't': 
	    i = someone;
	    if ( vch && can_see( mob, vch ) )
	    {
		one_argument( vch->name, fname );
		i = capitalize( fname );
	    }								break;
	case 'T': 
	    i = vch ? PERS( vch, mob ) : "";				break;
	case 'r': 
	    if ( !rch )	rch = get_random_char( mob );
	    i = someone;
	    if ( rch && can_see( mob, rch ) )
	    {
		one_argument( rch->name, fname );
		i = capitalize( fname );
	    }								break;
	case 'R': 
	    if ( !rch )	rch = get_random_char( mob );
	    i = rch ? PERS( rch, mob ) : "";				break;
	case 'q':
	    i = someone;
	    if ( mob->mprog_target && can_see( mob, mob->mprog_target ) )
	    {
		one_argument( mob->mprog_target->name, fname );
		i = capitalize( fname );
	    }								break;
	case 'Q':
	    i = ( mob->mprog_target ) ? PERS( mob->mprog_target, mob ) : "";
									break;
	case 'j': i = he_she[mob->sex];					break;
	case 'e': 
	    i = (  ch && can_see( mob,  ch ) ) ? he_she[ch->sex ] : someone;
									break;
	case 'E': 
	    i = ( vch && can_see( mob, vch ) ) ? he_she[vch->sex] : someone;
									break;
	case 'J': 
	    i = ( rch && can_see( mob, rch ) ) ? he_she[rch->sex] : someone;
									break;
	case 'X':
	    i = ( mob->mprog_target && can_see( mob, mob->mprog_target ) )
		? he_she[mob->mprog_target->sex] : someone;		break;
	case 'k': i = him_her[mob->sex];				break;
	case 'm':
	    i = (  ch && can_see( mob,  ch ) ) ? him_her[ch->sex ] : someone;
									break;
	case 'M':
	    i = ( vch && can_see( mob, vch ) ) ? him_her[vch->sex] : someone;
									break;
	case 'K': 
	    if ( rch )	rch = get_random_char( mob );
	    i = ( rch && can_see( mob, rch ) ) ? him_her[rch->sex] : someone;
									break;
	case 'Y': 
	    i = ( mob->mprog_target && can_see( mob, mob->mprog_target ) )
		? him_her[mob->mprog_target->sex] : someone;
									break;
	case 'l': i = his_her[mob->sex];				break;
	case 's': 
	    i = (  ch && can_see( mob,  ch ) ) ? his_her[ch->sex ] : someones;
									break;
	case 'S': 
	    i = ( vch && can_see( mob, vch ) ) ? his_her[vch->sex] : someones;
									break;
	case 'L': 
	    if ( !rch )	rch = get_random_char( mob );
	    i = ( rch && can_see( mob, rch ) ) ? his_her[rch->sex] : someones;
									break;
	case 'Z': 
	    i = ( mob->mprog_target && can_see( mob, mob->mprog_target ) )
		? his_her[mob->mprog_target->sex] : someones;
									break;
	case 'o':
	    i = something;
	    if ( obj1 && can_see_obj( mob, obj1 ) )
	    {
		one_argument( obj1->name, fname );
		i = fname;
	    }								break;
	case 'O':
	    i = ( obj1 && can_see_obj( mob, obj1 ) ) ? obj1->short_descr
		: something;						break;
	case 'p':
	    i = something;
	    if ( obj2 && can_see_obj( mob, obj2 ) )
	    {
		one_argument( obj2->name, fname );
		i = fname;
	    }								break;
	case 'P':
	    i = ( obj2 && can_see_obj( mob, obj2 ) ) ? obj2->short_descr
		: something;						break;
	}
 
	++str;
	while ( ( *point = *i ) != '\0' )
	    ++point, ++i;
 
    }
    *point = '\0';
    return;
}    

/*
 * ------------------------------------------------------------------------
 *  PROGRAM_FLOW
 *  This is the program driver. It parses the mob program code lines
 *  and passes "executable" commands to interpret()
 *  Lines beginning with 'mob' are passed to mob_interpret() to handle
 *  special mob commands (in mob_cmds.c)
 *-------------------------------------------------------------------------
 */

#define MAX_NESTED_LEVEL 12 /* Maximum nested if-else-endif's (stack size) */
#define BEGIN_BLOCK       0 /* Flag: Begin of if-else-endif block          */
#define IN_BLOCK         -1 /* Flag: Executable statements                 */
#define END_BLOCK        -2 /* Flag: End of if-else-endif block            */
#define MAX_CALL_LEVEL    5 /* Maximum nested calls                        */

void program_flow( int pvnum,  /* for diagnostic purposes */
		   char *source,  /* the actual MOBprog code */
	CHAR_DATA *mob, CHAR_DATA *ch, const void *arg1, const void *arg2 )
{
  CHAR_DATA *rch	      = NULL;
  char      *code, *line;
  char       data	      [ MAX_STRING_LENGTH ];
  char       buf	      [ MAX_STRING_LENGTH ];
  char       control	      [ MAX_INPUT_LENGTH  ];

  static int call_level; /* Keep track of nested "mpcall"s */

  int level, eval, check;
  int state[MAX_NESTED_LEVEL], /* Block state (BEGIN,IN,END) */
      cond [MAX_NESTED_LEVEL]; /* Boolean value based on the last if-check */

  int mvnum = mob->pIndexData->vnum;

  if ( ++call_level > MAX_CALL_LEVEL )
  {
    bug( "MOBprogs: MAX_CALL_LEVEL exceeded, vnum %d", mob->pIndexData->vnum );
    return;
  }

  /*
   * Reset "stack"
   */
  for ( level = 0; level < MAX_NESTED_LEVEL; level++ )
  {
    state[level] = IN_BLOCK;
    cond [level] = TRUE;
  }
  level = 0;
  code  = source;

  /*
   * Parse the MOBprog code
   */
  while ( *code )
  {
    bool first_arg = TRUE;
    char *b = buf, *c = control, *d = data;

    /*
     * Get a command line. We sneakily get both the control word
     * (if/and/or) and the rest of the line in one pass.
     */
    while( isspace( *code ) && *code ) code++;

    while ( *code )
    {
      if ( *code == '\n' || *code == '\r' )
        break;
      else if ( isspace(*code) )
      {
        if ( first_arg )
          first_arg = FALSE;
        else
          *d++ = *code;
      }
      else
      {
        if ( first_arg )
         *c++ = *code;
        else
         *d++ = *code;
      }
      *b++ = *code++;
    }
    *b = *c = *d = '\0';

    if ( buf[0] == '\0' )	break;
    if ( buf[0] == '*'  )	continue;

    line = data;

    /* 
     * Match control words
     */
    if ( !str_cmp( control, "if" ) )
    {
      if ( state[level] == BEGIN_BLOCK )
      {
        bugf( "Mobprog: misplaced if statement, mob %d prog %d", mvnum, pvnum );
        return;
      }
      state[level] = BEGIN_BLOCK;
      if ( ++level >= MAX_NESTED_LEVEL )
      {
        bugf( "Mobprog: Max nested level exceeded, mob %d prog %d", mvnum, pvnum );
        return;
      }
      if ( level && !cond[level-1] ) 
      {
        cond[level] = FALSE;
        continue;
      }
      line = one_argument( line, control );
      if ( ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
      {
        cond[level] = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
      }
      else
      {
        bugf( "Mobprog: invalid if_check (if), mob %d prog %d", mvnum, pvnum );
        return;
      }
      state[level] = END_BLOCK;
    }
    else if ( !str_cmp( control, "or" ) )
    {
      if ( !level || state[level-1] != BEGIN_BLOCK )
      {
        bugf( "Mobprog: or without if, mob %d prog %d", mvnum, pvnum );
        return;
      }
      if ( level && !cond[level-1] ) continue;
      line = one_argument( line, control );
      if ( ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
      {
        eval = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
      }
      else
      {
        bugf( "Mobprog: invalid if_check (or), mob %d prog %d", mvnum, pvnum );
        return;
      }
      cond[level] = eval ? TRUE : cond[level];
    }
    else if ( !str_cmp( control, "and" ) )
    {
      if ( !level || state[level-1] != BEGIN_BLOCK )
      {
        bugf( "Mobprog: and without if, mob %d prog %d", mvnum, pvnum );
        return;
      }
      if ( level && !cond[level-1] ) continue;
      line = one_argument( line, control );
      if ( ( check = keyword_lookup( fn_keyword, control ) ) >= 0 )
      {
        eval = cmd_eval( pvnum, line, check, mob, ch, arg1, arg2, rch );
      }
      else
      {
        bugf( "Mobprog: invalid if_check (and), mob %d prog %d", mvnum, pvnum );
        return;
      }
      cond[level] = cond[level] && eval;
    }
    else if ( !str_cmp( control, "endif" ) )
    {
      if ( !level || state[level-1] != BEGIN_BLOCK )
      {
        bugf( "Mobprog: endif without if, mob %d prog %d", mvnum, pvnum );
        return;
      }
      cond [  level] = TRUE;
      state[  level] = IN_BLOCK;
      state[--level] = END_BLOCK;
    }
    else if ( !str_cmp( control, "else" ) )
    {
      if ( !level || state[level-1] != BEGIN_BLOCK )
      {
        bugf( "Mobprog: else without if, mob %d prog %d", mvnum, pvnum );
        return;
      }
      if ( level && cond[level-1] == FALSE ) continue;
      state[level] = IN_BLOCK;
      cond [level] = (cond[level] == TRUE) ? FALSE : TRUE;
    }
    else if ( cond[level]
    && ( !str_cmp( control, "break" ) || !str_cmp( control, "end" ) ) )
    {
      call_level--;
      return;
    }
    else if ( (!level || cond[level]) && buf[0] != '\0' )
    {
      state[level] = IN_BLOCK;
      expand_arg( data, buf, mob, ch, arg1, arg2, rch );
      if ( !str_cmp( control, "mob" ) )
      {
        /* 
         * Found a mob restricted command, pass it to mob interpreter
         */
        line = one_argument( data, control );
        mob_interpret( mob, line );
      }
      else
      {
        /* 
         * Found a normal mud command, pass it to interpreter
         */
        interpret( mob, data );
      }
    }
  }
  call_level--;
}

/* 
 * ---------------------------------------------------------------------
 * Trigger handlers. These are called from various parts of the code
 * when an event is triggered.
 * ---------------------------------------------------------------------
 */

/*
 * A general purpose string trigger. Matches argument to a string trigger
 * phrase.
 */
void mp_act_trigger( char *argument, CHAR_DATA *mob, CHAR_DATA *ch, 
				const void *arg1, const void *arg2, int type )
{
    MPROG_LIST *prg;

    for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    {
    	if ( prg->trig_type == type && strstr( argument, prg->trig_phrase ) )
        {
	    program_flow( prg->vnum, prg->code, mob, ch, arg1, arg2 );
	    break;
	}
    }
    return;
}

/*
 * A general purpose percentage trigger. Checks if a random percentage
 * number is less than trigger phrase
 */
bool mp_raw_percent_trigger( CHAR_DATA *mob, CHAR_DATA *ch, 
				const void *arg1, const void *arg2, int type )
{
    MPROG_LIST *prg;

    for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    {
    	if ( prg->trig_type == type 
	    && number_percent( ) <= atoi( prg->trig_phrase ) )
        {
	    program_flow( prg->vnum, prg->code, mob, ch, arg1, arg2 );
	    return TRUE;
	}
    }
    return FALSE;
}

bool mp_percent_trigger( CHAR_DATA *mob, CHAR_DATA *ch, 
				const void *arg1, const void *arg2, int type )
{
    if (!mob->fighting)
	return mp_raw_percent_trigger( mob, ch, arg1, arg2, type );
    else
	return FALSE;
}

void mp_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount )
{
    MPROG_LIST *prg;

    /*
     * Original MERC 2.2 MOBprograms used to create a money object
     * and give it to the mobile. WFT was that? Funcs in act_obj()
     * handle it just fine.
     */
    for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
    {
	if ( prg->trig_type == TRIG_BRIBE
	    && amount >= atoi( prg->trig_phrase ) )
	{
	    program_flow( prg->vnum, prg->code, mob, ch, NULL, NULL );
	    break;
	}
    }
    return;
}

bool mp_exit_trigger( CHAR_DATA *ch, int dir )
{
    MPROG_LIST *prg;
    CHAR_DATA  *mob;

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {    
	if ( !IS_NPC( mob )
	    || ( !HAS_TRIGGER( mob, TRIG_EXIT ) && !HAS_TRIGGER( mob, TRIG_EXALL ) ) )
	    continue;

	for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
	{
	    /*
	     * Exit trigger works only if the mobile is not busy
	     * (fighting etc.). If you want to be sure all players
	     * are caught, use ExAll trigger
	     */
	    if ( prg->trig_type == TRIG_EXIT
	    	&& dir == atoi( prg->trig_phrase )
	    	&& can_see( mob, ch ) )
	    {
		program_flow( prg->vnum, prg->code, mob, ch, NULL, NULL );
		return TRUE;
	    }
	    else
	    if ( prg->trig_type == TRIG_EXALL
		&& dir == atoi( prg->trig_phrase ) )
	    {
		program_flow( prg->vnum, prg->code, mob, ch, NULL, NULL );
		return TRUE;
	    }
	}
    }
    return FALSE;
}

void mp_give_trigger( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj )
{

    char         buf	[MAX_INPUT_LENGTH], *p;
    MPROG_LIST  *prg;

    for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
      if ( prg->trig_type == TRIG_GIVE )
      {
        p = prg->trig_phrase;

        /*
         * Vnum argument
         */
        if ( is_number( p ) )
        {
          if ( obj->pIndexData->vnum == atoi( p ) )
          {
            program_flow( prg->vnum, prg->code, mob, ch, (void *)obj, NULL );
            return;
          }
        }

        /*
         * Object name argument, e.g. 'sword'
         */
        else
        {
          while ( *p )
          {
            p = one_argument( p, buf );

            if ( is_name( buf, obj->name ) || !str_cmp( "all", buf ) )
            {
              program_flow( prg->vnum, prg->code, mob, ch, (void *)obj, NULL );
              return;
            }
          }
        }
      }
    return;
}

void mp_greet_trigger( CHAR_DATA *ch )
{
    CHAR_DATA *mob;

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {    
	if ( IS_NPC( mob )
	    && ( HAS_TRIGGER( mob, TRIG_GREET ) || HAS_TRIGGER( mob, TRIG_GRALL ) ) )
	{
	    if ( HAS_TRIGGER( mob, TRIG_GREET ) && can_see( mob, ch ) )
		mp_percent_trigger( mob, ch, NULL, NULL, TRIG_GREET );
	    else                 
	    if ( HAS_TRIGGER( mob, TRIG_GRALL ) )
		mp_percent_trigger( mob, ch, NULL, NULL, TRIG_GRALL );
	}
    }
    return;
}

void mp_hprct_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{
    MPROG_LIST *prg;

    for ( prg = mob->pIndexData->mprogs; prg; prg = prg->next )
	if ( ( prg->trig_type == TRIG_HPCNT )
	&& ( ( 100 * mob->hit / mob->max_hit ) < atoi( prg->trig_phrase ) ) )
	{
	    program_flow( prg->vnum, prg->code, mob, ch, NULL, NULL );
	    break;
	}
    return;
}
