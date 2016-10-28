// jbwm - Minimalist Window Manager for X
// Copyright 2008-2016, Jeffrey E. Bedard <jefbed@gmail.com>
// Copyright 1999-2015, Ciaran Anscomb <jbwm@6809.org.uk>
// See README for license and other details.
#include "client.h"
#include "ewmh.h"
#include "ewmh_state.h"
#include "jbwm.h"
#include "log.h"
#include "screen.h"
#include "title_bar.h"
#include "util.h"
#include <X11/Xatom.h>
static struct {
	struct JBWMClient * current, * head;
} jbwm_client_data;
struct JBWMClient * jbwm_get_current_client(void)
{
	return jbwm_client_data.current;
}
struct JBWMClient * jbwm_get_head_client(void)
{
	return jbwm_client_data.head;
}
void jbwm_set_head_client(struct JBWMClient * restrict c)
{
	if (c)
		jbwm_client_data.head = c;
}
// Relink c's linked list to exclude c
void jbwm_relink_client_list(struct JBWMClient * c)
{
	JBWM_LOG("relink_window_list");
	if (jbwm_client_data.current == c) // Remove selection target
		jbwm_client_data.current = NULL;
	if (jbwm_client_data.head == c) {
		jbwm_client_data.head = c->next;
		return;
	}
	for (struct JBWMClient * p = jbwm_client_data.head;
		p && p->next; p = p->next) {
		if (p->next == c) { // Close the link
			p->next = c->next;
			return;
		}
	}
}
void jbwm_set_client_vdesk(struct JBWMClient * restrict c, const uint8_t d)
{
	JBWM_LOG("jbwm_set_client_vdesk");
	const uint8_t p = c->vdesk;
	c->vdesk = d;
	// use jbwm_set_vdesk to validate d:
	struct JBWMScreen * s = &jbwm_get_screens()[c->screen];
	Display * restrict dpy = jbwm_get_display();
	c->vdesk = jbwm_set_vdesk(dpy, s, d);
	jbwm_set_vdesk(dpy, s, p);
}
static struct JBWMClient * search(struct JBWMClient * c,
	const jbwm_window_t w)
{
#ifdef JBWM_USE_TITLE_BAR
	return (!c || c->parent == w || c->window == w || c->tb.win == w)
#else//!JBWM_USE_TITLE_BAR
	return (!c || c->parent == w || c->window == w)
#endif//JBWM_USE_TITLE_BAR
		? c : search(c->next, w);
}
// Return the client that has specified window as either window or parent
struct JBWMClient * jbwm_get_client(const jbwm_window_t w)
{
	return search(jbwm_client_data.head, w);
}
static void unselect_current(void)
{
	if(!jbwm_client_data.current)
		return;
	Display * restrict d = jbwm_get_display();
	XSetWindowBorder(d, jbwm_client_data.current->parent,
		jbwm_get_screens()[jbwm_client_data.current->screen]
		.pixels.bg);
#ifdef JBWM_USE_EWMH
	jbwm_ewmh_remove_state(d, jbwm_client_data.current->window,
		ewmh[WM_STATE_FOCUSED]);
#endif//JBWM_USE_EWMH
}
static void set_border(struct JBWMClient * restrict c)
{
	struct JBWMScreen * restrict s = &jbwm_get_screens()[c->screen];
	XSetWindowBorder(jbwm_get_display(), c->parent, c->opt.sticky
		? s->pixels.fc : s->pixels.fg);
}
void jbwm_select_client(struct JBWMClient * c)
{
	if(!c)
		return;
	unselect_current();
	Display * d = jbwm_get_display();
	XInstallColormap(d, c->cmap);
	XSetInputFocus(d, c->window,
		RevertToPointerRoot, CurrentTime);
	set_border(c);
	jbwm_client_data.current = c;
#ifdef JBWM_USE_EWMH
	jbwm_set_property(d, jbwm_get_screens()[c->screen].root,
		ewmh[ACTIVE_WINDOW], XA_WINDOW, &(c->parent), 1);
	jbwm_ewmh_add_state(d, c->window, ewmh[WM_STATE_FOCUSED]);
#endif//JBWM_USE_EWMH
}
void jbwm_toggle_sticky(struct JBWMClient * c)
{
	JBWM_LOG("stick");
	c->opt.sticky ^= true; // toggle
	jbwm_select_client(c);
	Display * restrict d = jbwm_get_display();
	jbwm_update_title_bar(d, c);
#ifdef JBWM_USE_EWMH
	if (c->opt.sticky)
		jbwm_ewmh_add_state(d, c->window, ewmh[WM_STATE_STICKY]);
	else
		jbwm_ewmh_remove_state(d, c->window,
			ewmh[WM_STATE_STICKY]);
#endif//JBWM_USE_EWMH
}
// Returns 0 on failure.
static Status xmsg(const jbwm_window_t w, const Atom a, const long x)
{
	JBWM_LOG("xmsg");
	return XSendEvent(jbwm_get_display(), w, false, NoEventMask, &(XEvent){
		.xclient.type = ClientMessage, .xclient.window = w,
		.xclient.message_type = a, .xclient.format = 32,
		.xclient.data.l[0] = x, .xclient.data.l[1] = CurrentTime
	});
}
static jbwm_atom_t get_atom(jbwm_atom_t * a, const char * name)
{
	return *a ? *a : (*a = XInternAtom(jbwm_get_display(), name, false));
}
static jbwm_atom_t get_wm_protocols(void)
{
	static jbwm_atom_t a;
	return get_atom(&a, "WM_PROTOCOLS");
}
static jbwm_atom_t get_wm_delete_window(void)
{
	static jbwm_atom_t a;
	return get_atom(&a, "WM_DELETE_WINDOW");
}
jbwm_atom_t jbwm_get_wm_state(void)
{
	static jbwm_atom_t a;
	return get_atom(&a, "WM_STATE");
}
void jbwm_set_wm_state(Display * restrict d,
	struct JBWMClient * restrict c, const int8_t state)
{
	jbwm_set_property(d, c->window, jbwm_get_wm_state(),
		XA_CARDINAL, &(uint32_t){state}, 1);
}
static bool has_delete_proto(const jbwm_window_t w)
{
	bool found=false;
	Atom *p;
	int i;
	if(XGetWMProtocols(jbwm_get_display(), w, &p, &i)) {
		while(i-- && !found)
			found = p[i] == get_wm_delete_window();
		XFree(p);
	}
	return found;
}
void jbwm_send_wm_delete(struct JBWMClient * restrict c)
{
	if (c->opt.remove) { // this allows a second click to force a kill
		XKillClient(jbwm_get_display(), c->window);
		return;
	}
	c->opt.remove = true;
	has_delete_proto(c->window)?xmsg(c->window, get_wm_protocols(),
		get_wm_delete_window()): XKillClient(jbwm_get_display(), c->window);
}
