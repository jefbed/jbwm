// Copyright 2017, Jeffrey E. Bedard
#ifndef JBWM_JBWMATOMINDEX_H
#define JBWM_JBWMATOMINDEX_H
enum JBWMAtomIndex {
	JBWM_EWMH_SUPPORTED,
	JBWM_EWMH_CURRENT_DESKTOP,
	JBWM_EWMH_NUMBER_OF_DESKTOPS,
	JBWM_EWMH_DESKTOP_VIEWPORT,
	JBWM_EWMH_DESKTOP_GEOMETRY,
	JBWM_EWMH_SUPPORTING_WM_CHECK,
	JBWM_EWMH_ACTIVE_WINDOW,
	JBWM_EWMH_MOVERESIZE_WINDOW,
	JBWM_EWMH_CLOSE_WINDOW,
	JBWM_EWMH_CLIENT_LIST,
	JBWM_EWMH_VIRTUAL_ROOTS,
	JBWM_EWMH_CLIENT_LIST_STACKING,
	JBWM_EWMH_FRAME_EXTENTS,
	JBWM_EWMH_WM_ALLOWED_ACTIONS,
	JBWM_EWMH_WM_NAME,
	JBWM_EWMH_WM_DESKTOP,
	JBWM_EWMH_WM_MOVERESIZE,
	JBWM_EWMH_WM_PID,
	JBWM_EWMH_WM_WINDOW_TYPE,
	JBWM_EWMH_WM_STATE,
	JBWM_EWMH_WM_ACTION_MOVE,
	JBWM_EWMH_WM_ACTION_RESIZE,
	JBWM_EWMH_WM_ACTION_CLOSE,
	JBWM_EWMH_WM_ACTION_SHADE,
	JBWM_EWMH_WM_ACTION_FULLSCREEN,
	JBWM_EWMH_WM_ACTION_CHANGE_DESKTOP,
	JBWM_EWMH_WM_ACTION_ABOVE,
	JBWM_EWMH_WM_ACTION_BELOW,
	JBWM_EWMH_WM_ACTION_MAXIMIZE_HORZ,
	JBWM_EWMH_WM_ACTION_MAXIMIZE_VERT,
	JBWM_EWMH_WM_STATE_STICKY,
	JBWM_EWMH_WM_STATE_MAXIMIZED_VERT,
	JBWM_EWMH_WM_STATE_MAXIMIZED_HORZ,
	JBWM_EWMH_WM_STATE_SHADED,
	JBWM_EWMH_WM_STATE_HIDDEN,
	JBWM_EWMH_WM_STATE_FULLSCREEN,
	JBWM_EWMH_WM_STATE_ABOVE,
	JBWM_EWMH_WM_STATE_BELOW,
	JBWM_EWMH_WM_STATE_FOCUSED,
	// The following entry must be last:
	JBWM_EWMH_ATOMS_COUNT,
};
#endif//!JBWM_JBWMATOMINDEX_H