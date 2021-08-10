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
 *  GreedMud 0.99.11 improvements copyright (C) 1997-2001 by Vasco Costa.  *
 *                                                                         *
 *  In order to use any part of this Envy Diku Mud, you must comply with   *
 *  the original Diku license in 'license.doc', the Merc license in        *
 *  'license.txt', as well as the Envy license in 'license.nvy'.           *
 *  In particular, you may not remove either of these copyright notices.   *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#ifdef _WIN32
char version_str [] = "$VER: GreedMud 0.99.11 Windows 32 Bit Version";
#else
#ifdef AmigaTCP 
char version_str [] = "$VER: GreedMud 0.99.11 Amiga Version";
#else
char version_str [] = "$VER: GreedMud 0.99.11 *NIX";
#endif
#endif

#include <sys/types.h>
#if defined( _MSC_VER )
#include <sys/timeb.h> /*for _ftime(), uses _timeb struct*/
#else
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>



/*
 * Signal handling.
 */
#include <signal.h>

#include "merc.h"


/*
 * Socket and TCP/IP stuff.
 */
#define IDENT_PORT		113

#if	defined( _MSC_VER )
#include <winsock.h>
#include <fcntl.h>
const	char   echo_off_str    [] = { '\xFF', '\xFB', '\x01', '\0' };
const	char   echo_on_str     [] = { '\xFF', '\xFC', '\x01', '\0' };
const	char   go_ahead_str    [] = { '\xFF', '\xF9', '\0' };
#else
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const	char   echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char   echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char   go_ahead_str    [] = { IAC, GA, '\0' };
#endif



/*
 * System dependent macros.
 */
#if	defined( _MSC_VER )
#define CLOSE			closesocket
#define READ( s, b, l )		recv( s, b, l, 0 )
#define WRITE( s, b, l )	send( s, b, l, 0 )
#else
#define CLOSE			close
#define READ			read
#define WRITE			write
#endif


/*
 * OS-dependent declarations (this will break on C++).
 */
#if	defined( _MSC_VER )
int	gettimeofday    args( ( struct timeval *tp, void *tzp ) );
#endif




/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_free;	/* Free list for descriptors	*/
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    merc_down;		/* Shutdown                     */
bool		    wizlock;		/* Game is wizlocked		*/
int                 numlock = 0;        /* Game is numlocked at <level> */
char		    str_boot_time [ MAX_INPUT_LENGTH ];
time_t		    current_time;	/* Time of this pulse		*/
int		    num_descriptors;
bool                MOBtrigger = TRUE;


/*
 * OS-dependent local functions.
 */
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( u_short port ) );
void	new_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
int	write_to_descriptor	args( ( int desc, char *txt, int length ) );
char *	get_ident		args( ( int desc ) );




/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				       bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( DESCRIPTOR_DATA *d ) );


int main( int argc, char **argv )
{
    struct  timeval now_time;
            u_short port;
            int     control;

    /*
     * Memory debugging if needed.
     */
    num_descriptors		= 0;

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Reserve one channel for our use.
     */
    if ( !( fpReserve = fopen( NULL_FILE, "r" ) ) )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 5005;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}
    }

    /*
     * Run the game.
     */
    control = init_socket( port );
    boot_db( );
    logln( "GreedMud is ready to rock on port %d.", port );
    game_loop_unix( control );
    CLOSE( control );
#if defined( _MSC_VER )
    WSACleanup( );
#endif

    /*
     * That's all, folks.
     */
    logln( "Normal termination of game." );
    exit( 0 );
    return 0;
}



int init_socket( u_short port )
{
    static struct sockaddr_in sa_zero;
           struct sockaddr_in sa;
                  int         x        = 1; 
                  int         fd;

#if defined( _MSC_VER )
    WSADATA wsaData;

    if ( WSAStartup( MAKEWORD( 1, 1 ), &wsaData ) != 0 )
    {
	perror( "No useable WINSOCK.DLL" );
	exit( 1 );
    }
#else
    system( "touch SHUTDOWN.TXT" );
#endif

    if ( ( fd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == -1 )
    {
        perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof( x ) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	CLOSE( fd );
	exit( 1 );
    }

#if defined( SO_DONTLINGER ) && !defined( SYSV )
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof( ld ) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    CLOSE( fd );
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof( sa ) ) == -1 )
    {
	perror( "Init_socket: bind" );
	CLOSE( fd );
	exit( 1 );
    }

    if ( listen( fd, 3 ) == -1 )
    {
	perror( "Init_socket: listen" );
	CLOSE( fd );
	exit( 1 );
    }

    remove( "SHUTDOWN.TXT" );
    return fd;
}



void game_loop_unix( int control )
{
    static struct timeval null_time;
           struct timeval last_time;

#if defined( __unix__ )
    signal( SIGPIPE, SIG_IGN   );
    signal( SIGXCPU, SIG_IGN   );
#endif

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;
	fd_set           in_set;
	fd_set           out_set;
	fd_set           exc_set;
	int              maxdesc;

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( (unsigned) maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );

#ifdef USE_PTHREADS
	    if ( d->lookup_status == NAME_LOOKUP_FOUND )
	    {
                pthread_join( d->thread_id, NULL );

      	      	if ( d->found_host )
	      	{
		    free_string( d->host );
		    d->host = str_dup( d->found_host );
		    free( d->found_host );
		    d->found_host = NULL;
	      	}

	      	if ( d->found_user )
	      	{
		    free_string( d->user );
		    d->user = str_dup( d->found_user );
		    free( d->found_user );
		    d->found_user = NULL;
	      	}
		
		d->lookup_status = NAME_LOOKUP_DONE;
	    }
#endif
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    if ( errno != EINTR )
	    {
		perror( "Game_loop: select: poll" );
		exit( 1 );
	    }

	    continue;
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    new_descriptor( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d; d = d_next )
	{
	    d_next = d->next;   

	    if ( !FD_ISSET( d->descriptor, &exc_set ) )
		continue;

	    FD_CLR( d->descriptor, &in_set  );
	    FD_CLR( d->descriptor, &out_set );
	    if ( d->character )
		save_char_obj( d->character );
	    d->outtop	= 0;
	    close_socket( d );
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );

	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

		     if ( d->showstr_point )
			show_string( d, d->incomm );
		else if ( d->str_editing )
			string_add( d->character, d->incomm );
		else
		switch ( d->connected )
		{
		case CON_PLAYING:   interpret( d->character, d->incomm ); break;
		default:	    nanny               ( d, d->incomm ); break;
		}

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
		&& FD_ISSET( d->descriptor, &out_set ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
#if !defined( _MSC_VER )
	{
	    struct timeval now_time;
	           long    secDelta;
	           long    usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ( (int) last_time.tv_usec )
	                - ( (int)  now_time.tv_usec )
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ( (int) last_time.tv_sec )
	                - ( (int)  now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    if ( errno != EINTR )
		    {
			perror( "Game_loop: select: stall" );
			exit( 1 );
		    }
		}
	    }
	}
#else
	{
	           int    times_up;
	           int    nappy_time;
	    struct _timeb start_time;
	    struct _timeb end_time;

	    _ftime( &start_time );
	    times_up = 0;

	    while ( times_up == 0 )
	    {
		_ftime( &end_time );
		if ( ( nappy_time =
		      (int) ( 1000.0 *
			     (double) ( ( end_time.time - start_time.time ) +
				       ( (double) ( end_time.millitm -
						   start_time.millitm ) *
					0.001 ) ) ) ) >=
		    (double)( 1000.0 / PULSE_PER_SECOND ) )
		  times_up = 1;
		else
		{
		    Sleep( (int) ( (double) ( 1000 / PULSE_PER_SECOND ) -
				   (double) nappy_time ) );
		    times_up = 1;
		}
	    }
	}
#endif

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}



