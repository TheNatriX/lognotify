#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


/* TODO: this struct must disappear from here */
struct  logfile
{
        int     wd;             /*      watch descriptor        */
        off_t   offset;         /*      last offset address     */
        char    name[256];      /*      name of log file        */

} *j;

struct logfile* wait_for_changes( void );

int diff_daemon( void )
{
	struct stat ss;
	int fd;
	int c;
	ssize_t recv_len;
	char buff[1024];
	char *super_buff[10];

	for(;;) {

		j = wait_for_changes();
		if( j )	{

			/* get current size */
			if( stat( j->name, &ss ) == -1 ) {
				fprintf( stderr, "Cannot stat file \"%s", j->name );
				perror( "\"" );
				continue;
			}

			/* if the size is not the same, read content from file */
			/* TODO: handle conditions when logs were truncated */
			if( j->offset == ss.st_size )
				continue;

			fd = open( j->name, O_RDONLY );
			lseek( fd, j->offset, SEEK_SET );

			while( ( recv_len = read( fd, buff,
				sizeof( buff ) ) ) > 1 ) {

				write( 1, buff, recv_len );
 
			}
			close( fd );

//			printf( "FILE: %s;\tLAST SIZE: %u;\tCURRENT SIZE: %u\n",
//				j->name, j->offset, ss.st_size );



			/* save the current size */
			j->offset = ss.st_size;
		}
	}
	
	return 0;
}


