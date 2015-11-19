#include <stdio.h>
#include <libgen.h>

#define VERSION		"v0.0"

char *wait_for_changes( char *s );

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
	char buff[1024];
	print_help( (const char*) basename( argv[0] ) );

	char *name;
	char *files[] = { "/home/natrix/test", "/home/natrix/test2", "/tmp", "nosuchfile", 0 };
	watch_files( files );
	for(;;) {
		puts( wait_for_changes( buff ) );
	}

	/* test */
//	prepare_environment();
//	argv[argc] = 0;
//	draw_on_screen( argv );

	return 0;
}

