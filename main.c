#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <getopt.h>

#define __USE_BSD
#include <unistd.h>

#include "lognotify.h"

#define MAX_WATCH_FILES 256


int argv_max_cols	= 0;
int argv_max_rows	= 0;
int argv_history	= 100;
int argv_text_color	= 0xffffffff;
int argv_window_color	= 0;
int argv_x_position	= 0;
int argv_y_position	= 0;
int argv_border_px	= 1;
int argv_border_color	= 0xff;
int argv_text_padding_px= 5;
int argv_row_spacer_px	= 1;
int argv_verbose	= 0;

char *files[MAX_WATCH_FILES];

void print_help(char *this)
{
	fprintf(stderr,
		"\n"
		"\n"
		"LOGNOTIFY %s [%s %s]\n"
		"\n"
		"USAGE:\n"
		"	%s <options> <logfile1> ... <logfileN>\n"
		"\n"
		"OPTIONS:\n"
		"	-c, --cols	 <N>	Use N columns. Def: auto\n"
		"	-r, --rows	 <N>	Use N rows. Def: auto\n"
		"	-s, --history	 <N>	History lines. Def: 100\n"
		"	-t, --text-color <X>	Hex of text color. Def: ffffffff\n"
		"	-w, --wind-color <X>	Hex of window color. Def: 0\n"
		"	-x, --x-position <N>	Show window from X pixels. Def: 0\n"
		"	-y, --y-position <N>	Show window from Y pixels. Def: 0\n"
		"	-b, --border-px	 <N>	Border size in pixels. Def: 1\n"
		"	--border-color	 <X>	Hex of border color. Def: ff\n"
		"	--text-padding	 <N>	Text padding from border. Def: 5\n"
		"	-v, --verbose		Print info messages.\n"
		"	-h, --help		Print this help page.\n"
		"\n",
		VERSION, __DATE__, __TIME__, this);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int c;
	int option_index = 0;
	char *me;
	const struct option long_options[] = {
		{"cols",	 required_argument, NULL, 'c'},
		{"rows",	 required_argument, NULL, 'r'},
		{"history",	 required_argument, NULL, 's'},
		{"text-color",	 required_argument, NULL, 't'},
		{"wind-color",	 required_argument, NULL, 'w'},
		{"x-position",	 required_argument, NULL, 'x'},
		{"y-position",	 required_argument, NULL, 'y'},
		{"border-px",	 required_argument, NULL, 'b'},
		{"verbose",	 no_argument,       NULL, 'v'},
		{"help",	 no_argument,	    NULL, 'h'},
		{"border-color", required_argument, NULL, 1},
		{"text-padding", required_argument, NULL, 2},
		{0, 0, 0, 0}
	};

	me = basename(argv[0]);
	while ((c = getopt_long(argc, argv, "c:r:s:w:x:y:b:1:2:3:vh",
			long_options, &option_index)) != -1) {
		switch (c) {
		case 'c':
			argv_max_cols = atoi(optarg);
			if (argv_max_cols < 0) {
				fprintf(stderr, "Column number cannot be negative"
						"\n\n\n");
				print_help(me);
			}
			break;
		case 'r':
			argv_max_rows = atoi(optarg);
			if (argv_max_rows < 0) {
				fprintf(stderr, "Row number cannot be negative"
						"\n\n\n");
				print_help(me);
			}
			break;
		case 's':
			argv_history = atoi(optarg);
			if (argv_history < 0) {
				fprintf(stderr, "History lines cannot be negative"
						"\n\n\n");
				print_help(me);
			}
			break;
		case 't':
			argv_text_color = atoi(optarg);
			if (argv_text_color < 0) {
				fprintf(stderr, "Text color cannot be negative"
						"\n\n\n");
				print_help(me);
			}
			break;
		case 'w':
			break;
		case 'x':
			argv_x_position = atoi(optarg);
			if (argv_x_position < 0) {
				fprintf(stderr, "X position cannot be negative"
						"\n\n\n");
				print_help(me);
			}
			break;
		case 'y':
			argv_y_position = atoi(optarg);
			if (argv_y_position < 0) {
				fprintf(stderr, "Y position cannot be negative"
						"\n\n\n");
				print_help(me);
			}

			break;
		case 'b':
			
			break;
		case 'v':
			argv_verbose = 1;
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		default:
			print_help(me);
		}
	}

	files[0] = argv[argc - 1];
#ifndef DEBUG
	daemon(1, 1);
#endif
	daemon_main();
	return 0;
}

