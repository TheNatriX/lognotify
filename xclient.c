#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lognotify.h"

/* TODO: these should be replaced by global vars editable from cmd line */
#define TEXT_ROW_PXL		12
#define TEXT_PADDING_X_PXL	2
#define TEXT_COLOR		0xfffffff
#define WINDOW_COLOR		0
#define BORDER_COLOR		0xff
#define BORDER_SIZE_PXL		1
int argv_max_rows =0;
int argv_history = 100;
#define TEXT_Y_PADDING_PXL	5
#define TEXT_X_PADDING_PXL	5



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


static int xc_scroll_view_counter;	/*	used to scroll window text	*/


/*
 * Initiate X connection, allocate Window View and Super Buffer based on
 * resolution, history lines and font metrics.
 * Returns a file descriptor for X connection from which X events can be read.
 */
int xc_init(void)
{
	int i;
	XFontStruct *fnt_struct;

	/*	open desplay		*/
	display = XOpenDisplay(NULL);
	if (!display) {
		fprintf(stderr, "Can't open display.\n");
		return 0;
	}

	/*	display number		*/
	screen_num = DefaultScreen(display);

	/*	get resolution		*/
	resolution_y = XDisplayHeight(display, screen_num);
	resolution_x = XDisplayWidth(display, screen_num);

	/* get metrics for default font	*/
	gc = DefaultGC(display, screen_num);
	fnt_struct = XQueryFont(display, XGContextFromGC(gc));
	if (!fnt_struct) {
		fprintf(stderr, "Can't query font metrics.\n");
		XCloseDisplay(display);
		return 0;
	}

	/*
	 * calculate window rows.
	 * if max rows was not specified, use half of vertical resolution.
	 */
	if (!argv_max_rows) {
		xc_window_view.rows = (resolution_y / 2
			- 2 * BORDER_SIZE_PXL - 2 * TEXT_Y_PADDING_PXL)
			/ (fnt_struct->ascent + fnt_struct->descent);

	} else {
		xc_window_view.rows = argv_max_rows;
		/* TODO check argv_max_rows vs resolution boundaries */
	}

	/*	calculate columns	*/
	xc_window_view.cols =
		(resolution_x - 2 * BORDER_SIZE_PXL - 2 * TEXT_X_PADDING_PXL)
		/ fnt_struct->max_bounds.width;

	/*	don't need metrics anymore	*/
	XFreeFontInfo(NULL, fnt_struct, 0);

	/*	Super Buffer size	*/
	xc_super_buffer.rows = xc_window_view.rows + argv_history;
	xc_super_buffer.cols = xc_window_view.cols;

	/* now allocate memory for Super Buffer	*/
	xc_super_buffer.content = malloc(sizeof(char*) * xc_super_buffer.rows);
	if (xc_super_buffer.content == NULL) {
		perror("Can't allocate memory");
		XCloseDisplay(display);
		return 0;
	}
	for (i = 0; i < xc_super_buffer.rows; i++) {
		/*	+1 for null byte	*/
		xc_super_buffer.content[i] = malloc(xc_super_buffer.cols + 1);
		if (xc_super_buffer.content[i] == NULL) {
			perror("Can't allocate memory");
			XCloseDisplay(display);
			return 0;
		}
	}

	/*	allocate memory for window pointers	*/
	xc_window_view.pointers = malloc(sizeof(char*) * xc_window_view.rows);
	if (xc_window_view.pointers == NULL) {
		XCloseDisplay(display);
		return 0;
	}

	/*	buffer is empty now	*/
	xc_super_buffer.cursor = 0;

	/*
	 * setting override_redirect to true to override handlig of
	 * window manager over our window.
	 */
	wattr.override_redirect = True;

	rootw = RootWindow(display, screen_num);
	return ConnectionNumber(display);
}

/* TODO: reimplement this function */
int draw_window(int x, int y, int rows)
{
	if (w) {
		/*XDestroyWindow(display, w);
		XFlush(display);
		w = 0;*/
		XClearWindow(display, w);
		return 0;
	}
	w = XCreateSimpleWindow(display, rootw, x, y,
		resolution_x - BORDER_SIZE_PXL * 2, rows * TEXT_ROW_PXL + BORDER_SIZE_PXL * 4,
		BORDER_SIZE_PXL, 0xffffffff, 0);

	XChangeWindowAttributes(display, w, CWOverrideRedirect, &wattr);

/*
	w = XCreateWindow( display, rootw, x, y, resolution_x - BORDER_SIZE_PXL * 2,
		TEXT_ROW_PXL, BORDER_SIZE_PXL, CopyFromParent, InputOutput,
		CopyFromParent, CWOverrideRedirect, &wattr );
*/	
	gc = XCreateGC(display, w, 0, NULL);
	XSetForeground(display, gc, WhitePixel(display, screen_num));
	XSelectInput(display, w,/* ExposureMask | KeyPressMask | KeyReleaseMask |
			PointerMotionMask | */ButtonPressMask /*|
			ButtonReleaseMask  | StructureNotifyMask*/);

	XMapWindow(display, w);
	return 0;
}

