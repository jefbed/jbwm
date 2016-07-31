// jbwm - Minimalist Window Manager for X
// Copyright 2008-2016, Jeffrey E. Bedard <jefbed@gmail.com>
// Copyright 1999-2015, Ciaran Anscomb <jbwm@6809.org.uk>
// See README for license and other details.

#ifndef JBWM_CLIENT_H
#define JBWM_CLIENT_H

#include "client_t.h"

void client_to_vdesk(Client * restrict c, const uint8_t d)
	__attribute((nonnull));

Client *find_client(jbwm_window_t w) __attribute__((hot,pure));

// Free result with XFree if not NULL
char * get_title(const jbwm_window_t w) __attribute__((pure));

void select_client(Client * c);

void send_wm_delete(const Client * restrict c) __attribute__((nonnull));

// state -1 returns WM_STATE atom
jbwm_atom_t set_wm_state(Client * c, const int8_t state);

void stick(Client * c) __attribute__((nonnull,cold));

#endif /* JBWM_CLIENT_H */
