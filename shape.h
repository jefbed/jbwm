// jbwm - Minimalist Window Manager for X
// Copyright 2008-2016, Jeffrey E. Bedard <jefbed@gmail.com>
// Copyright 1999-2015, Ciaran Anscomb <jbwm@6809.org.uk>
// See README for license and other details.
#ifndef JBWM_SHAPE_H
#define JBWM_SHAPE_H
#ifdef JBWM_USE_SHAPE
#include "JBWMClient.h"
void jbwm_new_shaped_client(struct JBWMClient * restrict c)
	__attribute__((nonnull));
void jbwm_set_shape(struct JBWMClient * restrict c)
	__attribute__((nonnull));
#else//!JBWM_USE_SHAPE
#define jbwm_new_shaped_client(c)
#define jbwm_set_shape(c)
#endif//JBWM_USE_SHAPE
#endif//!JBWM_SHAPE_H
