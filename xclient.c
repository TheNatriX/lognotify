#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/* TODO: these should be replaced by global vars editable from cmd line */
#define TEXT_ROW_PXL		12
#define TEXT_PADDING_X_PXL	2
#define TEXT_COLOR		0xfffffff
#define WINDOW_COLOR		0
#define BORDER_COLOR		0xff
#define BORDER_SIZE_PXL		1
int argv_max_rows =0;
int argv_history = 0;
#define TEXT_Y_PADDING_PXL	5
#define TEXT_X_PADDING_PXL	5



/*	Display related stuff	*/
static	Display	*display;		/*	display handle		*/
static	int	screen_num;		/*	usually :0		*/
static	int	resolution_x;		/*	horisontal axis pixels	*/
static	int	resolution_y;		/*	vertical axis pixels	*/


/*	Window stuff		*/
static	XSetWindowAttributes	wattr;	/*	attr. for our window	*/
static	Window			w;	/*	our window handle	*/
static	GC			gc;	/*	gc for our window	*/
static	Window			rootw;	/*	the root window		*/

static	struct	{
	int	rows;			/*	rows of our window	*/
	int	columns;		/*	columns of our window	*/
	char	**buffer;		/*	window content buffer	*/
}	xc_window;


/*
 * Initiate X connection, allocate window content buffer based on resolution,
 * history lines and font metrics.
 * Returns a file descriptor for X connection from which X events can be read.
 */
int xc_init( void )
{
	int i;
	XFontStruct *fnt_struct;

	/*	open desplay		*/
	display = XOpenDisplay( NULL );
	if( !display ) {
		fprintf( stderr, "Can't open display.\n" );
		return 0;
	}

	/*	display number		*/
	screen_num = DefaultScreen( display );

	/*	get resolution		*/
	resolution_y = XDisplayHeight( display, screen_num );
	resolution_x = XDisplayWidth( display, screen_num );

	/* get metrics for default font	*/
	gc = DefaultGC( display, screen_num );
	fnt_struct = XQueryFont( display, XGContextFromGC( gc )  );
	if( !fnt_struct ) {
		fprintf( stderr, "Can't query font metrics.\n" );
		XCloseDisplay( display );
		return 0;
	}

	/*
	 * calculate rows.
	 * if max rows was not specified, use half of vertical resolution.
	 */
	if( !argv_max_rows ) {
		xc_window.rows =
			( resolution_y / 2
			- 2 * BORDER_SIZE_PXL - 2 * TEXT_Y_PADDING_PXL )
			/ ( fnt_struct->ascent + fnt_struct->descent );

		/* add history lines */
		if( argv_history )
			xc_window.rows += argv_history;
	} else {
		xc_window.rows = argv_max_rows;
		/* TODO check argv_max_rows vs resolution boundaries */
	}
	/*	calculate columns	*/
	xc_window.columns =
		( resolution_x - 2 * BORDER_SIZE_PXL - 2 * TEXT_X_PADDING_PXL )
		/ fnt_struct->max_bounds.width;

	/*	free fnt_struct		*/
	XFreeFontInfo( NULL, fnt_struct, 0 );

	/* now allocate content buffer	*/
	xc_window.buffer = (char**) malloc( xc_window.rows * sizeof( char* ) );
	if( xc_window.buffer == NULL ) {
		perror( "Can't allocate memory" );
		XCloseDisplay( display );
		return 0;
	}
	for( i = 0; i < xc_window.rows; i++ ) {
		*( xc_window.buffer + i ) = (char*) malloc( xc_window.columns );

		if( ( *( xc_window.buffer + i ) ) == NULL ) {
			perror( "Can't allocate memory" );
			XCloseDisplay( display );
			return 0;
		}
	}

	/*
	 * setting override_redirect to true to override handlig of
	 * window manager over our window.
	 */
	wattr.override_redirect = True;

	rootw = RootWindow( display, screen_num );
	return ConnectionNumber( display );
}


int draw_window( int x, int y, int rows )
{
	if( w ) {
		XDestroyWindow( display, w );
		XFlush( display );
		w = 0;
	}
	w = XCreateSimpleWindow( display, rootw, x, y,
		resolution_x - BORDER_SIZE_PXL * 2, rows * TEXT_ROW_PXL + BORDER_SIZE_PXL * 4,
		BORDER_SIZE_PXL, 0xffffffff, 0 );

	XChangeWindowAttributes( display, w, CWOverrideRedirect, &wattr );

/*
	w = XCreateWindow( display, rootw, x, y, resolution_x - BORDER_SIZE_PXL * 2,
		TEXT_ROW_PXL, BORDER_SIZE_PXL, CopyFromParent, InputOutput,
		CopyFromParent, CWOverrideRedirect, &wattr );
*/	
	gc = XCreateGC( display, w, 0, NULL );
	XSetForeground( display, gc, WhitePixel( display, screen_num ) );
	XSelectInput(display, w,/* ExposureMask | KeyPressMask | KeyReleaseMask |
			PointerMotionMask | */ButtonPressMask /*|
			ButtonReleaseMask  | StructureNotifyMask*/ );

	XMapWindow( display, w );
	return 0;
}

int draw_on_screen( char *content )
{
	int txt_lines = 0;
	int one_line;
	int pos_px;

	char *p;
	char **line_ptr;
	char **line_ptr_bkp;

	p = content;
	while( *p ) {
		if( *p == '\n' )
			txt_lines++;
		p++;
	}

	line_ptr = (char**) malloc( txt_lines * sizeof( char* ) + 8 );
	line_ptr_bkp = line_ptr;

	p = content;
	txt_lines = 1;
	*line_ptr = content;
	while( ( p = strstr( p, "\n" ) ) ) { 
		*p = '\0';
		if( ++p ) {
			*(++line_ptr) = p;
			txt_lines++;
		}
		else break;
	}
	line_ptr = line_ptr_bkp;

	draw_window( 0, 0, txt_lines );

	for( one_line = 0, pos_px = 0; one_line < txt_lines;
			one_line++, line_ptr++ ) {
		pos_px += TEXT_ROW_PXL;
		XDrawString( display, w, gc, 1, pos_px,
				*(line_ptr), strlen( *(line_ptr) ) );
	}

	XFlush( display );

	line_ptr = line_ptr_bkp;
	free( line_ptr );

	return 0;
}

void handle_x_events( void ) {
	XEvent xev;
	while( XPending( display ) ) {
		XNextEvent( display, &xev );
		if( xev.type == ButtonPress ) {
			if( w ) {
				XDestroyWindow( display, w );
				XFlush( display );
				w = 0;
			}
		}
	}
}

