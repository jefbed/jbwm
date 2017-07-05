// Copyright 2017, Jeffrey E. Bedard
#include "key_event.h"
#include <stdlib.h>
#include "JBWMKeys.h"
#include "JBWMScreen.h"
#include "client.h"
#include "config.h"
#include "drag.h"
#include "exec.h"
#include "key_masks.h"
#include "keys.h"
#include "log.h"
#include "max.h"
#include "move_resize.h"
#include "screen.h"
#include "select.h"
#include "snap.h"
#include "title_bar.h"
#include "vdesk.h"
#include "wm_state.h"
__attribute__((nonnull))
static void point(struct JBWMClient * restrict c,
	const int16_t x, const int16_t y)
{
	XRaiseWindow(c->display, c->parent);
	jbwm_warp(c->display, c->window, x, y);
}
__attribute__((nonnull))
static void commit_key_move(struct JBWMClient * restrict c)
{
	jbwm_snap_border(c);
	jbwm_move_resize(c);
	point(c, 1, 1);
}
enum KeyMoveFlags {
	KEY_MOVE_HORIZONTAL = 1,
	KEY_MOVE_POSITIVE = 2,
	KEY_MOVE_MODIFIER = 4
};
static inline bool can_resize(struct JBWMClientOptions * restrict opt)
{
	return !opt->shaped && !opt->no_resize;
}
__attribute__((nonnull))
static void key_move(struct JBWMClient * restrict c, const uint8_t flags)
{
	enum { I = JBWM_RESIZE_INCREMENT };
	struct JBWMRectangle * restrict s = &c->size;
	uint16_t * restrict wh = flags & KEY_MOVE_HORIZONTAL ? &s->width
		: &s->height;
	const int8_t d = flags & KEY_MOVE_POSITIVE ? I : - I;
	if((flags & KEY_MOVE_MODIFIER) && (*wh > I << 1) && can_resize(&c->opt))
		*wh += d;
	else
		*(flags & KEY_MOVE_HORIZONTAL ? &s->x : &s->y) += d;
	commit_key_move(c);
}
static void handle_key_move(struct JBWMClient * restrict c,
	const KeySym k, const bool mod)
{
	/* These operations invalid when fullscreen.  */
	if (c->opt.fullscreen)
		return;
	uint8_t flags = (mod ? KEY_MOVE_MODIFIER : 0) | KEY_MOVE_POSITIVE;
	switch (k) {
	case JBWM_KEY_LEFT:
		flags &= ~KEY_MOVE_POSITIVE;
		// FALLTHROUGH
	case JBWM_KEY_RIGHT:
		flags |= KEY_MOVE_HORIZONTAL;
		break;
	case JBWM_KEY_UP:
		flags &= ~KEY_MOVE_POSITIVE;
	}
	key_move(c, flags);
}
static void set_maximized(struct JBWMClient * restrict c)
{
	jbwm_set_horz(c);
	jbwm_set_vert(c);
}
static void set_not_maximized(struct JBWMClient * restrict c)
{
	jbwm_set_not_horz(c);
	jbwm_set_not_vert(c);
}
static void toggle_maximize(struct JBWMClient * restrict c)
{
	const struct JBWMClientOptions o = c->opt;
	// Honor !MWM_FUNC_MAXIMIZE
	// Maximizing shaped windows is buggy, so return.
	if (o.shaped || o.no_max || o.fullscreen)
		return;
	(o.max_horz && o.max_vert
	 ? set_not_maximized : set_maximized)(c);
}
__attribute__((nonnull))
static void handle_client_key_event(struct JBWMClient * restrict c,
	const bool mod, const KeySym key)
{
	JBWM_LOG("handle_client_key_event: %d", (int)key);
	if (c->opt.fullscreen) {
		// only allow exiting from fullscreen
		if (key == JBWM_KEY_FS)
			jbwm_set_not_fullscreen(c);
		return; // prevent other operations while fullscreen
	}
	switch (key) {
	case JBWM_KEY_LEFT:
	case JBWM_KEY_RIGHT:
	case JBWM_KEY_UP:
	case JBWM_KEY_DOWN:
		handle_key_move(c, key, mod);
		break;
	case JBWM_KEY_KILL:
		jbwm_send_wm_delete(c);
		break;
	case JBWM_KEY_LOWER:
	case JBWM_KEY_ALTLOWER:
		XLowerWindow(c->display, c->parent);
		break;
	case JBWM_KEY_RAISE:
		XRaiseWindow(c->display, c->parent);
		break;
	case JBWM_KEY_FS:
		jbwm_set_fullscreen(c);
		break;
	case JBWM_KEY_MAX:
		toggle_maximize(c);
		break;
	case JBWM_KEY_MAX_H:
		(c->opt.max_horz ? jbwm_set_not_horz : jbwm_set_horz)(c);
		break;
	case JBWM_KEY_MAX_V:
		(c->opt.max_vert ? jbwm_set_not_vert : jbwm_set_vert)(c);
		break;
	case JBWM_KEY_STICK:
		jbwm_toggle_sticky(c);
		break;
	case JBWM_KEY_MOVE:
		jbwm_drag(c, false);
		break;
	case JBWM_KEY_SHADE:
		jbwm_toggle_shade(c);
		break;
	}
}
static struct JBWMClient * get_next_on_vdesk(struct JBWMClient * c)
{
	if (c)
		c = c->next;
	if (!c)
		c = jbwm_get_current_client() ? jbwm_get_head_client() : NULL;
	/* Non-NULL c indicates not yet being at the end of the linked list.
	 * If we've reached the current client, then we've made a loop (it was
	 * advanced past once in the first statement, c = c->next), so exit.
	 * Thus we check c != jbwm_get_current_client().  If c's vdesk does
	 * not equal the screen's vdesk the advance to the next client.  */
	return c && c != jbwm_get_current_client()
		&& c->vdesk != jbwm_get_screens()[c->screen].vdesk
		? get_next_on_vdesk(c) : c;
}
static void next(void)
{
	struct JBWMClient * restrict c =
		get_next_on_vdesk(jbwm_get_current_client());
	if (c) {
		jbwm_select_client(c);
		point(c, 0, 0);
		point(c, c->size.width, c->size.height);
	}
}
static void cond_client_to_desk(Display * d, struct JBWMClient * c,
	struct JBWMScreen * s, const uint8_t desktop, const bool mod)
{
	mod && c ? jbwm_set_client_vdesk(c, desktop)
		: jbwm_set_vdesk(d, s, desktop);
}
void jbwm_handle_key_event(XKeyEvent * e)
{
	JBWM_LOG("jbwm_handle_key_event");
	const KeySym key = XLookupKeysym(e, 0);
	struct JBWMClient * restrict c = jbwm_get_current_client();
	struct JBWMScreen * s = c ? jbwm_get_screen(c) : jbwm_get_screens();
	struct {
		uint8_t vdesk:6;
		bool mod:1;
		bool zero:1;
	} opt = {s->vdesk, e->state & jbwm_get_mod_mask(), 0};
	switch (key) {
	case JBWM_KEY_NEW:
		jbwm_start_terminal();
		break;
	case JBWM_KEY_QUIT:
		exit(0);
	case JBWM_KEY_NEXT:
		next();
		break;
	case XK_0:
		opt.zero = true;
		// FALLTHROUGH
	case XK_1: case XK_2: case XK_3: case XK_4: case XK_5:
	case XK_6: case XK_7: case XK_8: case XK_9:
		// First desktop 0, per wm-spec
		cond_client_to_desk(e->display, c, s, opt.zero
			? 10 : key - XK_1, opt.mod);
		break;
	case JBWM_KEY_PREVDESK:
		cond_client_to_desk(e->display, c, s, s->vdesk - 1, opt.mod);
		break;
	case JBWM_KEY_NEXTDESK:
		cond_client_to_desk(e->display, c, s, s->vdesk + 1, opt.mod);
		break;
	default:
		if (c)
			handle_client_key_event(c, opt.mod, key);
	}
}
