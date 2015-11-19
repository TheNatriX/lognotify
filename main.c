#include <stdio.h>
#include <libgen.h>

#define VERSION		"v0.0"

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

	char *files[] = { "/home/natrix/test", "/home/natrix/test2", "/tmp", "nosuchfile", 0 };
	watch_files( files );
	daemon();

	return 0;
}