void xc_write_on_window(const int rows)
{
	/* just a TEST; need implemenntation */
	int c;
	for (c = 0; c < rows; c++) {
		XDrawString(display, w, gc, 1, (c + 1) * TEXT_ROW_PXL,
				xc_window_view.pointers[c],
				strlen(xc_window_view.pointers[c]));
	}
}

void xc_scroll_buffer_up(unsigned int lines)
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

void xc_bind_view(int buffer_row)
{
	int row;
	if (buffer_row >= xc_window_view.rows)
		buffer_row -= xc_window_view.rows;
	else buffer_row = 0;
	for (row = 0; row < xc_window_view.rows; row++, buffer_row++)
		xc_window_view.pointers[row] = xc_super_buffer.content[buffer_row];
}

int xc_scroll_view_up(void)
{
	if (xc_scroll_view_counter > argv_history) {
		xc_scroll_view_counter = argv_history;
		return 0;
	} else if (xc_scroll_view_counter < 0) {
		xc_scroll_view_counter = 0;
		return 0;
	}
	xc_bind_view(xc_super_buffer.cursor - xc_scroll_view_counter++);
	return 1;
}

int xc_scroll_view_down(void)
{
	if (xc_scroll_view_counter > argv_history) {
		xc_scroll_view_counter = argv_history;
		return 0;
	} else if (xc_scroll_view_counter < 0) {
		xc_scroll_view_counter = 0;
		return 0;
	}
	xc_bind_view(xc_super_buffer.cursor - xc_scroll_view_counter--);
	return 1;
}

unsigned int xc_count_rows(const char *content)
{
	unsigned int rows = 0;
	register unsigned int cols = 0;

	while (*content) {

		/*	line too long	*/
		if (cols == xc_super_buffer.cols) {
			cols = 0;
			rows++;
		}

		/*	end of line	*/
		if (*content == '\n') {
			cols = 0;
			rows++;
			content++;
			continue;
		}

		content++;
		cols++;
	}

	/*	maybe last line doesn't have '\n'	*/
	if (*(content - 1) != '\n')
		rows++;

	return rows;
}

void xc_store_cursor_position(const char *last_modified_row)
{
	register unsigned int pos = 0;
	char *row_ptr = xc_super_buffer.content[pos];

	/*	count rows between first and last	*/
	while (row_ptr != last_modified_row) {
		pos++;
		row_ptr = xc_super_buffer.content[pos];
	}

	/*	store rows count	*/
	xc_super_buffer.cursor = pos;

/* TRACE */
#ifdef DEBUG
	fprintf(stderr, "%s():\tcursor == %u\n",
			__FUNCTION__, xc_super_buffer.cursor);
#endif
}

void xc_dispatch_to_screen(const char *content)
{
	unsigned int col;
	unsigned int row;

	/*	copy content to buffer	*/
	col = 0;
	row = xc_super_buffer.cursor;
	while (*content) {

		/* if text line is too big set cursor to the next row */
		if (col > xc_super_buffer.cols) {
			xc_super_buffer.content[row][col] = '\0';
			col = 0;
			row++;
		}

		/*	scroll up	*/
		if (row == xc_super_buffer.rows) {
			col = 0;
			row--;
			xc_scroll_buffer_up( 1 );
		}

		/*	new line	*/
		if (*content == '\n') {
			xc_super_buffer.content[row][col] = '\0';
			row++;
			col = 0;
			content++;
			continue;
		}

		/*	copy one char	*/
		xc_super_buffer.content[row][col] = *content;

		col++;
		content++;
	}

	/*	maybe last line doesn't have '\n'	*/
	if (*(content - 1) != '\n') {
		xc_super_buffer.content[row][col] = '\0';
		row++;
	}

	/* TEST */
	xc_store_cursor_position(xc_super_buffer.content[row]);
	xc_bind_view(xc_super_buffer.cursor);
	xc_scroll_view_counter = 0;
	draw_window(0, 0, xc_window_view.rows);
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
				xc_scroll_view_up();
				XClearWindow(display, w);
				xc_write_on_window(xc_window_view.rows);
				XFlush(display);
				break;
			case Button5:
				xc_scroll_view_down();
				XClearWindow(display, w);
				xc_write_on_window(xc_window_view.rows);
				XFlush(display);
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

