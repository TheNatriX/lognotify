#include <sys/types.h>
#include <stdio.h>


struct  logfile
{
        int     wd;             /*      watch descriptor        */
        off_t   offset;         /*      last offset address     */
        char    name[256];      /*      name of log file        */

} *j;

struct logfile * wait_for_changes( void );

int daemon( void )
{
	for(;;) {
	j = wait_for_changes();
	if( j )
		puts( j->name );
	}
	
	return 0;
}


