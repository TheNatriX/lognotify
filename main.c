#include <stdio.h>
#include <libgen.h>

#define VERSION		"v0.0"


int diff_daemon( void );
int watch_files( const char *files[] );

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

	const char *files[] = { "/home/natrix/test", "/home/natrix/test2",
		"/var/log/messages", 0 };
	watch_files( files );
	diff_daemon();

	return 0;
}

