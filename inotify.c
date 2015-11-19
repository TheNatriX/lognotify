#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/inotify.h>
#include <sys/types.h>


#define EVENT_SIZE	( sizeof(struct inotify_event) )
#define EVENT_BUF_LEN	( 1024 * ( EVENT_SIZE + 16 ) )


static	int inotify_fd;
static	char ibuffer[EVENT_BUF_LEN];
static	int file_num;


static	struct ifile {
	char name[256];
	int wd;
}	*ifiles;


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
	 * alloc memory for enough files, no need to be freed soon.
	 */
	ifiles = (struct ifile*) malloc( sizeof( struct ifile ) * file_num );
	if( !ifiles ) {
		perror( "Cannot allocate memory" );
		return 0;
	}

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
			perror( "Cannot add inotify watch" );
			return 0;
		}
		strcpy( ifiles[file_num].name, files[file_num] ); 
		file_num++;
	}

	return 1;
}


/*
 * returns the name of file just modified, NULL on error.
 * otherwise it hangs, waiting for file events.
 */
char *wait_for_changes( char *s )
{
	static int i = 0;
	static ssize_t recv_len = 0;
	static struct inotify_event *ievent;
	int x;

	if( i < recv_len ) {
		ievent = (struct inotify_event*) &ibuffer[i];
		i += EVENT_SIZE + ievent->len;
		if( ievent->name ) {
			for( x = 0; x < file_num; x++ ) {
				if( ifiles[x].wd == ievent->wd ) {
					if( s ) {
						/* TODO: secure here*/
						sprintf( s, "%s/%s", ifiles[x].name, ievent->name );
					}
					memset( ievent, 0, EVENT_SIZE );
				}
			}
		}
		else {
			for( x = 0; x < file_num; x++ )
				if( ievent->wd == ifiles[x].wd ) {
					 //ifiles[x].name;
				}
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
	 * this functon and return the file name.
	 */
	return wait_for_changes( s );
}

