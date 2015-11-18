#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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


int watch_files( const char *files[] )
{
	/* count files */
	file_num = 0;
	while( files[file_num] )
		file_num++;

	ifiles = (struct ifile*) malloc( sizeof( struct ifile ) * file_num );

	inotify_fd = inotify_init();
	if( inotify_fd == -1 ) {
		/* TODO */
	}

	file_num = 0;
	while( files[file_num] ) {
		ifiles[file_num].wd = inotify_add_watch( inotify_fd,
			files[file_num], IN_CLOSE_WRITE );
		if( ifiles[file_num].wd == -1 ) {
			/* TODO */
		}
		strcpy( ifiles[file_num].name, files[file_num] ); 
		file_num++;
	}

	return 0;
}


static	int i,x;
static	size_t recv_len;
static	struct inotify_event *ievent;

char *wait_for_changes( void )
{












/*
	i = 0;
	while( i < recv_len ) {
		ievent = (struct inotify_event*) &ibuffer[i];

		for( x = 0; x < file_num; x++ ) {
			if( ievent->wd == ifiles[x].wd )
				return ifiles[x].name;
		}

		i += EVENT_SIZE + ievent->len;		
	}
	recv_len = read( inotify_fd, ibuffer, sizeof( ibuffer ) );
*/
	return 0;
}

