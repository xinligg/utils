#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <giblib/giblib.h>
#include <X11/Xlib.h>
#include <Imlib2.h>

struct screen {
  Display *display;
  Screen *screen;
  Visual *visual;
  Colormap colormap;
  Window root;
};

void
screen_init(struct screen *myscreen)
{
  myscreen->display = XOpenDisplay(NULL);
  if (!myscreen->display) {
    perror("XOpenDisplay");
    exit(EXIT_FAILURE);
  }
  myscreen->screen = ScreenOfDisplay(myscreen->display,
                  DefaultScreen(myscreen->display));
  myscreen->visual = DefaultVisual(myscreen->display,
                DefaultScreen(myscreen->display));
  myscreen->colormap = DefaultColormap(myscreen->display,
                DefaultScreen(myscreen->display));
  myscreen->root = RootWindow(myscreen->display,
                DefaultScreen(myscreen->display));
  imlib_context_set_display(myscreen->display);
  imlib_context_set_visual(myscreen->visual);
  imlib_context_set_colormap(myscreen->colormap);
}

int
main(int argc, char *argv[])
{
  struct screen myscreen;
  Imlib_Image image;

  screen_init(&myscreen);
  image = gib_imlib_create_image_from_drawable(myscreen.root,
                           0,
                           0,
                           0,
                           myscreen.screen->width,
                           myscreen.screen->height,
                           1);
  imlib_context_set_image(image);
  imlib_image_set_format("jpeg");
  imlib_save_image("screen.jpeg");
  imlib_free_image();
  exit(EXIT_SUCCESS);
}
