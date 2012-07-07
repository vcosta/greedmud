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
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



const char *HELP_STRING =
"{o{cSEdit help (commands on blank line):{x\n\r"
"{o{c-----------------------------------------------------------------------{x\n\r"
"{o{y.r {w'{yold{w' '{ynew{w' {xReplace a substring (requires '', \"\").{x\n\r"
"{o{y.h             {xGet help (this info).{x\n\r"
"{o{y.s             {xShow string so far.{x\n\r"
"{o{y.f             {xWord wrap string.{x\n\r"
"{o{y.c             {xClear string so far.{x\n\r"
"{o{y.d             {xDelete last line.{x\n\r"
"{o{y.d LINE        {xDelete line LINE.{x\n\r"
"{o{y@              {xEnd string.{x\n\r"
"{o{c-----------------------------------------------------------------------{x\n\r";


void string_edit( CHAR_DATA *ch, char **ps )
{
    send_to_char( "{o{cBegin entering your text now (.h = help .s = show .c = clear @ = save){x\n\r", ch );
    send_to_char( "{o{c-----------------------------------------------------------------------{x\n\r", ch );

    if ( !*ps )
        *ps = str_dup( "" );
    else
        **ps = '\0';

    ch->desc->str_editing = ps;
    return;
}


void string_append( CHAR_DATA *ch, char **ps )
{
    send_to_char( "{o{cBegin entering your text now (.h = help .s = show .c = clear @ = save){x\n\r", ch );
    send_to_char( "{o{c-----------------------------------------------------------------------{x\n\r", ch );

    if ( !*ps )
        *ps = str_dup( "" );
    send_to_char( *ps, ch );
    
    if ( *(*ps + strlen( *ps ) - 1) != '\r' )
    send_to_char( "\n\r", ch );

    ch->desc->str_editing = ps;

    return;
}


char *string_replace( char * orig_s, char * old_s, char * new_s )
{
    char xbuf [ MAX_STRING_LENGTH ];
    int i;

    xbuf[0] = '\0';
    strcpy( xbuf, orig_s );
    if ( strstr( orig_s, old_s ) )
    {
        i = strlen( orig_s ) - strlen( strstr( orig_s, old_s ) );
        xbuf[i] = '\0';
        strcat( xbuf, new_s );
        strcat( xbuf, &orig_s[i+strlen( old_s )] );
        free_string( orig_s );
    }

    return str_dup( xbuf );
}


char *one_line(char *s)
{
  while (*s && *s != '\n')
    s++;
  return s;
}


/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add( CHAR_DATA *ch, char *argument )
{
  char   buf [ MAX_STRING_LENGTH ];
  char **ps;

  ps=ch->desc->str_editing;

  /* Thanks to James Seng. */
  smash_tilde(argument);

  if ( *argument == '.' )
  {
    char  arg1 [ MAX_INPUT_LENGTH ];
    char  arg2 [ MAX_INPUT_LENGTH ];
    char  arg3 [ MAX_INPUT_LENGTH ];

    char *b;	/* beggining	*/
    char *e;	/* end		*/
    int   n;	/* current line	*/

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if ( !str_cmp( arg1, ".d" ) )
    {
      int x;

      if ( arg2[0] == '\0' )
      {
	/* Delete last line from string. */
	for (e=*ps, b=e; *e; )
	{
	  b=e;
	  if ( *(e=one_line(b)) )
	    e+=2;			/* skip "\n\r" */
	}

	buf[0]='\0';
	strncat(buf, *ps, b-*ps);

	free_string(*ps);
	*ps=str_dup(buf);

	send_to_char("SEdit: Last line of string deleted.\n\r", ch);
	return;
      }

      x=UMAX(0, atoi(arg2));

      /* Copy the lines before 'x'. */
      for ( b=*ps, n=0; n<x; n++ )
      {
	if ( *(b=one_line(b)) )
	  b+=2;
	else
	  return;
      }

      buf[0]='\0';
      strncat(buf, *ps, b-*ps);

      /* Copy the lines after 'x'. */
      if ( *(e=one_line(b)) )
	strcat(buf, e+2);

      free_string(*ps);
      *ps=str_dup(buf);
      return;
    }

    if ( !str_cmp( arg1, ".c" ) )
    {
      **ps='\0';
      send_to_char("SEdit: String cleared.\n\r", ch);
      return;
    }

    if ( !str_cmp( arg1, ".s" ) )
    {
      send_to_char(
	"{o{cString so far:{x\n\r"
	"{o{c-----------------------------------------------------------------------{x\n\r", ch);

      buf[0] = '\0';

      for ( b=*ps, n=0; *b; b=e, n++ )
      {
        e=one_line(b);
	if (*e)
	  e+=2;

        sprintf(buf+strlen(buf), "{o{c%2d>{x ", n);
	strncat(buf, b, e-b);
      }

      send_to_char(buf, ch);

      send_to_char(
        "{o{c-----------------------------------------------------------------------{x\n\r", ch);
      return;
    }

    if ( !str_cmp( arg1, ".r" ) )
    {
      if ( arg2[0] == '\0' )
      {
        send_to_char("usage:  .r \"old string\" \"new string\"\n\r", ch);
        return;
      }

      *ps=string_replace(*ps, arg2, arg3);
      sprintf(buf, "SEdit: '%s' replaced with '%s'.\n\r", arg2, arg3);
      send_to_char(buf, ch);
      return;
    }

    if ( !str_cmp( arg1, ".f" ) )
    {
      *ps=format_string(*ps);
      send_to_char("SEdit: String formatted.\n\r", ch);
      return;
    }
    
    if ( !str_cmp( arg1, ".h" ) )
    {
      send_to_char( HELP_STRING, ch );
      return;
    }
        
    send_to_char( "SEdit:  Invalid dot command.\n\r", ch );
    return;
  }

  if ( *argument == '@' )
  {
    /* Force character out of editing mode. */
    ch->desc->str_editing = NULL;
    return;
  }

  /*
   * Truncate strings to MAX_STRING_LENGTH.
   */
  if ( strlen(*ps) + strlen(argument) + 3 >= MAX_STRING_LENGTH )
  {
    send_to_char("String too long, last line skipped.\n\r", ch);

    /* Force character out of editing mode. */
    ch->desc->str_editing = NULL;
    return;
  }

  strcpy(buf, *ps);
  strcat(buf, argument);
  strcat(buf, "\n\r");

  free_string(*ps);
  *ps=str_dup(buf);
  return;
}



