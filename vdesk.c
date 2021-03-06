// jbwm - Minimalist Window Manager for X
// Copyright 2008-2020, Jeffrey E. Bedard <jefbed@gmail.com>
// Copyright 1999-2015, Ciaran Anscomb <evilwm@6809.org.uk>
// See README for license and other details.
#include "vdesk.h"
#include "client.h"
#include "config.h"
#include "ewmh.h"
#include "atom.h"
#include "JBWMClient.h"
#include "JBWMScreen.h"
#include "util.h"
#include <X11/Xatom.h> // keep
static void check_visibility(struct JBWMClient * c,
  const uint8_t v)
{
  if (c) {
    if (c->vdesk == v || c->opt.sticky) {
      // allow moving windows by sticking
      c->vdesk = v;
      jbwm_restore_client(c);
    } else
      jbwm_hide_client(c);
    check_visibility(c->next, v);
  }
}
uint8_t jbwm_set_vdesk(struct JBWMScreen *s,
  struct JBWMClient *head, uint8_t v)
{
  Display *d;
  check_visibility(head, v);
  s->vdesk = v;
  d=s->xlib->display;
  // The data (v) must be a 32 bit type.
  XChangeProperty(d, s->xlib->root, jbwm_atoms[JBWM_NET_CURRENT_DESKTOP],
    XA_CARDINAL, 8, PropModeReplace, (unsigned char *)&v, 1);
  return v;
}
