#include <sys/types.h>

extern char *wait_for_changes( char *, size_t );

struct logfile
{
	char	name[256];
	off_t	offset;

} *p_logfile;


int daemon( void )
{
	char buff[1024];

	for(;;) {
		if( wait_for_changes( buff, sizeof( buff ) ) ) {
			puts( buff );
		}
		
	}
	
	return 0;
}


