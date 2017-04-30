// jbwm - Minimalist Window Manager for X
// Copyright 2008-2017, Jeffrey E. Bedard <jefbed@gmail.com>
// Copyright 1999-2015, Ciaran Anscomb <jbwm@6809.org.uk>
// See README for license and other details.
//#undef DEBUG
#include "screen.h"
#include <X11/Xatom.h> // keep
#include "client.h"
#include "config.h"
#include "display.h"
#include "ewmh.h" // keep
#include "log.h"
#include "util.h" // keep
static struct JBWMScreen * screens;
struct JBWMScreen * jbwm_get_screens(void)
{
	return screens;
}
void jbwm_set_screens(struct JBWMScreen * restrict s)
{
	screens = s;
}
extern inline Window jbwm_get_root(struct JBWMClient * restrict c);
extern inline struct JBWMScreen * jbwm_get_screen(struct JBWMClient
	* restrict c);
static void check_visibility(struct JBWMScreen * s,
	struct JBWMClient * restrict c, const uint8_t v)
{
	if (c->screen != s->screen)
		return;
	if (c->vdesk == v || c->opt.sticky) {
		c->vdesk = v; // allow moving windows by sticking
		jbwm_restore_client(c);
	} else
		jbwm_hide_client(c);
}
uint8_t jbwm_set_vdesk(struct JBWMScreen * s, uint8_t v)
{
	JBWM_LOG("jbwm_set_vdesk");
	if (v == s->vdesk || v > JBWM_MAX_DESKTOPS)
		return s->vdesk;
	for (struct JBWMClient * restrict c = jbwm_get_head_client();
		c; c = c->next)
		check_visibility(s, c, v);
	s->vdesk = v;
#ifdef JBWM_USE_EWMH
	jbwm_set_property(jbwm_get_display(), s->root,
		jbwm_ewmh_get_atom(JBWM_EWMH_CURRENT_DESKTOP),
		XA_CARDINAL, &v, 1);
#endif//JBWM_USE_EWMH
	return s->vdesk;
}