/*
 *  Thanks to Kalgen for the new procedure (no more bug!)
 *  Original wordwrap() written by Surreality.
 */
char *format_string( char *oldstring /*, bool fSpace */)
{
  char *rdesc;
  char  xbuf	[ MAX_STRING_LENGTH ];
  char  xbuf2	[ MAX_STRING_LENGTH ];
  int   i	= 0;
  bool  cap	= TRUE;
  
  xbuf[0] = xbuf2[0] = '\0';
  
  i = 0;
  
  if ( strlen( oldstring ) + 3 >= MAX_STRING_LENGTH )
  {
     bug( "String to format_string() longer than MAX_STRING_LENGTH.", 0 );
     return ( oldstring );
  }

  for ( rdesc = oldstring; *rdesc; rdesc++ )
  {
    if ( *rdesc == '\n' )
    {
      if ( xbuf[i-1] != ' ' )
      {
        xbuf[i] = ' ';
        i++;
      }
    }
    else if ( *rdesc == '\r' ) ;
    else if ( *rdesc == ' ' )
    {
      if ( xbuf[i-1] != ' ' )
      {
        xbuf[i] = ' ';
        i++;
      }
    }
    else if ( *rdesc == ')' )
    {
      if ( xbuf[i-1] == ' ' && xbuf[i-2] == ' ' && 
          ( xbuf[i-3] == '.' || xbuf[i-3] == '?' || xbuf[i-3] == '!' ) )
      {
        xbuf[i-2] = *rdesc;
        xbuf[i-1] = ' ';
        xbuf[i] = ' ';
        i++;
      }
      else
      {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else
        {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else
      {
        xbuf[ i ] = *rdesc;
        if (*( rdesc + 1 ) != '\"' )
        {
          xbuf[ i+1 ] = ' ';
          xbuf[ i+2 ] = ' ';
          i += 3;
        }
        else
        {
          xbuf[ i+1 ] = '\"';
          xbuf[ i+2 ] = ' ';
          xbuf[ i+3 ] = ' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else
    {
      xbuf[i] = *rdesc;
      if ( cap )
        {
          cap = FALSE;
          xbuf[i] = UPPER( xbuf[i] );
        }
      i++;
    }
  }
  xbuf[i] = 0;
  strcpy( xbuf2, xbuf );
  
  rdesc = xbuf2;
  
  xbuf[0] = 0;
  
  for ( ; ; )
  {
    for (i = 0; i < 77; i++ )
    {
      if ( !*( rdesc + i ) ) break;
    }
    if ( i < 77 )
    {
      break;
    }
    for ( i = ( xbuf[0] ? 76: 73 ) ; i ; i-- )
      if (*( rdesc + i ) == ' ') break;

    if ( i )
    {
      *( rdesc + i ) = 0;
      strcat( xbuf, rdesc );
      strcat( xbuf, "\n\r" );
      rdesc += i + 1;
      while ( *rdesc == ' ' ) rdesc++;
    }
    else
    {
      bug ( "No spaces", 0 );
      *( rdesc + 75 ) = 0;
      strcat( xbuf, rdesc );
      strcat( xbuf, "-\n\r" );
      rdesc += 76;
    }
  }
  while (*( rdesc + i ) && (*( rdesc + i ) == ' '||
                        *( rdesc + i ) == '\n'||
                        *( rdesc + i ) == '\r' ) )
    i--;
  *( rdesc + i + 1 ) = 0;
  strcat( xbuf, rdesc );
  if ( xbuf[ strlen( xbuf ) - 2 ] != '\n' )
    strcat( xbuf, "\n\r" );

  free_string( oldstring );
  return( str_dup( xbuf ) );
}



char * string_unpad( char * argument )
{
    char buf[MAX_STRING_LENGTH];
    char *s;

    s = argument;

    while ( *s == ' ' )
        s++;

    strcpy( buf, s );
    s = buf;

    if ( *s != '\0' )
    {
        while ( *s != '\0' )
            s++;
        s--;

        while( *s == ' ' )
            s--;
        s++;
        *s = '\0';
    }

    free_string( argument );
    return str_dup( buf );
}



/*
 * Same as capitalize but changes the pointer's data.
 */
char * string_proper( char * argument )
{
    char *s;

    s = argument;

    while ( *s != '\0' )
    {
        if ( *s != ' ' )
        {
            *s = UPPER(*s);
            while ( *s != ' ' && *s != '\0' )
                s++;
        }
        else
            s++;
    }

    return argument;
}
