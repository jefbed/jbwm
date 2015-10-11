/*
 * jbwm - Minimalist Window Manager for X Copyright (C) 1999-2006 Ciaran
 * Anscomb <jbwm@6809.org.uk> see README for license and other details.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "jbwm.h"

#define MAXIMUM_PROPERTY_LENGTH 4096

static void init_geometry(Client * c);
static void reparent(Client * c);

#ifdef XDEBUG
static const char *map_state_string(int map_state);
static void debug_wm_normal_hints(XSizeHints * size);
#else
#define debug_wm_normal_hints(s)
#endif

static void
map_client(Client * c, ScreenInfo * s)
{
	/*
	 * Only map the window frame (and thus the window) if it's supposed
	 * to be visible on this virtual desktop.
	 */
	if(s->vdesk == c->vdesk)
		unhide(c);
	else
		set_wm_state(c, IconicState);
}

static void
initialize_client_fields(Client * c, ScreenInfo * s, Window w)
{
	c->next = head_client;
	head_client = c;

	c->screen = s;
	c->window = w;
	c->window_type = None;
}

Client *
Client_new(Window w, ScreenInfo * s)
{
	Client *c;

	c = calloc(1, sizeof(Client));
	initialize_client_fields(c, s, w);
	c->border = JBWM_BORDER_WIDTH;
	init_geometry(c);

	return c;
}

void
make_new_client(Window w, ScreenInfo * s)
{
	Client *c;

	XGrabServer(jbwm.X.dpy);
	/*
	 * First a bit of interaction with the error handler due to X's
	 * tendency to batch event notifications.  We set a global variable
	 * to the id of the window we're initialising then do simple X call
	 * on that window.  If an error is raised by this (and nothing else
	 * should do so as we've grabbed the server), the error handler
	 * resets the variable indicating the window has already disappeared,
	 * so we stop trying to manage it.
	 */
	jbwm.initialising = w;
	XSync(jbwm.X.dpy, false);
	/*
	 * If 'initialising' is now set to None, that means doing the
	 * XFetchName raised BadWindow - the window has been removed before
	 * we got a chance to grab the server.
	 */
	if(jbwm.initialising == None)
		goto end;
	jbwm.initialising = None;
	c = Client_new(w, s);
	if(c->flags & JB_CLIENT_DONT_MANAGE)
		goto end;
	XSelectInput(jbwm.X.dpy, c->window,
		EnterWindowMask | PropertyChangeMask |
		ColormapChangeMask);
#ifdef USE_SHAPE
	set_shape(c);
#endif /* USE_SHAPE */
	reparent(c);
	map_client(c, s);
	/* Enable alt-dragging within window */
	jbwm_grab_button(w, jbwm.keymasks.grab, AnyButton);
end:
	XUngrabServer(jbwm.X.dpy);
}

/*
 * Calls XGetWindowAttributes, XGetWMHints and XGetWMNormalHints to determine
 * window's initial geometry.
 * 
 * XGetWindowAttributes
 */

static unsigned long
handle_mwm_hints(Client * c)
{
	PropMwmHints *mprop;
	unsigned long nitems;
	const Atom mwm_hints = GETATOM("_XA_MWM_HINTS");

	if((mprop =
		jbwm_get_property(c->window, mwm_hints, mwm_hints,
		&nitems)))
	{
		if(nitems >= PROP_MWM_HINTS_ELEMENTS
			&& (mprop->flags & MWM_HINTS_DECORATIONS)
			&& !(mprop->decorations & MWM_DECOR_ALL)
			&& !(mprop->decorations & MWM_DECOR_BORDER))
		{
			c->border = 0;
		}
		XFree(mprop);
	}

	return nitems;
}

static void
set_size(Client * c, const unsigned int width,
	const unsigned int height)
{
	c->geometry.width = width;
	c->geometry.height = height;
	send_config(c);
}

static void
init_geometry_size(Client * c, XWindowAttributes * attr)
{
	const int awidth = attr->width;
	const int aheight = attr->height;

	if((awidth >= c->size->min_width)
		&& (aheight >= c->size->min_height))
		set_size(c, awidth, aheight);
	else
		set_size(c, c->size->min_width, c->size->min_height);
}

static void
set_position(Client * c, const int x, const int y)
{
	c->geometry.x = x;
	c->geometry.y = y;
	send_config(c);
}

static void
init_geometry_position(Client * c, XWindowAttributes * attr)
{
	if(!c->size)
		return;
	if((attr->map_state == IsViewable)
		|| c->size->flags & USPosition)
		set_position(c, attr->x, attr->y);
	else
	{
		const ScreenInfo *s = c->screen;

		if(!s)
			return;
		{
			const short mx = s->width;
			const short my = s->height;
			const XRectangle *g = &(c->geometry);
			const ubyte b = c->border;

			Window root = s->root;
			Position p;

			get_mouse_position((int *)&p.x, (int *)&p.y,
				root);
			set_position(c,
				(p.x * (mx - b - g->width)) / mx,
				(p.y * (my - b - g->height)) / my);
		}
	}
}