int nonblock( int desc )
{
#if defined( _MSC_VER )
    unsigned long val = 1;

    return ioctlsocket( desc, FIONBIO, &val );
#elif defined( AmigaTCP )
    long val = 1;

    return IoctlSocket( desc, FIONBIO, &val );
#elif defined( __unix__ )
#  if !defined( FNDELAY )
#    if defined( __hpux )
#      define FNDELAY O_NONBLOCK
#    else
#      define FNDELAY O_NDELAY
#    endif
#  endif
    return fcntl( desc, F_SETFL, FNDELAY );
#else
    return 0;
#endif
}

bool wouldblock( void )
{
#if	defined( __unix__ )
    return ( errno == EWOULDBLOCK || errno == EAGAIN );
#elif	defined( _MSC_VER )
    return ( WSAGetLastError( ) == WSAEWOULDBLOCK || errno == EAGAIN );
#else
    return FALSE;
#endif
}


#ifdef USE_PTHREADS
char *mystrdup( char *s )
{
    char *p;
    
    p = (char *) malloc( strlen( s ) + 1 );
    if ( p != NULL )
      strcpy( p, s );
    return p;
}

int start_socket( struct sockaddr_in *sa )
{
    int 	    fd;
    u_short         port;

    if ( ( fd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == -1 )
    {
        perror( "name_thr: socket" );
	return -1;
    }

    port	   = htons( IDENT_PORT );
    sa->sin_family = AF_INET;
    sa->sin_port   = port;

    if ( connect( fd, (struct sockaddr *) sa, sizeof( *sa ) ) == -1 )
    {
	perror( "name_thr: connect" );
	CLOSE( fd );
	return -1;
    }

    return fd;
}

void name_thr_cleanup( void *argument )
{
  ((DESCRIPTOR_DATA *)argument)->lookup_status = NAME_LOOKUP_FOUND;
}

void *name_thr( void *argument )
{
    struct sockaddr_in  us;
    struct sockaddr_in  them;
    DESCRIPTOR_DATA    *d;
    struct hostent     *he;
    size_t	        size;
    int			sd;
    char	        buf	[ MAX_STRING_LENGTH ];
    char	        host	[ MAX_STRING_LENGTH ];
    char	        user	[ MAX_STRING_LENGTH ];
    int			nread;
    int       	      	desc;

    d = (DESCRIPTOR_DATA *) argument;

    host[0] = '\0';
    user[0] = '\0';

    desc = d->descriptor;

    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
    pthread_cleanup_push( name_thr_cleanup, argument );

    size = sizeof( us );
    if ( getsockname( desc, (struct sockaddr *) &us, &size ) == -1 )
    {
        perror( "name_thr: getsockname " );
	return NULL;
    }

    size = sizeof( them );
    if ( getpeername( desc, (struct sockaddr *) &them, &size ) == -1 )
    {
        perror( "name_thr: getpeername " );
	return NULL;
    }

    he = gethostbyaddr((char *)&them.sin_addr, sizeof(&them.sin_addr), AF_INET);

    if ( he )
      strcpy( host, he->h_name );

    sprintf( buf, "%u, %u\n", ntohs( them.sin_port ), ntohs( us.sin_port ) );

    if ( ( sd = start_socket( &them ) ) == -1 )
	return NULL;

    WRITE( sd, buf, strlen( buf ) + 1 );

    nread = READ( sd, buf, MAX_STRING_LENGTH );

    CLOSE( sd );

    if ( nread != -1 )
    {
	buf[nread] = '\0';

	sscanf( buf, "%*d , %*d : USERID : %*s : %[^\r\n]s", user );
    }

    if ( host[0] != '\0' )
        d->found_host = mystrdup( host );
    if ( user[0] != '\0' )
        d->found_user = mystrdup( user );

    pthread_cleanup_pop( 1 );
    return NULL;
}
#endif


void new_descriptor( int control )
{
    static DESCRIPTOR_DATA  d_zero;
           DESCRIPTOR_DATA *dnew;
    struct sockaddr_in      sock;
           BAN_DATA        *pban;
    char                    buf [ MAX_STRING_LENGTH ];
    int                     desc;
    socklen_t               size;
    int                     addr;
    int			    i;

    size = sizeof( sock );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size ) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }


    if ( nonblock( desc ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    if ( !descriptor_free )
    {
	dnew		= (DESCRIPTOR_DATA *) alloc_perm( sizeof( *dnew ) );
    }
    else
    {
	dnew		= descriptor_free;
	descriptor_free	= descriptor_free->next;
    }

    *dnew		= d_zero;
    dnew->descriptor	= desc;
    dnew->character     = NULL;
    dnew->connected	= CON_GET_NAME;
    dnew->showstr_head  = str_dup( "" );
    dnew->showstr_point = 0;
    dnew->str_editing	= NULL;
    dnew->outsize	= 2000;
    dnew->outbuf	= (char *) alloc_mem( dnew->outsize );

    for ( i = 0; i < MAX_HISTORY; i++)
      dnew->hist[i]	= NULL;

    dnew->histsize	= 0;

    size = sizeof( sock );

    /*
     * Would be nice to use inet_ntoa here but it takes a struct arg,
     * which ain't very compatible between gcc and system libraries.
     */
    addr = ntohl( sock.sin_addr.s_addr );
    sprintf( buf, "%d.%d.%d.%d",
	( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	);
    dnew->host = str_dup( buf );
    dnew->user = str_dup( "(unknown)" );

#ifdef USE_PTHREADS
    {
	dnew->found_host = NULL;
	dnew->found_user = NULL;
        dnew->lookup_status = NAME_LOOKUP_SPAWNED;

	if ( pthread_create( &dnew->thread_id, NULL, name_thr, dnew ) != 0 )
	    dnew->lookup_status = NAME_LOOKUP_DONE;
    }
#endif

    logln( "Sock.sinaddr:  %s@%s", dnew->user, buf );

    sprintf( log_buf, "New connection: %s@%s", dnew->user, dnew->host );
    wiznet( NULL, WIZ_LOGINS, L_DIR, log_buf );


    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    for ( pban = ban_list; pban; pban = pban->next )
    {
	if ( !str_suffix( pban->name, dnew->host ) )
	{
	    write_to_descriptor( desc,
		"Your site has been banned from this Mud.\n\r", 0 );
	    CLOSE( desc );
	    free_string( dnew->host );
	    free_mem( dnew->outbuf, dnew->outsize );
	    dnew->next		= descriptor_free;
	    descriptor_free	= dnew;
	    return;
	}
    }

    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;

	if ( help_greeting[0] == '.' )
	    write_to_buffer( dnew, help_greeting+1, 0 );
	else
	    write_to_buffer( dnew, help_greeting  , 0 );
    }

    if ( ++num_descriptors > sysdata.max_players )
	sysdata.max_players = num_descriptors;
    if ( sysdata.max_players > sysdata.all_time_max )
    {
	free_string( sysdata.time_of_max );
	sprintf( buf, "%24.24s", ctime( &current_time ) );
	sysdata.time_of_max = str_dup( buf );
	sysdata.all_time_max = sysdata.max_players;
	logln( "Broke all-time maximum player record: %d",
	     sysdata.all_time_max );
	save_sysdata( );
    }

    return;
}



void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) )
    {
	logln( "Closing link to %s.", ch->name );
	if ( IS_PLAYING( dclose ) )
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    ch->desc = NULL;
	}
	else
	{
	    free_char( dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

#ifdef USE_PTHREADS
    if ( dclose->lookup_status == NAME_LOOKUP_SPAWNED )
    {
	pthread_cancel( dclose->thread_id );
      	pthread_join( dclose->thread_id, NULL );
    }

    if ( dclose->found_host )
      free( dclose->found_host );
    if ( dclose->found_user )
      free( dclose->found_user );
#endif

    CLOSE( dclose->descriptor );
    free_string( dclose->host );
    free_string( dclose->user );

    /* RT socket leak fix */
    free_mem( dclose->outbuf, dclose->outsize );

    dclose->next	= descriptor_free;
    descriptor_free	= dclose;

    --num_descriptors;
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    size_t iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen( d->inbuf );
    if ( iStart >= sizeof( d->inbuf ) - 10 )
    {
	logln( "%s input overflow!", d->host );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
    for ( ; ; )
    {
	int nRead;

	nRead = READ( d->descriptor, d->inbuf + iStart,
		     sizeof( d->inbuf ) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    logln( "EOF encountered on read." );
	    return FALSE;
	}
        else if ( wouldblock( ) )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    CHAR_DATA    *ch;
    int           i;
    int		  j;
    int           k;
    char	  inbuf [ MAX_INPUT_LENGTH*4 ];

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii( d->inbuf[i] ) && isprint( d->inbuf[i] ) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 )
    {
        if ( d->incomm[0] != '!' )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 31 )
	    {
		logln( "%s input spamming!", d->host );
		wiznet( NULL, WIZ_SPAM, 0, log_buf );
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
	    }
	}
    }

    if ( IS_PLAYING( d ) && d->histsize > 0 )
    {
      /*
       * Do '!' substitution.  - Zen.
       */
      if ( d->incomm[0] == '!' )
      {
	char *cmd;

	cmd = NULL;

        if ( d->incomm[1] == '!' )
	{
	  cmd = d->hist[d->histsize-1];
	}
	else
	if ( d->incomm[1] == '?' )
	{
	  int n;

	  for ( n = d->histsize-1; n >= 0; n-- )
	  {
	      if ( !str_infix( &d->incomm[2], d->hist[n] ) )
	      {
	          cmd = d->hist[n];
	          break;
	      }
	  }
	}
	else
	if ( is_number( &d->incomm[1] ) )
	{
	  int n;

	  n = atoi( &d->incomm[1] );
	  if ( n >= 0 )
	      cmd = (d->histsize-n >  0) ? d->hist[n]:NULL;
	  else
	      cmd = (d->histsize+n >= 0) ? d->hist[d->histsize+n]:NULL;
	}
	else
	{
	  int n;

	  for ( n = d->histsize-1; n >= 0; n-- )
	  {
	      if ( !str_prefix( &d->incomm[1], d->hist[n] ) )
	      {
	          cmd = d->hist[n];
	          break;
	      }
	  }
	}

	if ( cmd )
	{
	  strcpy( d->incomm, cmd );
	  write_to_descriptor( d->descriptor, d->incomm, 0 );
	  write_to_descriptor( d->descriptor, "\n\r", 2 );
	}
      }

      /*
       * Do '^' substitution.  - Zen.
       */
      if ( d->incomm[0] == '^' )
      {
	char  arg1 [ MAX_INPUT_LENGTH ];
	char  arg2 [ MAX_INPUT_LENGTH ];
        char  buf  [ MAX_INPUT_LENGTH ];
	char *ptr1;
	char *ptr2;

	arg1[0] = '\0';
	arg2[0] = '\0';

	strcpy( buf, &d->incomm[1] );

	ptr1 = buf;
	if ( ( ptr2 = strchr( ptr1, '^' ) ) )
	{
	  *ptr2 = '\0';
	  strcpy( arg1, ptr1 );

	  ptr1 = ptr2 + 1;    
	  if ( ( ptr2 = strchr( ptr1, '^' ) ) )
	  {
	    *ptr2 = '\0';
	    strcpy( arg2, ptr1 );
	  }
	}

	if ( arg1[0] != '\0' )
	{
	  strexg( d->incomm, d->hist[d->histsize-1], arg1, arg2 );
	  write_to_descriptor( d->descriptor, d->incomm, 0 );
	  write_to_descriptor( d->descriptor, "\n\r", 2 );
	}
      }
    }
    
    
    /*
     * Update history list.  - Zen
     */
    if (   IS_PLAYING( d )
	&& d->incomm[0] != '!'
	&& d->incomm[0] != '^' )
    {
	char *cmd;
	int   n;

	cmd = str_dup( d->incomm );

	if ( d->histsize >= MAX_HISTORY )
	{
	    free_string( d->hist[0] );
	    
	    d->histsize = MAX_HISTORY-1;
	    for ( n = 0; n < d->histsize; n++ )
	      d->hist[n]=d->hist[n+1];
	}

	d->hist[d->histsize] = cmd;
	d->histsize++;
    }

    ch = ( d->original ? d->original : d->character );


    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;

    inbuf[0] = '\0';
    j	     = 0;

    /*
     * Do alias substitution.
     */
    if ( IS_PLAYING( d ) && ch )
    {
	ALIAS_DATA *alias;
	char *      arg1;
	char        cmd	 [ MAX_INPUT_LENGTH ];

	arg1 = one_argument( d->incomm, cmd );

	for ( alias = ch->pcdata->alias_list; alias; alias = alias->next )
	    if ( !str_cmp( cmd, alias->cmd ) ) break;

	if ( alias )
	{
	    char  arg [ MAX_INPUT_LENGTH ];
	    int   n;
	    char *e;

	    strcpy( arg, arg1 );
	    n = strexg( d->incomm, alias->subst, "$", arg );

	    if ( ( e = strstr( d->incomm, "\n\r" ) ) )
	    {
	      *e = '\0';
	      e += 2;
	      strcpy( inbuf, e );
	      j = n - (e - d->incomm);
	    }
	}
    }

    if ( MAX_INPUT_LENGTH-j > 0 )
      strncat( inbuf, &d->inbuf[i], MAX_INPUT_LENGTH-j );
    strcpy( d->inbuf, inbuf );
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
	   char *ptr;
	   char  buf [ MAX_STRING_LENGTH ];
	   int   shown_lines = 0;
	   int   total_lines = 0;
	   int   n;
    extern bool  merc_down;

    /*
     * Bust a prompt.
     */
    if ( fPrompt && !merc_down && IS_PLAYING( d ) )
    {
        if ( d->showstr_point )
	{
	    for ( ptr = d->showstr_head; ptr != d->showstr_point; ptr++ )
		if ( *ptr == '\n' )
		    shown_lines++;

	    total_lines = shown_lines;
	    for ( ptr = d->showstr_point; *ptr != '\0'; ptr++ )
		if ( *ptr == '\n' )
		    total_lines++;

	    sprintf( buf,
" %s(%d%%) Please type (c)ontinue, (r)efresh, (b)ack, (q)uit, or RETURN %s\r",
		    MOD_REVERSE, 100 * shown_lines / total_lines, MOD_CLEAR );
	    write_to_buffer( d, buf, 0 );
	}
	else
	  if ( d->connected > CON_PASSWD_GET_OLD && ! d->str_editing )
	  {
	    CHAR_DATA *ch;

	    ch = d->original ? d->original : d->character;
	    if ( xIS_SET( ch->act, PLR_BLANK     ) )
	        write_to_buffer( d, "\n\r", 2 );

	    if ( xIS_SET( ch->act, PLR_PROMPT    ) )
	        bust_a_prompt( d );

	    if ( xIS_SET( ch->act, PLR_TELNET_GA ) )
	        write_to_buffer( d, go_ahead_str, 0 );
	  }
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by )
    {
	write_to_buffer( d->snoop_by, "% ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    n = write_to_descriptor( d->descriptor, d->outbuf, d->outtop );
    if ( n == -1 )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	if ( n > 0 )
	{
	    d->outtop -= n;
	    memmove( &d->outbuf[0], &d->outbuf[n], d->outtop );
	    d->outbuf[d->outtop] = '\0';
	}
	
	return TRUE;
    }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( DESCRIPTOR_DATA *d )
{
         CHAR_DATA *ch;
   const char      *str;
   const char      *i;
         char      *point;
         char       buf  [ MAX_STRING_LENGTH ];
         char       buf2 [ MAX_STRING_LENGTH ];
         char      *pbuff;
         char       buffer [ 4 * MAX_STRING_LENGTH ];

   /* Will always have a pc ch after this */
   ch = ( d->original ? d->original : d->character );
   if( !ch->pcdata->prompt || ch->pcdata->prompt[0] == '\0' )
   {
      send_to_char( "\n\r\n\r", ch );
      return;
   }

   point = buf;
   str = ch->pcdata->prompt;

   while( *str != '\0' )
   {
      if( *str != '%' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
         default :
            i = " "; break;
         case 'h' :
            sprintf( buf2, "%d", ch->hit                               );
            i = buf2; break;
         case 'H' :
            sprintf( buf2, "%d", ch->max_hit                           );
            i = buf2; break;
         case 'm' :
            sprintf( buf2, "%d", ch->mana                              );
            i = buf2; break;
         case 'M' :
            sprintf( buf2, "%d", ch->max_mana                          );
            i = buf2; break;
         case 'u' :
            sprintf( buf2, "%d", num_descriptors                       ); 
            i = buf2; break;
         case 'U' :
            sprintf( buf2, "%d", sysdata.max_players                   );
            i = buf2; break;
         case 'v' :
            sprintf( buf2, "%d", ch->move                              ); 
            i = buf2; break;
         case 'V' :
            sprintf( buf2, "%d", ch->max_move                          );
            i = buf2; break;
         case 'x' :
            sprintf( buf2, "%d", ch->exp                               );
            i = buf2; break;
         case 'X' :
            sprintf( buf2, "%d", (ch->level+1)*EXP_PER_LEVEL - ch->exp );
            i = buf2; break;
         case 'g' :
            sprintf( buf2, "%d", ch->gold                              );
            i = buf2; break;
         case 'w' :
	    sprintf( buf2, "%d", ch->wait                              );
	    i = buf2; break;
         case 'a' :
            if( ch->level > 10 )
               sprintf( buf2, "%d", ch->alignment                      );
            else
               sprintf( buf2, "%s", IS_GOOD( ch ) ? "good"
		                  : IS_EVIL( ch ) ? "evil" : "neutral" );
            i = buf2; break;
         case 'r' :
            if( ch->in_room )
               sprintf( buf2, "%s",
               ( ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) ) ||
                 ( !IS_AFFECTED( ch, AFF_BLIND ) &&
		   !room_is_dark( ch->in_room ) ) )
	       ? ch->in_room->name : "darkness"                        );
            else
               sprintf( buf2, " "                                      );
            i = buf2; break;
         case 'R' :
            if( IS_IMMORTAL( ch ) && ch->in_room )
               sprintf( buf2, "%d", ch->in_room->vnum                  );
            else
               sprintf( buf2, " "                                      );
            i = buf2; break;
         case 'z' :
            if( IS_IMMORTAL( ch ) && ch->in_room )
               sprintf( buf2, "%s", ch->in_room->area->name            );
            else
               sprintf( buf2, " "                                      );
            i = buf2; break;
         case 'i' :  /* Invisible notification on prompt sent by Kaneda */
	    sprintf( buf2, "%s", IS_AFFECTED( ch, AFF_INVISIBLE ) ?
		                 "invisible" : "visible" );
	    i = buf2; break;
         case 'I' :
            if( IS_IMMORTAL( ch ) )
               sprintf( buf2, "(wizinv: %s)", xIS_SET( ch->act, PLR_WIZINVIS ) ?
		                              "on" : "off" );
            else
               sprintf( buf2, " "                                      );
            i = buf2; break;
         case '%' :
            sprintf( buf2, "%%"                                        );
            i = buf2; break;
      }
      ++str;
      while( ( *point = *i ) != '\0' )
         ++point, ++i;
   }
   *point = '\0';

   pbuff	= buffer;
   colourconv( pbuff, buf, ch );
   write_to_buffer( d, buffer, 0 );
   return;
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen( txt );

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

        if ( d->outsize >= 32000 )
        {
            /* empty buffer */
            d->outtop = 0;
            bugf( "Buffer overflow. Closing (%s).",
		( d->character ? d->character->name : "???" ) );
	    write_to_descriptor( d->descriptor, "\n\r*** BUFFER OVERFLOW!!! ***\n\r", 0 );
	    close_socket( d );
            return;
        }

	outbuf      = (char *) alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.  Modifications sent in by Zavod.
     */
    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    d->outbuf[d->outtop] = '\0';
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 */
int write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

    if ( length <= 0 )
	length = strlen( txt );

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = WRITE( desc, txt + iStart, nBlock ) ) == -1 )
	{
	  if ( wouldblock( ) )
	    return iStart;
	  perror( "Write_to_descriptor" );
	  return -1;
	}
    } 

    return length;
}



