// jbwm - Minimalist Window Manager for X
// Copyright 2008-2016, Jeffrey E. Bedard <jefbed@gmail.com>
// Copyright 1999-2015, Ciaran Anscomb <jbwm@6809.org.uk>
// See README for license and other details.

#ifndef JBWM_SCREEN_H
#define JBWM_SCREEN_H

#include "JBWMClient.h"

enum {DESKTOPS=10};

void jbwm_drag(struct JBWMClient * restrict c, const bool resize)
	__attribute__((nonnull));

void moveresize(struct JBWMClient * restrict c)
	__attribute__((nonnull));

uint8_t switch_vdesk(struct JBWMScreen * s, uint8_t v)
	__attribute__((nonnull));

void unhide(struct JBWMClient * restrict c)
	__attribute__((nonnull));

#endif /* JBWM_SCREEN_H */
