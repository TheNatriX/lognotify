#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/inotify.h>
#include <sys/types.h>


#define EVENT_SIZE	( sizeof(struct inotify_event) )
#define EVENT_BUF_LEN	( 1024  *  ( EVENT_SIZE + 16 ) )


static	int	file_num;
static	int	inotify_fd;
static	char	ibuffer[EVENT_BUF_LEN];


static	struct	ifile
{
	char	name[256];	/*	name of log file	*/
	int	wd;		/*	watch descriptor	*/

} *ifiles;


/*
 * this function initiate inotify and add files to
 * inotify instance in order to watch those files.
 */
int watch_files( const char *files[] )
{
	/* count files */
	file_num = 0;
	while( files[file_num] )
		file_num++;

	/*
	 * alloc memory for counted files, no need to be freed soon.
	 */
	ifiles = (struct ifile*) malloc( sizeof( struct ifile ) * file_num );
	if( !ifiles ) {
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
	 * add files to inotify instance.
	 */
	file_num = 0;
	while( files[file_num] ) {

		ifiles[file_num].wd = inotify_add_watch( inotify_fd,
			files[file_num], IN_CLOSE_WRITE );

		if( ifiles[file_num].wd == -1 ) {
			fprintf( stderr, "Cannot watch \"%s",
				files[file_num] );
			perror( "\"" );
			return 0;
		}

		/* TODO: maybe char name[256] must be dyn. allocated */
		strncpy( ifiles[file_num].name, files[file_num],
			sizeof( ifiles[file_num].name ) );

		file_num++;
	}

	return 1;
}


/*
 * returns the name of file just modified, NULL on error.
 * otherwise it hangs, waiting for file events.
 */
char *wait_for_changes( char *s, size_t sz )
{
	static int i = 0;
	static ssize_t recv_len = 0;
	static struct inotify_event *ievent;
	int x;

	if( i < recv_len ) {
		ievent = (struct inotify_event*) &ibuffer[i];
		i += EVENT_SIZE + ievent->len;
		/* TODO: here can be a single FOR loop */
		if( ievent->name[0] ) {
			for( x = 0; x < file_num; x++ ) {
				if( ifiles[x].wd == ievent->wd ) {
					snprintf( s, sz, "%s/%s",
						ifiles[x].name,	ievent->name );
					ievent->name[0] = 0;
				}
			}
		}
		else {
			for( x = 0; x < file_num; x++ )
				if( ievent->wd == ifiles[x].wd )
					 strncpy( s, ifiles[x].name, sz );
		}

		return s;
	}

	/* wait for events */
	recv_len = read( inotify_fd, ibuffer, sizeof( ibuffer ) );
	if( recv_len == -1 ) {
		perror( "Cannot read from inotify file descriptor" );
		return NULL;
	}
	i = 0;

	/*
	 * instead of returning nothing here, we can just call again
	 * this function and return the file name.
	 */
	return wait_for_changes( s, sz );
}