void display_title( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch;

    ch = d->character;

    if ( xIS_SET( ch->act, PLR_COLOUR ) )
    {
	write_to_buffer( d, "\033[2J", 0 );
	send_ansi_title( d );
    }
    else
    {
	write_to_buffer( d, "\014", 0 );
	send_ascii_title( d );
    }

    write_to_buffer( d, "\014", 0 );
    write_to_buffer( d, "Press [RETURN] ", 0 );

    return;
}


void display_classes( CHAR_DATA *ch )
{
    CLASS_TYPE *cclass;
    char        buf  [ MAX_STRING_LENGTH ];
    char        buf1 [ MAX_STRING_LENGTH ];
    int         col;

    col     = 0;
    buf1[0] = '\0';

    strcat( buf1, "{o{b--------------------------------[ {wClass list {b]--------------------------------{x\n\r\n\r" );

    for ( cclass = class_first; cclass; cclass = cclass->next )
    {
	if ( is_class( ch, cclass ) )
	    continue;

	sprintf( buf, "%18s  ", cclass->name );
	strcat( buf1, buf );
	if ( ++col % 4 == 0 )
	    strcat( buf1, "\n\r" );
    }

    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );

    return;
}



/*
 * Deal with sockets that haven't logged in yet, and then some more!
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    CHAR_DATA  *ch;
    char       *pwdnew;
    char       *classname;
    CLASS_TYPE *cclass;
    char       *p;
    char        buf  [ MAX_STRING_LENGTH ];
    char        buf1 [ MAX_STRING_LENGTH ];
    int         iRace;
    int         lines;
    int         col;
    bool        fOld;

    while ( isspace( *argument ) )
	argument++;

    /* This is here so we wont get warnings.  ch = NULL anyways - Kahn */
   ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER( argument[0] );

	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
	}

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if ( xIS_SET( ch->act, PLR_DENY ) )
	{
	    logln( "Denying access to %s@%s.", argument, d->host );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    /* Must be immortal with wizbit in order to get in */
	    if ( wizlock
		&& !IS_HERO( ch )
		&& !xIS_SET( ch->act, PLR_WIZBIT ) )
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	    if ( ch->level <= numlock
		&& !xIS_SET( ch->act, PLR_WIZBIT )
		&& numlock != 0 )
	    {

		write_to_buffer( d,
			"The game is locked to your level character.\n\r\n\r",
				0 );
		if ( ch->level == 0 )
		    do_help( ch, "wizlocked" );
		close_socket( d ) ;
		return;
	    }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
	    sprintf( buf, "Did I get that right, %s [Y/N]? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );

	if ( check_playing( d, ch->name ) )
	    return;

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	logln( "%s@%s has connected.", ch->name, d->host );

	display_title( d );

	d->connected = CON_READ_MOTD;
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "New character.\n\rGive me a password for %s: %s",
		    ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strlen( argument ) < 5 )
	{
	    write_to_buffer( d,
	       "Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );

	write_to_buffer( d, "\n\rDo you want ANSI colour [Y/N]? ", 0 );
	d->connected = CON_GET_COLOUR;
	break;

    case CON_GET_COLOUR:
	switch ( argument[0] )
	{
	case 'y': case 'Y': xSET_BIT( ch->act, PLR_COLOUR );	break;
	case 'n': case 'N':					break;
	default:
	    write_to_buffer( d, "That's an option.\n\rDo you want ANSI colour [Y/N]? ", 0 );
	    return;
	}

	write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
	d->connected = CON_DISPLAY_RACE;
	break;

    case CON_DISPLAY_RACE:
	col     = 0;
	buf1[0] = '\0';

	strcat( buf1, "{o{b--------------------------------[ {wRace list {b]---------------------------------{x\n\r\n\r" );

	for ( iRace = 0; iRace < MAX_RACE; iRace++ )
	{
	    if ( !IS_SET( race_table[ iRace ].race_abilities, RACE_PC_AVAIL ) )
	        continue;
	    sprintf( buf, "%18s  ", race_table[iRace].name );
	    strcat( buf1, buf );
	    if ( ++col % 4 == 0 )
		strcat( buf1, "\n\r" );
	}

	if ( col % 4 != 0 )
	    strcat( buf1, "\n\r" );

	strcat( buf1, "\n\rSelect a race: " );

	send_to_char( buf1, ch );
	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_GET_NEW_RACE:
	for ( iRace = 0; iRace < MAX_RACE; iRace++ )
	    if ( !str_prefix( argument, race_table[iRace].name )
		&& IS_SET( race_table[ iRace ].race_abilities,
			  RACE_PC_AVAIL ) )
	    {
		ch->race = race_lookup( race_table[iRace].name );
		break;
	    }

	if ( iRace == MAX_RACE )
	{
	    write_to_buffer( d,
			    "That is not a race.\n\rWhat IS your race? ", 0 );
	    return;
	}

	write_to_buffer( d, "\n\r", 2 );
	do_help( ch, race_table[ch->race].name );

	send_to_char( "\n\r{oOther racial features include:{x\n\r", ch );
	sprintf( buf, "        * Size %s.\n\r",
		flag_string( size_flags, race_table[ch->race].size ) );
	send_to_char( buf, ch );

	sprintf( buf,
		"        * Str %+d, Int %+d, Wis %+d, Dex %+d, Con %+d.\n\r",
		race_table[ch->race].str_mod,
		race_table[ch->race].int_mod,
		race_table[ch->race].wis_mod,
		race_table[ch->race].dex_mod,
		race_table[ch->race].con_mod );
	send_to_char( buf, ch );

	write_to_buffer( d, "\n\rAre you sure you want this race?  ", 0 );
	d->connected = CON_CONFIRM_NEW_RACE;
	break;

    case CON_CONFIRM_NEW_RACE:
	switch ( argument[0] )
	{
	  case 'y': case 'Y': break;
	  default:
	      write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
	      d->connected = CON_DISPLAY_RACE;
	      return;
	}

	ch->pcdata->points = 0;

	write_to_buffer( d, "\n\rWhat is your sex [M/F/N]? ", 0 );
	d->connected = CON_GET_NEW_SEX;
	break;

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE;  break;
	case 'n': case 'N': ch->sex = SEX_NEUTRAL; break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}

	d->connected = CON_DISPLAY_1ST_CLASS;
	write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
	break;
	
    case CON_DISPLAY_1ST_CLASS:
	display_classes( ch );

	send_to_char( "\n\rSelect a primary class: ", ch );

	d->connected = CON_GET_1ST_CLASS;
	break;

    case CON_GET_1ST_CLASS:
	if ( !( cclass = class_lookup( argument ) ) || is_class( ch, cclass ) )
	{
	    write_to_buffer( d,
		"That's not a class.\n\rWhat IS your class? ", 0 );
	    return;
	}

	ch->cclass[0] = cclass;
	classname     = cclass->name;

	write_to_buffer( d, "\n\r", 2 );

	if ( classname[0] != '\0' )
	    do_help( ch, classname );
	else
	    bug( "Nanny CON_GET_1ST_CLASS:  ch->cclass[0] (%d) not valid", 0 );

	write_to_buffer( d, "Are you sure you want this class?  ", 0 );
	d->connected = CON_CONFIRM_1ST_CLASS;
	break;

    case CON_CONFIRM_1ST_CLASS:
	switch ( argument[0] )
	{
	  case 'y': case 'Y': break;
	  default:
	      ch->cclass[0] = NULL;

	      write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
	      d->connected = CON_DISPLAY_1ST_CLASS;
	      return;
	}

	write_to_buffer( d, "\n\r", 2 );

        write_to_buffer( d, "Multiclassing allows you to have more skills at a xp cost...\n\r", 0 );
        write_to_buffer( d, "Do you want a multiclass character [Y/N]? ", 0 );
        d->connected = CON_DEFAULT_CHOICE;
	break;

case CON_DEFAULT_CHOICE:
        write_to_buffer( d, "\n\r", 2 );

        switch ( argument[0] )
        {
        case 'y': case 'Y':
	    write_to_buffer( d, "Press Return to continue:\n\r", 0 );

	    d->connected = CON_DISPLAY_2ND_CLASS;
            break;
        case 'n': case 'N':
	    send_to_char( "\n\rAlignment [G/N/E]? ", ch );

	    d->connected = CON_GET_NEW_ALIGNMENT;
            break;
        default:
            write_to_buffer( d, "Multiclass [Y/N]? ", 0 );
            return;
        }
        break;

    case CON_DISPLAY_2ND_CLASS:
	display_classes( ch );

	send_to_char( "\n\rSelect a secondary class: ", ch );

	d->connected = CON_GET_2ND_CLASS;
	break;

    case CON_GET_2ND_CLASS:
	if ( !( cclass = class_lookup( argument ) ) || is_class( ch, cclass ) )
	{
	    write_to_buffer( d,
		"That's not a class.\n\rWhat IS your class? ", 0 );
	    return;
	}

	ch->cclass[1] = cclass;
	classname     = cclass->name;

	write_to_buffer( d, "\n\r", 2 );

	if ( classname[0] != '\0' )
	    do_help( ch, classname );
	else
	    bug( "Nanny CON_GET_2ND_CLASS:  ch->cclass[1] (%d) not valid", 0 );

	write_to_buffer( d, "Are you sure you want this class?  ", 0 );
	d->connected = CON_CONFIRM_2ND_CLASS;
	break;

    case CON_CONFIRM_2ND_CLASS:
	switch ( argument[0] )
	{
	  case 'y': case 'Y': break;
	  default:
	      ch->cclass[1] = NULL;

	      write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
	      d->connected = CON_DISPLAY_2ND_CLASS;
	      return;
	}

	send_to_char( "\n\rAlignment [G/N/E]? ", ch );

	d->connected = CON_GET_NEW_ALIGNMENT;
	break;

    case CON_GET_NEW_ALIGNMENT:
        write_to_buffer( d, "\n\r", 2 );

	switch ( argument[0] )
	{
	  case 'g': case 'G': ch->alignment = 750;	break;
	  case 'n': case 'N': ch->alignment = 0;	break;
	  case 'e': case 'E': ch->alignment = -750;	break;
	  default:
	      write_to_buffer( d, "That's not a valid alignment.\n\r", 0 );
	      write_to_buffer( d, "Alignment [G/N/E]? ", 0 );
	      return;
	}

	logln( "%s@%s new player.", ch->name, d->host );
	wiznet( ch, WIZ_NEWBIE, 0, log_buf );

	write_to_buffer( d, "\n\r", 2 );
	ch->pcdata->pagelen = 20;

	display_title( d );

	d->connected = CON_SHOW_MOTD;
	break;

    case CON_SHOW_MOTD:
	if ( xIS_SET( ch->act, PLR_COLOUR ) )
	    write_to_buffer( d, "\033[2J", 0 );
	else
	    write_to_buffer( d, "\014", 0 );

	lines = ch->pcdata->pagelen;
	ch->pcdata->pagelen = 20;
	if ( IS_HERO( ch ) )
	    do_help( ch, "imotd" );
	do_help( ch, "motd" );
	ch->pcdata->pagelen = lines;

	write_to_buffer( d, "Press [RETURN] ", 0 );

	d->connected = CON_READ_MOTD;
        break;

    case CON_READ_MOTD:
	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;

	send_to_char(
    "\n\rWelcome to Greed Envy Diku Mud.  May your visit here be ... Fun.\n\r",
	    ch );

	if ( ch->level == 0 )
	{
	    OBJ_DATA *obj;
	    int       iClass;

	    switch ( ch->cclass[0]->attr_prime )
	    {
	    case APPLY_STR: ch->perm_str = 16; break;
	    case APPLY_INT: ch->perm_int = 16; break;
	    case APPLY_WIS: ch->perm_wis = 16; break;
	    case APPLY_DEX: ch->perm_dex = 16; break;
	    case APPLY_CON: ch->perm_con = 16; break;
	    }

	    ch->level	= 1;
	    ch->exp	= EXP_PER_LEVEL;
	    ch->gold    = 5500 + number_fuzzy( 3 )
	                * number_fuzzy( 4 ) * number_fuzzy( 5 ) * 9;
	    ch->hit	= ch->max_hit;
	    ch->mana	= ch->max_mana;
	    ch->move	= ch->max_move;

	    buf[0]	= '\0';
	    strcat( buf, "the" );

	    for ( iClass = 0; iClass < MAX_MULTICLASS && ch->cclass[iClass]; iClass++ )
		sprintf( buf+strlen( buf ), "/%s",
		    ch->cclass[iClass]->title[ch->level] [ch->sex == SEX_FEMALE ? 1 : 0] );

	    buf[3] = ' ';
	    set_title( ch, buf );

	    free_string( ch->pcdata->prompt );
	    ch->pcdata->prompt = str_dup( daPrompt );

	    obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_BANNER ), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_LIGHT );

	    obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_VEST   ), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_BODY );

	    obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_SHIELD ), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_SHIELD );

	    obj = create_object( 
				get_obj_index( ch->cclass[0]->weapon ),
				0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_WIELD );

	    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	}
	else if ( ch->in_room )
	{
	    char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL( ch ) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

	if ( !xIS_SET( ch->act, PLR_WIZINVIS )
	    && !IS_AFFECTED( ch, AFF_INVISIBLE ) )
	    act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );

        sprintf( log_buf, "%s has entered the game.", ch->name );
        wiznet( ch, WIZ_LOGINS, get_trust( ch ), log_buf );

	/* show for board status */
	do_board( ch, "" );
	do_look( ch, "auto" );
	break;

    case CON_PASSWD_GET_OLD:
	write_to_buffer( d, "\n\r", 2 );

        if ( argument[0] == '\0' )
	{
	    write_to_buffer( d, "Password change aborted.\n\r", 0 );
	    write_to_buffer( d, echo_on_str, 0 );
	    d->connected = CON_PLAYING;
	    return;
	}

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Wrong password.  Wait 10 seconds.\n\r", 0 );
	    WAIT_STATE( ch, 40 );
	    write_to_buffer( d, echo_on_str, 0 );
	    d->connected = CON_PLAYING;
	    return;
	}

	write_to_buffer( d, "New password: ", 0 );
	d->connected = CON_PASSWD_GET_NEW;
	break;

    case CON_PASSWD_GET_NEW:
	write_to_buffer( d, "\n\r", 2 );

        if ( argument[0] == '\0' )
	{
	    write_to_buffer( d, "Password change aborted.\n\r", 0 );
	    write_to_buffer( d, echo_on_str, 0 );
	    d->connected = CON_PLAYING;
	    return;
	}

	if ( strlen( argument ) < 5 )
	{
	    write_to_buffer( d,
	       "Password must be at least five characters long.\n\rNew password: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rNew password: ",
		    0 );
		return;
	    }
	}

	strcpy( buf, pwdnew );
	free_string( ch->pcdata->pwdnew );
	ch->pcdata->pwdnew = str_dup( buf );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_PASSWD_CONFIRM_NEW;
	break;

    case CON_PASSWD_CONFIRM_NEW:
	write_to_buffer( d, "\n\r", 2 );

	if ( strcmp( crypt( argument, ch->pcdata->pwdnew ), ch->pcdata->pwdnew ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_PASSWD_GET_NEW;
	    return;
	}

	strcpy( buf, ch->pcdata->pwdnew );
	free_string( ch->pcdata->pwdnew );
	ch->pcdata->pwdnew = str_dup( "" );
	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd = str_dup( buf );
	write_to_buffer( d, echo_on_str, 0 );
	ch->desc->connected = CON_PLAYING;
	break;

    case CON_RETIRE_GET_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );
	write_to_buffer( d, echo_on_str, 0 );

        if ( argument[0] == '\0' )
	{
	    write_to_buffer( d, "Retire aborted.\n\r", 0 );
	    d->connected = CON_PLAYING;
	    return;
	}

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Wrong password.  Wait 10 seconds.\n\r", 0 );
	    WAIT_STATE( ch, 40 );
	    d->connected = CON_PLAYING;
	    return;
	}

	write_to_buffer( d, "Are you sure [Y/N]? ", 0 );
	d->connected = CON_RETIRE_CONFIRM;
	break;

    case CON_RETIRE_CONFIRM:
	switch ( argument[0] )
	{
	  case 'y': case 'Y': break;
	  default:
	      write_to_buffer( d, "Retire aborted.\n\r", 0 );
	      d->connected = CON_PLAYING;
	      return;
	}

	write_to_buffer( d, "Hope you return soon brave adventurer!\n\r", 0 );
	write_to_buffer( d, "[Add little to little ",                     0 );
	write_to_buffer( d, "and there will be a big pile]\n\r\n\r",      0 );

	act( "$n has retired the game.", ch, NULL, NULL, TO_ROOM );
	logln( "%s has retired the game.", ch->name );
	wiznet( ch, WIZ_LOGINS, get_trust( ch ), log_buf );

	do_quit( ch, "" );

	delete_char_obj( ch ); /* handy function huh? :) */
	break;

    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words & obsenities.
     */
    if ( is_name( name, strAsshole ) )
        return FALSE;

    /*
     * Length restrictions.
     */
    if ( strlen( name ) <  3 )
	return FALSE;

