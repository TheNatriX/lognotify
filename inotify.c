#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>


#define EVENT_SIZE	( sizeof(struct inotify_event) )
#define EVENT_BUF_LEN	( 1024  *  ( EVENT_SIZE + 16 ) )


static	int	file_num;
static	int	inotify_fd;
static	char	ibuffer[EVENT_BUF_LEN];


/* TODO: maybe this struct must be placed in a general header. */
struct	logfile
{
	int	wd;		/*	watch descriptor	*/
	off_t	offset;		/*	last offset address	*/
	char	name[256];	/*	name of log file	*/

} *p_logfile;


/*
 * this function initiate inotify and add files to
 * inotify instance in order to watch those files.
 */
int watch_files( const char *files[] )
{
	struct stat ss;

	/* count files */
	file_num = 0;
	while( files[file_num] )
		file_num++;

	/*
	 * allocate memory for counted files, no need to be freed soon.
	 */
	p_logfile = (struct logfile*) malloc(
		sizeof( struct logfile ) * file_num );

	if( !p_logfile ) {
		perror( "Cannot allocate memory" );
		return 0;
	}

	/*
	 * initiate inotify.
	 */
	inotify_fd = inotify_init();
	if( inotify_fd == -1 ) {
		perror( "Cannot initiate inotify" );
		return 0;
	}

	/*
	 * add files to inotify instance and get its size as an offset address.
	 */
	file_num = 0;
	while( files[file_num] ) {

		p_logfile[file_num].wd = inotify_add_watch( inotify_fd,
			files[file_num], IN_CLOSE_WRITE );

		if( p_logfile[file_num].wd == -1 ) {
			fprintf( stderr, "Cannot watch \"%s",
				files[file_num] );
			perror( "\"" );
			return 0;
		}

		/* get its size */
		if( stat( files[file_num], &ss ) == -1 ) {
			fprintf( stderr, "Cannot stat file \"%s",
				files[file_num] );
			perror( "\"" );
			return 0;
		}
		p_logfile[file_num].offset = ss.st_size;


		/* TODO: maybe char name[256] must be dyn. allocated */
		strncpy( p_logfile[file_num].name, files[file_num],
			sizeof( p_logfile[file_num].name ) );

		file_num++;
	}

	return 1;
}


/*
 * returns a pointer to the right struct of file just modified, NULL on error.
 * otherwise it hangs, waiting for file events.
 */
struct logfile* wait_for_changes( void )
{
	static int i = 0;
	static ssize_t recv_len = 0;
	static struct inotify_event *ievent;
	int x;

	if( i < recv_len ) {
		ievent = (struct inotify_event*) &ibuffer[i];
		i += EVENT_SIZE + ievent->len;

		/*
		 * check against each watch descriptor to find out
		 * what file was modified.
		 *
		 * return a pointer to the mallocated structure.
		 */
		for( x = 0; x < file_num; x++ )
			if( ievent->wd == p_logfile[x].wd )
				return &p_logfile[x];
	}

	/* wait for events */
	recv_len = read( inotify_fd, ibuffer, sizeof( ibuffer ) );
	if( recv_len == -1 ) {
		perror( "Cannot read from inotify file descriptor" );
		return 0;
	}
	i = 0;

	/*
	 * instead of returning nothing here, we can just call again
	 * this function and return a valid response.
	 */
	return wait_for_changes();
}

