#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


/* TODO: this struct must disappear from here */
struct  logfile
{
        int     wd;             /*      watch descriptor        */
        size_t	size;           /*      last known size		*/
        char    name[256];      /*      name of log file        */

} *j;

struct logfile* wait_for_changes( void );
int draw_on_screen( char *content );
int prepare_environment( void );

int diff_daemon( void )
{
	struct stat ss;
	int fd;
	ssize_t recv_len;
	char *buf;


prepare_environment();

	for(;;) {

		j = wait_for_changes();
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
		
			buf = malloc( ss.st_size - j->size + 1 );

			fd = open( j->name, O_RDONLY );
			lseek( fd, j->size, SEEK_SET );

			recv_len = read( fd, buf, ss.st_size - j->size );
		       
			if( recv_len > 0 ) {
				*(buf + recv_len) = 0;
				//puts( buf );
				draw_on_screen( buf );
			}

			close( fd );

			/* save the current size */
			j->size = ss.st_size;
			free(buf);
		}
	}
	
	return 0;
}