#if defined( __unix__ ) || defined( AmigaTCP )
    if ( strlen( name ) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha( *pc ) )
		return FALSE;
	    if ( LOWER( *pc ) != 'i' && LOWER( *pc ) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash [ MAX_KEY_HASH ];
	       MOB_INDEX_DATA *pMobIndex;
	       int             iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch; ch = ch->next )
    {
        if ( ch->deleted )
	    continue;

	if ( !IS_NPC( ch )
	    && ( !fConn || !ch->desc )
	    && !str_cmp( d->character->name, ch->name ) )
	{
 	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc     = d;
		ch->timer    = 0;
		d->connected = CON_PLAYING;

		send_to_char( "Reconnecting.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		logln( "%s@%s reconnected.", ch->name, d->host );
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;
    CHAR_DATA       *ch;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	    && dold->character
	    && !str_cmp( name, dold->original
			? dold->original->name : dold->character->name )
	    && !dold->character->deleted )
	{
	    if ( !IS_PLAYING( dold )
		&& dold->connected != CON_GET_NAME )
	    {
		write_to_buffer( d, "Already playing.\n\rName: ", 0 );
		d->connected = CON_GET_NAME;
		if ( d->character )
		{
		    free_char( d->character );
		    d->character = NULL;
		}
		return TRUE;
	    }

	    ch = ( dold->original ? dold->original : dold->character );

	    write_to_buffer( d,
		"Already playing...  kicking off old connection.\n\r", 0 );

	    write_to_buffer( dold,
		"Kicking off old connection...  bye!\n\r", 0 );
	    close_socket( dold );

	    free_char( d->character );
	    d->character = ch;
	    ch->desc     = d;
	    ch->timer    = 0;
	    d->connected = CON_PLAYING;

	    send_to_char( "Reconnecting.\n\r", ch );

	    act( "$n has reconnected, kicking off old link.",
		ch, NULL, NULL, TO_ROOM );

	    logln( "%s@%s reconnected, kicking off old link.",
		    ch->name, d->host );
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if (   !ch
	|| !ch->desc
	||  ch->desc->connected != CON_PLAYING
	|| !ch->was_in_room
	||  ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}

/*
 * Write to all in the room.
 */
void send_to_room( const char *txt, ROOM_INDEX_DATA *room )
{
    DESCRIPTOR_DATA *d;
    
    for ( d = descriptor_list; d; d = d->next )
        if ( d->character != NULL )
	    if ( d->character->in_room == room )
	        act( txt, d->character, NULL, NULL, TO_CHAR );
}

/*
 * Write to all characters.
 */
void send_to_all_char( const char *text )
{
    DESCRIPTOR_DATA *d;

    if ( !text )
        return;
    for ( d = descriptor_list; d; d = d->next )
        if ( IS_PLAYING( d ) )
	    send_to_char( text, d->character );

    return;
}

/*
 * Write to one char.
 */
void send_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if( !txt || !ch->desc )
        return;

    /*
     * Bypass the paging procedure if the text output is small
     * Saves process time.
     */
    if( strlen( txt ) < 600 || !xIS_SET( ch->act, PLR_PAGER ) )
	write_to_buffer( ch->desc, txt, strlen( txt ) );
    else
    {
        free_string( ch->desc->showstr_head );
	ch->desc->showstr_head  = str_dup( txt );
	ch->desc->showstr_point = ch->desc->showstr_head;
	show_string( ch->desc, "" );
    }

    return;
}

/*
 * Send to one char, new colour version, by Lope.
 * Enhanced by Zen.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    char  buf [ MAX_STRING_LENGTH * 4 ];

    if( !txt || !ch->desc )
        return;

    colourconv( buf, txt, ch );

    /*
     * Bypass the paging procedure if the text output is small
     * Saves process time.
     */
    if( strlen( buf ) < 600 || !xIS_SET( ch->act, PLR_PAGER ) )
	write_to_buffer( ch->desc, buf, strlen( buf ) );
    else
    {
        free_string( ch->desc->showstr_head );
	ch->desc->showstr_head  = str_dup( buf );
	ch->desc->showstr_point = ch->desc->showstr_head;
	show_string( ch->desc, "" );
    }

    return;
}

/*
 * source: EOD, by John Booth?
 */
void printf_to_char( CHAR_DATA *ch, char *fmt, ... )
{
    char        buf [ MAX_STRING_LENGTH ];
    va_list     args;

    va_start ( args, fmt );
    vsprintf ( buf, fmt, args );
    va_end ( args );

    send_to_char( buf, ch );
    return;
}

 /* The heart of the pager.  Thanks to N'Atas-Ha, ThePrincedom
    for porting this SillyMud code for MERC 2.0 and laying down the groundwork.
    Thanks to Blackstar, hopper.cs.uiowa.edu 4000 for which
    the improvements to the pager was modeled from.  - Kahn */
 /* 12/1/94 Fixed bounds and overflow bugs in pager thanks to BoneCrusher
    of EnvyMud Staff - Kahn */

void show_string( struct descriptor_data *d, char *input )
{
    register char *scan;
             char  buffer[ MAX_STRING_LENGTH*6 ];
             char  buf   [ MAX_INPUT_LENGTH    ];
             int   line      = 0;
             int   toggle    = 0;
             int   pagelines = 20;

    one_argument( input, buf );

    switch( UPPER( buf[0] ) )
    {
    case '\0':
    case 'C': /* show next page of text */
	break;

    case 'R': /* refresh current page of text */
	toggle = 1;
	break;

    case 'B': /* scroll back a page of text */
	toggle = 2;
	break;

    default: /*otherwise, stop the text viewing */
	if ( d->showstr_head )
	{
	    free_string( d->showstr_head );
	    d->showstr_head = str_dup( "" );
	}
	d->showstr_point = 0;
	return;

    }

    if ( d->original )
        pagelines = d->original->pcdata->pagelen;
    else
        pagelines = d->character->pcdata->pagelen;

    if ( toggle )
    {
	if ( d->showstr_point == d->showstr_head )
	    return;
	if ( toggle == 1 )
	    line = -1;
	do
	{
	    if ( *d->showstr_point == '\n' )
	      if ( ( line++ ) == ( pagelines * toggle ) )
		break;
	    d->showstr_point--;
	} while( d->showstr_point != d->showstr_head );
    }
    
    line    = 0;
    *buffer = 0;
    scan    = buffer;
    if ( *d->showstr_point )
    {
	do
	{
	    *scan = *d->showstr_point;
	    if ( *scan == '\n' )
	      if ( ( line++ ) == pagelines )
		{
		  scan++;
		  break;
		}
	    scan++;
	    d->showstr_point++;
	    if( *d->showstr_point == 0 )
	      break;
	} while( 1 );
    }

    /* On advice by Scott Mobley and others */
/*
    *scan++ = '\n';
    *scan++ = '\r';
*/
    *scan = 0;

    write_to_buffer( d, buffer, strlen( buffer ) );
    if ( *d->showstr_point == 0 )
    {
      free_string( d->showstr_head );
      d->showstr_head  = str_dup( "" );
      d->showstr_point = 0;
    }

    return;
}

/*
 * The primary output interface for formatted output.
 */
void act( const char *format, CHAR_DATA *ch, const void *arg1,
	 const void *arg2, int type )
{
           OBJ_DATA        *obj1        = (OBJ_DATA  *) arg1;
	   OBJ_DATA        *obj2        = (OBJ_DATA  *) arg2;
	   CHAR_DATA       *to;
	   CHAR_DATA       *vch         = (CHAR_DATA *) arg2;
    static char *    const  he_she  [ ] = { "it",  "he",  "she" };
    static char *    const  him_her [ ] = { "it",  "him", "her" };
    static char *    const  his_her [ ] = { "its", "his", "her" };
    const  char            *str;
    const  char            *i;
           char            *point;
           char            *pbuff;
           char             buf     [ MAX_STRING_LENGTH ];
           char             buf1    [ MAX_STRING_LENGTH ];
           char             buffer  [ MAX_STRING_LENGTH*2 ];
           char             fname   [ MAX_INPUT_LENGTH  ];
	   unsigned int	    num;

    /*
     * Discard null and zero-length messages.
     */
    if ( !format || format[0] == '\0' )
	return;

    if ( ch->deleted )
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
	if ( !vch )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    sprintf( buf1, "Bad act string:  %s", format );
	    bug( buf1, 0 );
	    return;
	}
	to = vch->in_room->people;
    }
    
    for ( ; to; to = to->next_in_room )
    {
	if ( to->deleted
	    || ( !to->desc && IS_NPC( to ) && !HAS_TRIGGER( to, TRIG_ACT ) )
	    || !IS_AWAKE( to ) )
	    continue;

	if ( type == TO_CHAR    && to != ch )
	    continue;
	if ( type == TO_VICT    && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM    && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;

	point	= buf;
	str	= format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;

	    if ( !arg2 && isupper( *str ) )
	    {
		bug( "Act: missing arg2 for code %d.", *str );
		sprintf( buf1, "Bad act string:  %s", format );
		bug( buf1, 0 );
		i = " <@@@> ";
	    }
	    else
	    {
		switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
		          sprintf( buf1, "Bad act string:  %s", format );
		          bug( buf1, 0 );
			  i = " <@@@> ";				break;
		/* Thx alex for 't' idea */
		case 't': i = (char *) arg1;				break;
		case 'T': i = (char *) arg2;          			break;
		case 'n': i = PERS( ch,  to  );				break;
		case 'N': i = PERS( vch, to  );				break;
		case 'e': i = he_she  [URANGE( 0, ch  ->sex, 2 )];	break;
		case 'E': i = he_she  [URANGE( 0, vch ->sex, 2 )];	break;
		case 'm': i = him_her [URANGE( 0, ch  ->sex, 2 )];	break;
		case 'M': i = him_her [URANGE( 0, vch ->sex, 2 )];	break;
		case 's': i = his_her [URANGE( 0, ch  ->sex, 2 )];	break;
		case 'S': i = his_her [URANGE( 0, vch ->sex, 2 )];	break;

		case 'p':
		    i = can_see_obj( to, obj1 )
			    ? obj1->short_descr
			    : "something";
		    break;

		case 'P':
		    i = can_see_obj( to, obj2 )
			    ? obj2->short_descr
			    : "something";
		    break;

		case 'd':
		    if ( !arg2 || ( (char *) arg2 )[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
		}
	    }
		
	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}

        *point++	= '\n';
        *point++	= '\r';
        *point		= '\0';

	/* needed to capitalize because of ColourUp */
	for ( num = 0; num <= strlen( buf ) ; num++ )
	{
	    if ( buf[num] == '{' )
		num++;
	    else
	    {
		buf[num] = UPPER( buf[num] );
		break;
	    }
	}

	pbuff		= buffer;
	colourconv( pbuff, buf, to );
	if ( to->desc && to->desc->connected == CON_PLAYING )
	    write_to_buffer( to->desc, buffer, 0 );
	if ( MOBtrigger && IS_NPC( to ) )
	    mp_act_trigger( buffer, to, ch, obj1, vch, TRIG_ACT );
    }

    MOBtrigger = TRUE;
    return;
}


