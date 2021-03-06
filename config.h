// jbwm - Minimalist Window Manager for X
// Copyright 2008-2020, Jeffrey E. Bedard <jefbed@gmail.com>
// Copyright 1999-2015, Ciaran Anscomb <evilwm@6809.org.uk>
// See README for license and other details.
#ifndef JBWM_CONFIG_H
#define JBWM_CONFIG_H
#include <X11/X.h> // for keymasks
/* Note:  Also, adjust the values in JBWMDefaults.h as necessary.  */
#define JBWM_TERM "xterm"
/* Default fonts */
#ifdef JBWM_USE_XFT
#define JBWM_FONT "Sans:size=10:aspect=1.2"
#else//!JBWM_USE_XFT
#define JBWM_FONT "fixed"
#endif//JBWM_USE_XFT
/* Default colors */
#ifndef USE_OLD_THEME
//#define JBWM_FG "#bbf"
#define JBWM_FG "grey"
#define JBWM_FC "green1"
#define JBWM_BG "black"
#define JBWM_CLOSE "green4"
#define JBWM_RESIZE "green4"
#define JBWM_SHADE "green3"
#define JBWM_STICK "green4"
#define JBWM_NAME "jbwm"
#else//USE_OLD_THEME
#define JBWM_FG "#bad0ff"
#define JBWM_FC "#00ff2f"
#define JBWM_BG "#3b4352"
#define JBWM_CLOSE "#ff2600"
#define JBWM_RESIZE "#00a1ff"
#define JBWM_SHADE "#ffe200"
#define JBWM_STICK "#5bf662"
#define JBWM_NAME "jbwm"
#endif//!USE_OLD_THEME
/* Compile-time defaults of an integer type shall be stored here.  */
enum {
  JBWM_KEYMASK_GRAB = Mod4Mask,
  JBWM_KEYMASK_MOD = ControlMask,
  JBWM_SNAP = 10,
  JBWM_RESIZE_INCREMENT = 20,
  JBWM_NUMBER_OF_DESKTOPS = 10
};
#endif//JBWM_CONFIG_H
