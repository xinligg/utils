#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <png.h>
#include <string.h>
#include <errno.h>
#include <Imlib2.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

struct screen
{
  Display *display;
  Screen *screen;
  Visual *visual;
  Colormap colormap;
  Window root;
};

void
screen_init (struct screen *myscreen)
{
  myscreen->display = XOpenDisplay (NULL);
  if (!myscreen->display)
    {
      perror ("XOpenDisplay");
      exit (EXIT_FAILURE);
    }
  myscreen->screen = ScreenOfDisplay (myscreen->display,
				      DefaultScreen (myscreen->display));
  myscreen->visual = DefaultVisual (myscreen->display,
				    DefaultScreen (myscreen->display));
  myscreen->colormap = DefaultColormap (myscreen->display,
					DefaultScreen (myscreen->display));
  myscreen->root = RootWindow (myscreen->display,
			       DefaultScreen (myscreen->display));
  imlib_context_set_display (myscreen->display);
  imlib_context_set_visual (myscreen->visual);
  imlib_context_set_colormap (myscreen->colormap);
}

Window
create_full_window (Display * display)
{
  int screen = DefaultScreen (display);
  int screen_num = DefaultScreen (display);
  int display_width = DisplayWidth (display, screen_num);
  int display_height = DisplayHeight (display, screen_num);

  Window win;
  unsigned long windowMask;
  XSetWindowAttributes winAttrib;

  windowMask = CWBackPixel | CWBorderPixel;
  winAttrib.border_pixel = BlackPixel (display, screen);
  winAttrib.background_pixel = BlackPixel (display, screen);
  winAttrib.override_redirect = 0;

  win = XCreateWindow (display, DefaultRootWindow (display),
		       0, 0,
		       display_width, display_height,
		       0, DefaultDepth (display, screen),
		       InputOutput, CopyFromParent, windowMask, &winAttrib);

  Atom wm_state = XInternAtom (display, "_NET_WM_STATE", True);
  Atom wm_fullscreen =
    XInternAtom (display, "_NET_WM_STATE_FULLSCREEN", True);

  XChangeProperty (display, win, wm_state, XA_ATOM, 32,
		   PropModeReplace, (unsigned char *) &wm_fullscreen, 1);

//  XMoveWindow (display, win, 0, 0);

  return win;
}


Window
create_simple_window (Display * display, int width, int height, int x, int y)
{

  int screen_num = DefaultScreen (display);

  int win_border_width = 0;

  Window win;


  win = XCreateSimpleWindow (display, RootWindow (display, screen_num),
			     x, y, width, height, win_border_width,
			     BlackPixel (display, screen_num),
			     WhitePixel (display, screen_num));

  /* make the window actually appear on the screen. */

  XMapWindow (display, win);

  /* flush all pending requests to the X server. */

  XFlush (display);

  return win;

}

GC
create_gc (Display * display, Window win, int reverse_video)
{

  GC gc;			/* handle of newly created GC.  */

  unsigned long valuemask = 0;	/* which values in 'values' to  */

  /* check when creating the GC.  */

  XGCValues values;		/* initial values for the GC.   */

  unsigned int line_width = 2;	/* line width for the GC.       */

  int line_style = LineSolid;	/* style for lines drawing and  */

  int cap_style = CapButt;	/* style of the line's edje and */

  int join_style = JoinBevel;	/*  joined lines.       */

  int screen_num = DefaultScreen (display);

  gc = XCreateGC (display, win, valuemask, &values);

  if (gc < 0)
    {

      fprintf (stderr, "XCreateGC: \n");

    }

  /* allocate foreground and background colors for this GC. */

  if (reverse_video)
    {

      XSetForeground (display, gc, WhitePixel (display, screen_num));

      XSetBackground (display, gc, BlackPixel (display, screen_num));

    }

  else
    {

      XSetForeground (display, gc, BlackPixel (display, screen_num));

      XSetBackground (display, gc, WhitePixel (display, screen_num));

    }


  XSetLineAttributes (display, gc, line_width, line_style, cap_style,
		      join_style);


  XSetFillStyle (display, gc, FillSolid);

  return gc;

}

