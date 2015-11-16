#include <sys/inotify.h>

int watch( const char *path )
{
	char buffer[16384];
	int i_fd = inotify_init();
	if( i_fd == -1 ) {
	/* TODO */
		perror( "inotify" );
		exit(0);
	}
	
	inotify_add_watch( i_fd, path, IN_CLOSE_WRITE );
	while( read( i_fd, buffer, sizeof( buffer ) ) ) {
		/* TODO */
		puts("O");

	}

	return 0;
}

