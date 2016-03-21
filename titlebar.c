// jbwm - Minimalist Window Manager for X
// Copyright 2008-2016, Jeffrey E. Bedard <jefbed@gmail.com>
// Copyright 1999-2015, Ciaran Anscomb <jbwm@6809.org.uk>
// See README for license and other details.
#include "jbwm.h"

static GC colorgc(ScreenInfo * restrict s, const char *restrict colorname)
{
	XColor c, nullc;
	XAllocNamedColor(D, DefaultColormap(D, s->screen), colorname, &c,
			 &nullc);
	XGCValues v = {.foreground = c.pixel };
	return XCreateGC(D, s->root, GCForeground, &v);
}

static inline void setup_gcs(ScreenInfo * restrict s)
{
	jbwm.gc.close = colorgc(s, TITLEBAR_CLOSE_BG);
	jbwm.gc.shade = colorgc(s, TITLEBAR_SHADE_BG);
	jbwm.gc.resize = colorgc(s, TITLEBAR_RESIZE_BG);
}

static void new_titlebar(Client * restrict c)
{
	if (c->flags & (JB_NO_TB|JB_SHAPED))
		return;

	const Window w = XCreateSimpleWindow(D, c->parent, 0, 0, 
		c->size.width, TDIM, 0, 0, 0);
	c->titlebar=w;
	if (!jbwm.gc.close)
		setup_gcs(c->screen);

	XSelectInput(D, w, ExposureMask);
	XSetWindowBackground(D, w, c->screen->bg.pixel);
	XMapRaised(D, w);
	jbwm_grab_button(w, 0, AnyButton);
}

#ifdef USE_XFT
static void
draw_xft(Client * restrict c, const XPoint * restrict p,
	 char *restrict name, const size_t l)
{
	XGlyphInfo e;
	XftTextExtentsUtf8(D, jbwm.font, (XftChar8 *) name, l, &e);
	const uint8_t s = c->screen->screen;
	Visual *v = DefaultVisual(D, s);
	const Colormap cm = DefaultColormap(D, s);
	XftDraw *xd = XftDrawCreate(D, c->titlebar, v, cm);
	XftColor color;
	XftColorAllocName(D, v, cm, DEF_FG, &color);
	/* Prevent the text from going over the resize button.  */
	const unsigned short max_width = c->size.width - 3 * TDIM;
	XftDrawStringUtf8(xd, &color, jbwm.font, p->x, p->y,
			  (XftChar8 *) name,
			  e.width > max_width
			  && e.width > 0 ? l * max_width / e.width : l);
	XftDrawDestroy(xd);
	XftColorFree(D, v, cm, &color);
}
#endif//USE_XFT

GC tbgc;

static void draw_title(Client * restrict c)
{
	char * name = get_title(c->window);
	const size_t l = strlen(name);
	const XPoint p = { TDIM + 4, jbwm.font->ascent - JBWM_BORDER };
#ifdef USE_XFT
	draw_xft(c, &p, name, l);
#else//!USE_XFT
	XDrawString(D, c->titlebar, c->screen->gc, p.x, p.y, name, l);
#endif//USE_XFT
	XFree(name);
}

static void draw(const Window t, GC gc, const int x)
{
	XFillRectangle(D, t, gc, x, 0, TDIM, TDIM);
}

static inline void draw_close(const uint32_t f, const Window t)
{
	if (!(f & JB_NO_CLOSE_DECOR))
		draw(t, jbwm.gc.close, 0);
}

static inline void draw_shade(const uint32_t f, const Window t, const int x)
{
	if (!(f & JB_NO_MIN_DECOR))
		draw(t, jbwm.gc.shade, x);
}

static inline void draw_resize(const uint32_t f, const Window t, const int x)
{
	if (!(f & JB_NO_MIN_DECOR))
		draw(t, jbwm.gc.resize, x);
}

static inline int tboffset(const int w, const int n)
{
	return w - n * TDIM;
}

static void draw_titlebar(Client * restrict c)
{
	const unsigned short w = c->size.width;
	const Window t = c->titlebar;
	XClearWindow(D, t);
	const uint32_t f = c->flags;
	draw_close(f, t);
	draw_resize(f, t, tboffset(w, 1));
	draw_shade(f, t, tboffset(w, 2));

	if (!(f & JB_TEAROFF))
		draw_title(c);
}

void update_titlebar(Client * c)
{
	assert(c);

	if (c->flags & (JB_NO_TB | JB_SHAPED))
		return;

	if (c->flags & JB_MAXIMIZED) {
		/* May generate BadWindow on subsequent invocations,
		   however the error handler makes such irrelevant.  */
		XDestroyWindow(D, c->titlebar);
		c->titlebar = 0;
		return;
	}

	if (!c->titlebar) {
		new_titlebar(c);
		/* Return here to prevent BadWindow/BadDrawable errors */
		return;
	}

	/* Expand/Contract the titlebar width as necessary:  */
	XMoveResizeWindow(D, c->titlebar, 0, 0, c->size.width, TDIM);
	draw_titlebar(c);
}