static void
TeardownPng (png_structp png, png_infop info)
{

  if (png)
    {

      png_infop *realInfo = (info ? &info : NULL);

      png_destroy_read_struct (&png, realInfo, NULL);

    }

}

void
LoadPng (FILE * file, unsigned char **data, char **clipData,
	 unsigned int *width, unsigned int *height, unsigned int *rowbytes)
{

  size_t size = 0, clipSize = 0;

  png_structp png = NULL;

  png_infop info = NULL;

  unsigned char **rowPointers = NULL;

  int depth = 0, colortype = 0, interlace = 0, compression = 0, filter = 0;

  unsigned clipRowbytes = 0;


  png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  info = png_create_info_struct (png);

  png_init_io (png, file);

  png_read_info (png, info);

  png_get_IHDR (png, info, (png_uint_32 *) width, (png_uint_32 *) height,
		&depth, &colortype, &interlace, &compression, &filter);



  *rowbytes = png_get_rowbytes (png, info);

  if (colortype == PNG_COLOR_TYPE_RGB)
    {

      // X hates 24bit images - pad to RGBA
      png_set_filler (png, 0xff, PNG_FILLER_AFTER);

      *rowbytes = (*rowbytes * 4) / 3;

    }

  png_set_bgr (png);


  *width = png_get_image_width (png, info);

  *height = png_get_image_height (png, info);

  size = *height * *rowbytes;

  clipRowbytes = *rowbytes / 32;

  if (*rowbytes % 32)

    ++clipRowbytes;

  clipSize = clipRowbytes * *height;

  // This gets freed by XDestroyImage

  *data = (unsigned char *) malloc (sizeof (png_byte) * size);

  rowPointers =
    (unsigned char **) malloc (*height * sizeof (unsigned char *));

  png_bytep cursor = *data;

  int i = 0, x = 0, y = 0;

  for (i = 0; i < *height; ++i, cursor += *rowbytes)

    rowPointers[i] = cursor;

  png_read_image (png, rowPointers);

  *clipData = (char *) calloc (clipSize, sizeof (unsigned char));

  if (colortype == PNG_COLOR_TYPE_RGB)
    {

      memset (*clipData, 0xff, clipSize);

    }
  else
    {

      // Set up bitmask for clipping fully transparent areas

      for (y = 0; y < *height; ++y, cursor += *rowbytes)
	{

	  for (x = 0; x < *rowbytes; x += 4)
	    {

	      // Set bit in mask when alpha channel is nonzero

	      if (rowPointers[y][x + 3])

		(*clipData)[(y * clipRowbytes) + (x / 32)] |=
		  (1 << ((x / 4) % 8));

	    }

	}

    }

  TeardownPng (png, info);

  free (rowPointers);

}

#define PACKAGE_VERSION "1.0"
void
DisplayAbout (void)
{
  printf ("delaykill v" PACKAGE_VERSION " \n");
}

void
DisplayUsage (void)
{
  DisplayAbout ();
  printf ("usage: delaykill [ options ]\n");
}

void
DisplayHelp (void)
{
  DisplayUsage ();
  printf ("  -n program  kill program name\n"
	  "  -t timeout  deylay time\n" "  -v          Display version\n");
}

