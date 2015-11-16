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
static int text_line;

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
		return -1;
	}

	screen_num = DefaultScreen( display );
	res_y = XDisplayHeight( display, screen_num );
	res_x = XDisplayWidth( display, screen_num );
	
	xattr.override_redirect = True;
	rootw = RootWindow( display, screen_num );
	return 0;
}


int draw_window( int x, int y, int rows )
{
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
	XSelectInput( display, w, ExposureMask | ButtonPressMask );
	XMapWindow( display, w );

	return 0;
}

int draw_on_screen( const char **content )
{
	char **content_bkp = content;
	int cnt_len = 0;

	while( *content_bkp ) {
		cnt_len++;
		content_bkp++;
	}


	text_line = 0;
	draw_window( 0, 0, cnt_len );
	while( *content ) {
		text_line += TEXT_ROW_PXL;
		XDrawString( display, w, gc, 10, text_line, *content, strlen( *content ) );
		content++;
	}

	XFlush( display );

        XEvent e;
        while( 1 ) {
                XNextEvent( display, &e );
//              if( e.type == Expose && e.xexpose.count < 1 ) {

//              }
//              else
                if( e.type == ButtonPress ) {
                      XDestroyWindow( display, w ); 
			break;
		}

        }

	return 0;
}

