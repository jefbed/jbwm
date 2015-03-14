#ifndef SCREENINFO_H
#define SCREENINFO_H

typedef struct ScreenInfo
{
	int screen;
	Window root;
	GC gc;
	XColor fg, bg;
	ubyte vdesk;
	XColor fc;
	char *display;
	unsigned short width, height;
} ScreenInfo;


#endif /* not SCREENINFO_H */