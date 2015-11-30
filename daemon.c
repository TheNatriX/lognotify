#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>


/* TODO: this struct must disappear from here */
struct  logfile
{
        int     wd;             /*      watch descriptor        */
        size_t	size;           /*      last known size		*/
        char    name[256];      /*      name of log file        */

} *j;

struct logfile* read_inotify_events( int ifd );
int draw_on_screen( char *content );
int prepare_environment( void );
int watch_files( const char *files[] );
void handle_x_events( void );

extern const char *files[];

int daemon_main( void )
{
	struct stat ss;
	int file_fd;
	int ifd;
	int xfd;
	ssize_t recv_len;
	char *buf;

	fd_set rfds;
	/* TODO change name of this function */
	xfd = prepare_environment();

	ifd = watch_files( files );

	for(;;) {
		FD_ZERO( &rfds );
		FD_SET( ifd, &rfds );
		FD_SET( xfd, &rfds );


		if( select( ((ifd > xfd) ? ifd : xfd) + 1,
				       &rfds, NULL, NULL, NULL ) == -1 )
			perror( "select" );

		if( FD_ISSET( ifd, &rfds ) ) {
			//puts( "inotify event" );
			j = read_inotify_events( ifd );
		}
		else if( FD_ISSET( xfd, &rfds ) ) {
			//puts( "xorg event" );
			handle_x_events();
		}
		if( j )	{

			/* get the current size */
			if( stat( j->name, &ss ) == -1 ) {
				fprintf( stderr, "Cannot stat file \"%s", j->name );
				perror( "\"" );
				continue;
			}

			/* if the size is not the same, read content from file */
			/* TODO: handle conditions when logs were truncated */
			if( j->size == ss.st_size )
				continue;
		
			buf = malloc( ss.st_size - j->size + 10 );
			/* TODO check */

			file_fd = open( j->name, O_RDONLY );
			/* TODO check */

			lseek( file_fd, j->size, SEEK_SET );
			/* TODO check */

			recv_len = read( file_fd, buf, ss.st_size - j->size );
			/* TODO check */

			if( recv_len > 0 ) {
				/* append null byte at the end of string */
				*(buf + recv_len) = '\0';

				//puts( buf );
				draw_on_screen( buf );
				sleep(2);
			}

			close( file_fd );

			/* save the current size */
			j->size = ss.st_size;
			free(buf);
			j = NULL;
		}
	}
	
	return 0;
}

