#include <sys/inotify.h>
#include <sys/types.h>

#define EVENT_SIZE	( sizeof(struct inotify_event) )
#define EVENT_BUF_LEN	( 1024 * ( EVENT_SIZE + 16 ) )

int watch( const char *path )
{
	char buffer[EVENT_BUF_LEN];
	int i_fd = inotify_init();
	size_t recv_len;
	int i;

	struct inotify_event *event;

	if( i_fd == -1 ) {
	/* TODO */
		perror( "inotify" );
		exit(0);
	}
	
	inotify_add_watch( i_fd, path, IN_ALL_EVENTS );//IN_CLOSE_WRITE );
	recv_len = read( i_fd, buffer, sizeof( buffer ) );

	i = 0;
	while( i < recv_len ) {
		event = (struct inotify_event*) &buffer[i];

		puts( event->name );

		i += EVENT_SIZE + event->len;		
	}

	return 0;
}

