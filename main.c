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

	char *files[] = { "/var/log/messages", "/var/log/syslog", "/home/natrix/test", 0 };
	watch_files( files );
	wait_events();

	/* test */
//	prepare_environment();
//	argv[argc] = 0;
//	draw_on_screen( argv );

	return 0;
}

