


/*	LOGNOTIFY VERSION	*/
#define VERSION		"v0.0"



/*	PROTOTYPES		*/
int	daemon_main( void );			/*	daemon.c		*/
int	xc_init( void );			/*	xclient.c		*/
void	xc_handle_events( void );		/*	xclient.c		*/
void	xc_dispatch_to_screen( const char *content );

int	watch_files( const char *files[] );	/*	inotify.c		*/
struct	logfile* read_inotify_events( int ifd );/*	inotify.c		*/


/*	LOG FILE HANDLING	*/
struct	logfile
{
	int	wd;				/*	watch descriptor	*/
	size_t	size;				/*	last known size		*/
	char	name[256];			/*	name of log file	*/

};

