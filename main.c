#include <stdio.h>

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

	watch( "/home/natrix/test" );

	/* test */
	prepare_environment();
	argv[argc] = 0;
	draw_on_screen( argv );

	return 0;
}

