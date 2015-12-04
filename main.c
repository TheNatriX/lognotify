#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>

#include "lognotify.h"

#define MAX_WATCH_FILES 256


char *files[MAX_WATCH_FILES];

void print_help( char *this )
{
	printf(
		"\n"
		"\n"
		"LOGNOTIFY %s [%s %s]\n"
		"\n"
		"USAGE:\n"
		"	%s <logfile1> <logfile2> ... <logfile%d>\n"
		"\n",
		VERSION, __DATE__, __TIME__, this, MAX_WATCH_FILES
	);
	exit( EXIT_FAILURE );
}

int main( int argc, char *argv[] )
{
	int i;

	if( ( argc < 2 ) && ( argc < ( MAX_WATCH_FILES + 1 ) ) )
		print_help( basename( argv[0] ) );

	/* dummy files count */
	for( i = 0; i < argc - 1; i++ )
		files[i] = argv[i+1];

	daemon( 1, 1 );
	daemon_main();

	return 0;
}