static void
init_long_properties(Client * c, unsigned long *nitems)
{
	unsigned long *lprop;

	if((lprop =
		jbwm_get_property(c->window, JBWM_ATOM_VWM_DESKTOP,
		XA_CARDINAL, nitems)))
	{
		if(*nitems && valid_vdesk(lprop[0]))
			c->vdesk = lprop[0];
		XFree(lprop);
	}
}

static void
init_atom_properties(Client * c, unsigned long *nitems)
{
	Atom *aprop;

	if((aprop =
		jbwm_get_property(c->window, JBWM_ATOM_WM_STATE,
		XA_ATOM, nitems)))
	{
		unsigned long i;

		for(i = 0; i < *nitems; i++)
			if(aprop[i] == JBWM_ATOM_VWM_STICKY)
				add_sticky(c);
		XFree(aprop);
	}
}

static void
init_geometry_properties(Client * c)
{
	unsigned long nitems;

	nitems = handle_mwm_hints(c);
	if(!c->screen)
		return;
	c->vdesk = c->screen->vdesk;
	init_long_properties(c, &nitems);
	remove_sticky(c);
	init_atom_properties(c, &nitems);

}

static void
init_geometry(Client * c)
{
	XWindowAttributes attr;

	initialize_client_ce(c);
	init_geometry_properties(c);

	XGetWindowAttributes(jbwm.X.dpy, c->window, &attr);
#ifdef USE_CMAP
	c->cmap = attr.colormap;
#endif /* USE_CMAP */
	get_wm_normal_hints(c);
	init_geometry_size(c, &attr);
	init_geometry_position(c, &attr);
	/* Test if the reparent that is to come 
	   would trigger an unmap event. */
	if(attr.map_state == IsViewable)
		c->ignore_unmap++;
}

static void
reparent(Client * c)
{
	XSetWindowAttributes p_attr;
	unsigned long valuemask =
		CWOverrideRedirect | CWBorderPixel | CWBackPixel |
		CWEventMask;
	const XRectangle *g = &(c->geometry);
	const ubyte b = c->border;
	const int x = g->x - b;
	const int y = g->y - b; //- y_mod;
	const Window w = c->window;

	if(!c->screen)
		return;
#ifndef USE_SHAPE
	p_attr.background_pixel = BlackPixel(jbwm.X.dpy, c->screen->screen);
	valuemask |= CWBackPixel;
#endif /* !USE_SHAPE */
	p_attr.border_pixel = c->screen->bg.pixel;
	p_attr.override_redirect = true;
	p_attr.event_mask = ChildMask | ButtonPressMask | EnterWindowMask;
	c->parent = XCreateWindow(jbwm.X.dpy, c->screen->root, x, y, 
		g->width, g->height, b, DefaultDepth(jbwm.X.dpy, 
		c->screen->screen), CopyFromParent, DefaultVisual(jbwm.X.dpy,
		c->screen->screen), valuemask, &p_attr);
	XAddToSaveSet(jbwm.X.dpy, w);
	XSetWindowBorderWidth(jbwm.X.dpy, w, 0);
#ifdef USE_TBAR
	if(!(c->flags & JB_CLIENT_NO_TB))
		update_info_window(c);
#endif
	XReparentWindow(jbwm.X.dpy, w, c->parent, 0, 0);
	XMapWindow(jbwm.X.dpy, w);
}

/* Get WM_NORMAL_HINTS property */
void
get_wm_normal_hints(Client * c)
{
	if(c->size)
		XFree(c->size);
	c->size = XAllocSizeHints();
	if(!c->size)	/* If memory could not be allocated */
		return;
	{
		long supplied_return;

		XGetWMNormalHints(jbwm.X.dpy, c->window, c->size,
			&supplied_return);
	}
}

void *
jbwm_get_property(Window w, Atom property, Atom req_type,
	unsigned long *nitems_return)
{
	Atom actual_type;
	int actual_format;
	unsigned long bytes_after;
	unsigned char *prop;

	if(XGetWindowProperty(jbwm.X.dpy, w, property, 0L,
		MAXIMUM_PROPERTY_LENGTH / 4, false, req_type,
		&actual_type, &actual_format, nitems_return,
		&bytes_after, &prop) == Success)
	{
		if(actual_type == req_type)
			return (void *)prop;
		else
			XFree(prop);
	}
	return NULL;
}