int
main (int argc, char *argv[])
{
  char *program = NULL;
  int timeout = 5;
  for (int x = 1; x < argc; x++)
    {
      if (!strcmp (argv[x], "-v"))
	{
	  DisplayAbout ();
	  exit (0);
	}
      else if (!strcmp (argv[x], "-h"))
	{
	  DisplayHelp ();
	  exit (0);
	}
      else if (!strcmp (argv[x], "-n") && x + 1 < argc)
	{
	  program = argv[++x];
	}
      else if (!strcmp (argv[x], "-t") && x + 1 < argc)
	{
	  timeout = atoi (argv[++x]);
	}
      else
	{
	  printf ("unrecognized option: %s\n", argv[x]);
	  DisplayHelp ();
	  exit (1);
	}
    }


  struct screen myscreen;
  Imlib_Image image;

  screen_init (&myscreen);
  imlib_context_set_drawable (myscreen.root);
  image = imlib_create_image_from_drawable (0,
					    0,
					    0,
					    myscreen.screen->width,
					    myscreen.screen->height, 1);
  imlib_context_set_image (image);
  imlib_image_set_format ("png");
  imlib_save_image ("/dev/shm/xlscreen.png");
  imlib_free_image ();

  Display *display;

  int screen_num;

  Window win;
  unsigned int display_width, display_height;
  unsigned int width, height;
  char *display_name = getenv ("DISPLAY");

  GC gc;

  /*  in our window.               */
  unsigned width_ = 0, height_ = 0;

  unsigned char *data = NULL;
  char *clip = NULL;
  unsigned rowbytes = 0;
//      unsigned long *image, *mask;

  /* open connection with the X server. */

  display = myscreen.display;

  if (display == NULL)
    {

      fprintf (stderr, "%s: cannot connect to X server '%s'\n", argv[0],
	       display_name);

      exit (1);

    }

  /* get the geometry of the default screen for our display. */
  screen_num = DefaultScreen (myscreen.display);

  display_width = DisplayWidth (display, screen_num);

  display_height = DisplayHeight (display, screen_num);

  /* make the new window occupy 1/9 of the screen's size. */

  width = (display_width / 3);

  height = (display_height / 3);

  printf ("window width - '%d'; height - '%d'\n", width, height);

//      win = create_simple_window(display, display_width, display_height, 0, 0);
  win = create_full_window (display);

  gc = create_gc (display, win, 0);

  XSync (display, False);

  /* catch expose events */
  XMapRaised (display, win);
  XSelectInput (display, win, ExposureMask | KeyPressMask);
//      XSelectInput(display, win, ExposureMask);

  /* wait for the expose event */

  XEvent ev;

  XNextEvent (display, &ev);

  // Load image

  FILE *file = fopen ("/dev/shm/xlscreen.png", "r");

  if (!file)

    return -1;

  LoadPng (file, &data, &clip, &width_, &height_, &rowbytes);

//  Atom wm_state   = XInternAtom (display, "_NET_WM_STATE", True );
//  Atom wm_fullscreen = XInternAtom (display, "_NET_WM_STATE_FULLSCREEN", True );
//
//  XChangeProperty(display, win, wm_state, XA_ATOM, 32,
//                PropModeReplace, (unsigned char *)&wm_fullscreen, 1);
//
//  XMoveWindow (display, win, 0, -100);

  if (!data)
    return -1;
  XImage *ximage =
    XCreateImage (display, DefaultVisual (display, DefaultScreen (display)),
		  DefaultDepth (display, DefaultScreen (display)), ZPixmap, 0,
		  (char *) data, width_, height_, 8, rowbytes);

  if (ximage)
    {

      XPutImage (display, win, gc, ximage, 0, 0, 0, 0, width_, height_);

    }
  else
    {

      free (data);

    }

//      while(1)
//      {
//              XNextEvent(display, &ev);
//              if ( ev.type ==  Expose)
//              {
//                      XPutImage (display, win, gc, ximage, 0, 0, 0, 0, width_, height_);
//                      XFlush(display);
//              
//              }
//              if ( ev.type == KeyPress);
//              {
//                      if (XK_q == XLookupKeysym (&ev.xkey, 0))
//                      {
//                              break;
////                            exit (EXIT_SUCCESS);
//                      }
//              }
//      }       
//      char cmd[256] = "killall -9 ";
  char cmd[256] = "killall ";
  if (program)
    {
      strcat (cmd, program);
      strcat (cmd, "&");
      system (cmd);
    }
  sleep (timeout);
  free (clip);
  XFlush (display);

  XSync (display, False);
  XCloseDisplay (display);

//      return 0;
  exit (EXIT_SUCCESS);
}
