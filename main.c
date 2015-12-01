#include <stdio.h>
#include <libgen.h>
#include <unistd.h>

#define VERSION		"v0.0"


int daemon_main( void );
int watch_files( const char *files[] );

const char *files[] = { "/home/natrix/test", "/home/natrix/test2",
		"/var/log/syslog", 0 };

void print_help( const char *this )
{
	printf(
		"\n"
		"\n"
		"	lognotify	%s	%s %s\n"
		"\n",
		VERSION, __DATE__, __TIME__
	);	
}
int main( int argc, char *argv[] )
{
	print_help( (const char*) basename( argv[0] ) );

	daemon( 1, 1 );
	daemon_main();

	return 0;
}

