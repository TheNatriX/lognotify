#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lognotify.h"


extern 	int argv_max_cols;
extern	int argv_max_rows;
extern	int argv_history;
extern	int argv_text_padding_px;
extern	int argv_text_color;
extern	int argv_window_color;
extern	int argv_x_position;
extern	int argv_y_position;
extern	int argv_row_spacer_px;
extern	int argv_border_px;
extern	int argv_border_color;
extern	int argv_verbose;


/*	Display related stuff		*/
static	Display	*display;		/*	display handle		*/
static	int	screen_num;		/*	usually :0		*/
static	int	resolution_x;		/*	horisontal axis pixels	*/
static	int	resolution_y;		/*	vertical axis pixels	*/


/*	X11 windowing stuff		*/
static	XSetWindowAttributes	wattr;	/*	attr. for our window	*/
static	Window			w;	/*	our window handle	*/
static	GC			gc;	/*	gc for our window	*/
static	Window			rootw;	/*	the root window		*/
static	XFontStruct		*font_metrics;


/*	Super Buffer contains all content to be displayed, it's size	*/
/*	in rows and columns, including history lines			*/
static	struct	{
	unsigned int	rows;		/*	rows of super buffer	*/
	unsigned int	cols;		/*	cols of super buffer	*/
	unsigned int	cursor;		/*	last modified line	*/
	char	**	content;	/*	the text matrix		*/
}	xc_super_buffer;


/*	Window View is the current view of Super Buffer and contains	*/
/*	only an array of pointers to Super Buffer and window size	*/
/*	in rows and columns		*/
static	struct	{
	unsigned int	rows;		/*	rows of our window	*/
	unsigned int	cols;		/*	columns of our window	*/
	char	**	pointers;	/*	window's pointer array	*/
}	xc_window_view;


static int xc_scroll_view_counter;	/* used to scroll window text	*/


/*
 * Initiate X connection, allocate Window View and Super Buffer based on
 * resolution, history lines and font metrics.
 * Returns a file descriptor for X connection from which X events can be read.
 */
int xc_init(void)
{
	int i;

	display = XOpenDisplay(NULL);
	if (!display) {
		fprintf(stderr, "Can't open display.\n");
		return 0;
	}
	screen_num = DefaultScreen(display);
	resolution_y = XDisplayHeight(display, screen_num);
	resolution_x = XDisplayWidth(display, screen_num);
	gc = DefaultGC(display, screen_num);
	font_metrics = XQueryFont(display, XGContextFromGC(gc));
	if (!font_metrics) {
		fprintf(stderr, "Can't query font metrics.\n");
		XCloseDisplay(display);
		return 0;
	}
	/* if max rows was not specified, use half of vertical resolution */
	if (!argv_max_rows) {
		xc_window_view.rows = (resolution_y / 2
			- 2 * argv_border_px - 2 * argv_text_padding_px)
			/ (font_metrics->ascent + font_metrics->descent +
			argv_row_spacer_px);

	} else {
		xc_window_view.rows = argv_max_rows;
	}
	if (!argv_max_cols) {
		xc_window_view.cols =
			(resolution_x - 2 * argv_border_px - 2 *
			argv_text_padding_px) / font_metrics->max_bounds.width;
	} else {
		xc_window_view.cols = argv_max_cols;
	}
	/* Super Buffer size */
	xc_super_buffer.rows = xc_window_view.rows + argv_history;
	xc_super_buffer.cols = xc_window_view.cols;
	xc_super_buffer.content = malloc(sizeof(char*) * xc_super_buffer.rows);
	if (xc_super_buffer.content == NULL) {
		perror("Can't allocate memory");
		XCloseDisplay(display);
		return 0;
	}
	for (i = 0; i < xc_super_buffer.rows; i++) {
		/* +1 for null byte */
		xc_super_buffer.content[i] = malloc(xc_super_buffer.cols + 1);
		if (xc_super_buffer.content[i] == NULL) {
			perror("Can't allocate memory");
			XCloseDisplay(display);
			return 0;
		}
		memset(xc_super_buffer.content[i], 0x00, xc_super_buffer.cols + 1);
	}
	xc_window_view.pointers = malloc(sizeof(char*) * xc_window_view.rows);
	if (xc_window_view.pointers == NULL) {
		XCloseDisplay(display);
		return 0;
	}
	xc_super_buffer.cursor = 0;
	/*
	 * setting override_redirect to true to override handlig of
	 * window manager over our window.
	 */
	wattr.override_redirect = True;
	rootw = RootWindow(display, screen_num);
	return ConnectionNumber(display);
}

static void xc_draw_window(int rows)
{
	if (w) {
		XClearWindow(display, w);
		return;
	}
	w = XCreateSimpleWindow(display, rootw,
		argv_x_position, argv_y_position,
		resolution_x - argv_border_px * 2,
		rows * (font_metrics->ascent + font_metrics->descent +
		argv_row_spacer_px) + argv_border_px * 2 + argv_text_padding_px,
		argv_border_px, argv_border_color, argv_window_color);

	XChangeWindowAttributes(display, w, CWOverrideRedirect, &wattr);
	gc = XCreateGC(display, w, 0, NULL);
	XSetForeground(display, gc, WhitePixel(display, screen_num));
	XSelectInput(display, w, ButtonPressMask);
	XMapWindow(display, w);
}

static void xc_write_on_window(const int rows)
{
	/* just a TEST; need implemenntation */
	int c;
	for (c = 0; c < rows; c++) {
		XDrawString(display, w, gc, argv_border_px + argv_text_padding_px,
				(c + 1) * (font_metrics->ascent +
					font_metrics->descent + argv_row_spacer_px),
				xc_window_view.pointers[c],
				strlen(xc_window_view.pointers[c]));
	}
}

