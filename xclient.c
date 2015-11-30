#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define TEXT_ROW_PXL		12
#define BORDER_PXL		1
#define TEXT_PADDING_X_PXL	2
#define TEXT_COLOR		0xfffffff
#define WINDOW_COLOR		0
#define BORDER_COLOR		0xff
#define BORDER_PXL		1

static int res_x;
static int res_y;

static Display *display;
static int screen_num;

static XSetWindowAttributes xattr;
static Window rootw;
static Window w;
static GC gc;

int prepare_environment( void )
{

	display = XOpenDisplay( NULL );
	if( !display ) {
		fprintf( stderr, "Can't open display.\n" );
		return 0;
	}

	screen_num = DefaultScreen( display );
	res_y = XDisplayHeight( display, screen_num );
	res_x = XDisplayWidth( display, screen_num );
	
	xattr.override_redirect = True;
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
		res_x - BORDER_PXL * 2, rows * TEXT_ROW_PXL + BORDER_PXL * 4,
		BORDER_PXL, 0xffffffff, 0 );

	XChangeWindowAttributes( display, w, CWOverrideRedirect, &xattr );

/*
	w = XCreateWindow( display, rootw, x, y, res_x - BORDER_PXL * 2,
		TEXT_ROW_PXL, BORDER_PXL, CopyFromParent, InputOutput,
		CopyFromParent, CWOverrideRedirect, &xattr );
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

	line_ptr = malloc( txt_lines * sizeof( char * ) + 10 );
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
	}
	line_ptr = line_ptr_bkp;

	draw_window( 0, 0, txt_lines );

	for( one_line = 0, pos_px = 0; one_line < txt_lines;
			one_line++, line_ptr++ ) {
		pos_px += TEXT_ROW_PXL;
		XDrawString( display, w, gc, 10, pos_px,
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