int colour( char type, CHAR_DATA *ch, char *string )
{
    char  *code;
    char  *p;

    code = NULL;

    if ( ch->desc && xIS_SET( ch->act, PLR_COLOUR ) )
	switch ( type )
	{
	case 'x':	code = MOD_CLEAR;	break;
	case 'o':	code = MOD_BOLD;	break;
	case 'l':	code = MOD_BLINK;	break;
	case 'a':	code = MOD_FAINT;	break;
	case 'n':	code = MOD_UNDERLINE;	break;
	case 'e':	code = MOD_REVERSE;	break;
	case 'b':	code = FG_BLUE;		break;
	case 'c':	code = FG_CYAN;		break;
	case 'd':	code = FG_BLACK;	break;
	case 'g':	code = FG_GREEN;	break;
	case 'm':	code = FG_MAGENTA;	break;
	case 'r':	code = FG_RED;		break;
	case 'w':	code = FG_WHITE;	break;
	case 'y':	code = FG_YELLOW;	break;
	case 'B':	code = BG_BLUE;		break;
	case 'C':	code = BG_CYAN;		break;
	case 'G':	code = BG_GREEN;	break;
	case 'M':	code = BG_MAGENTA;	break;
	case 'R':	code = BG_RED;		break;
	case 'W':	code = BG_WHITE;	break;
	case 'Y':	code = BG_YELLOW;	break;
	case 'D':	code = BG_BLACK;	break;
	default:				break;
	}

    switch ( type )
    {
    case '{':	code = "{";		break;
    case ';':	code = ";";		break;
    case '-':	code = "~";		break;
    default:				break;
    }

    if ( !code )
	code = MOD_CLEAR;

    for ( p = code; *p != '\0'; *string++ = *p++ )
	;
	
    *string = '\0';

    return ( p - code );
}

void colourconv( char *buffer, const char *txt, CHAR_DATA *ch )
{
    const char  *point;

    if ( !txt )
	return;

    for ( point = txt; *point; point++ )
    {
       if ( *point == '{' )
       {
    	    buffer += colour( *++point, ch, buffer );
    	    continue;
       }
       *buffer = *point;
       buffer++;
    }

    *buffer = '\0';
    return;
}



/*
 * Win32 support functions.
 */
#if defined( _MSC_VER )
int gettimeofday( struct timeval *tp, void *tzp )
{
    DWORD         msec = GetTickCount( );

    tp->tv_sec  = (msec / 1000);
    tp->tv_usec = (msec % 1000) * 1000;

    return 0;
}
#endif