static void xc_scroll_buffer_up(unsigned int lines)
{
	/*
	 * instead of copying characters to upper lines
	 * its better to move only line pointers in a circular way
	 */
	int c;
	char *p;
	register int x;

	for (c = 0; c < lines; c++) {
		p = xc_super_buffer.content[0];
		for (x = 0; x < xc_super_buffer.rows - 1; x++)
			xc_super_buffer.content[x] = xc_super_buffer.content[x + 1];
		xc_super_buffer.content[x] = p;
	}
}

static void xc_bind_view(int buffer_row)
{
	int row;
	if (buffer_row >= xc_window_view.rows)
		buffer_row -= xc_window_view.rows;
	else buffer_row = 0;
	for (row = 0; row < xc_window_view.rows; row++, buffer_row++)
		xc_window_view.pointers[row] = xc_super_buffer.content[buffer_row];
}

static int xc_scroll_view_up(void)
{
	if(xc_super_buffer.cursor <= xc_window_view.rows)
		return 0;

	xc_scroll_view_counter++;
	if (xc_scroll_view_counter > xc_super_buffer.cursor - xc_window_view.rows) {
		xc_scroll_view_counter =
			xc_super_buffer.cursor - xc_window_view.rows;
		return 0;
	} else if (xc_scroll_view_counter < 0) {
		xc_scroll_view_counter = 0;
		return 0;
	}

/* TRACE */
#ifdef DEBUG
	fprintf(stderr, "xc_scroll_view_up(): xc_scroll_view_counter == %d\n",
			xc_scroll_view_counter);
#endif
	xc_bind_view(xc_super_buffer.cursor - xc_scroll_view_counter);
	return 1;
}

static int xc_scroll_view_down(void)
{
	xc_scroll_view_counter--;
	if (xc_scroll_view_counter > argv_history) {
		xc_scroll_view_counter = argv_history;
		return 0;
	} else if (xc_scroll_view_counter < 0) {
		xc_scroll_view_counter = 0;
		return 0;
	}

/* TRACE */
#ifdef DEBUG
	fprintf(stderr, "xc_scroll_view_down(): xc_scroll_view_counter == %d\n",
			xc_scroll_view_counter);
#endif
	xc_bind_view(xc_super_buffer.cursor - xc_scroll_view_counter);
	return 1;
}

static unsigned int xc_count_rows(const char *content)
{
	unsigned int rows = 0;
	register unsigned int cols = 0;

	while (*content) {
		/* line too long */
		if (cols == xc_super_buffer.cols) {
			cols = 0;
			rows++;
		}
		/* end of line */
		if (*content == '\n') {
			cols = 0;
			rows++;
			content++;
			continue;
		}
		content++;
		cols++;
	}
	/* maybe last line doesn't have '\n' */
	if (*(content - 1) != '\n')
		rows++;
	return rows;
}

static void xc_store_cursor_position(const char *last_modified_row)
{
	register unsigned int pos = 0;
	char *row_ptr = xc_super_buffer.content[pos];
	while (row_ptr != last_modified_row) {
		pos++;
		row_ptr = xc_super_buffer.content[pos];
	}
	xc_super_buffer.cursor = pos;

/* TRACE */
#ifdef DEBUG
	fprintf(stderr, "xc_store_cursor_position(): xc_super_buffer.cursor == %u\n"
			, xc_super_buffer.cursor);
#endif
}

void xc_dispatch_to_screen(const char *content)
{
	unsigned int col;
	unsigned int row;

	/* copy content to buffer */
	col = 0;
	row = xc_super_buffer.cursor;
	while (*content) {
		/* if text line is too big set cursor to the next row */
		if (col > xc_super_buffer.cols) {
			xc_super_buffer.content[row][col] = '\0';
			col = 0;
			row++;
		}
		/* scroll up */
		if (row == xc_super_buffer.rows) {
			col = 0;
			row--;
			xc_scroll_buffer_up(1);
		}
		/* new line */
		if (*content == '\n') {
			xc_super_buffer.content[row][col] = '\0';
			row++;
			col = 0;
			content++;
			continue;
		}
		/* copy one char */
		xc_super_buffer.content[row][col] = *content;
		col++;
		content++;
	}
	/* maybe last line doesn't have '\n' */
	if (*(content - 1) != '\n') {
		xc_super_buffer.content[row][col] = '\0';
		row++;
	}

	/* TEST */
	xc_store_cursor_position(xc_super_buffer.content[row]);
	xc_bind_view(xc_super_buffer.cursor);
	xc_scroll_view_counter = 0;
	xc_draw_window(xc_window_view.rows);
	xc_write_on_window(xc_window_view.rows);
	XFlush(display);
}

void xc_handle_events(void)
{
	XEvent xev;
	while (XPending(display)) {
		XNextEvent(display, &xev);
		if (xev.type == ButtonPress) {
			switch (xev.xbutton.button) {
			case Button4:
				if (xc_scroll_view_up()) {
					XClearWindow(display, w);
					xc_write_on_window(xc_window_view.rows);
					XFlush(display);
				}
				break;
			case Button5:
				if (xc_scroll_view_down()) {
					XClearWindow(display, w);
					xc_write_on_window(xc_window_view.rows);
					XFlush(display);
				}
				break;
			default:
				if (w) {
					XDestroyWindow(display, w);
					XFlush(display);
					w = 0;
				}
			}
		}
	}
}

