/* jbwm, Copyright 2015 Jeffrey E. Bedard */
#ifndef JBWM_TITLEBAR_H
#define JBWM_TITLEBAR_H

#include "graphics.h"

void 
update_titlebar(Client * c);

void 
delete_titlebar(Client * c);

/* Ensure that the title text fits within the titlebar.  */
#ifdef USE_TBAR
//#define TDIM (jbwm.X.font->ascent+jbwm.X.font->descent-JBWM_BORDER)
#define TDIM 16
#else /* !USE_TBAR */
#define TDIM 8
#endif
#ifdef USE_XPM
#undef TDIM
#define TDIM 24
#endif

#endif /* JBWM_TITLEBAR_H */
