// Copyright 2016, Jeffrey E. Bedard
#ifndef JBWM_WM_STATE_H
#define JBWM_WM_STATE_H
#include "JBWMClient.h"
// Return WM_STATE atom
Atom jbwm_get_wm_state(Display * d);
// Send WM_DELETE message to the client
void jbwm_send_wm_delete(struct JBWMClient * restrict c)
	__attribute__((nonnull));
// Set WM_STATE property
void jbwm_set_wm_state(struct JBWMClient * restrict c, const int8_t state);
#endif//!JBWM_WM_STATE_H
