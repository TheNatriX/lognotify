#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>

#include "lognotify.h"

char *files[256];

void print_help( char *this )
{
	fprintf( stderr,
		"\n"
		"\n"
		"LOGNOTIFY %s [%s %s]\n"
		"\n"
		"USAGE:\n"
		"	%s <logfile1> <logfile2> ... <logfileN>\n"
		"\n",
		VERSION, __DATE__, __TIME__, this
	);
}

int main( int argc, char *argv[] )
{
	int i;

	if( argc < 2 ) {
		print_help( basename( argv[0] ) );
		exit( EXIT_FAILURE );
	}

	/* dummy files count */
	for( i = 0; i < argc - 1; i++ ) {
		files[i] = argv[i+1];
	}

//	daemon( 1, 1 );
	daemon_main();

	return 0;
}

