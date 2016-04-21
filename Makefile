# Copyright 2008-2016, Jeffrey E. Bedard <jefbed@gmail.com> 
# Copyright 1999-2015, Ciaran Anscomb <jbwm@6809.org.uk>
# See README for license and other details.

# Debugging symbols
CFLAGS+=-ggdb
# Link time optimization greatly reduces binary size:
# However, it may not work correctly with clang.
#CFLAGS+=-flto
# Enable all warnings
CFLAGS+=-W -Wall -Wextra

# Uncomment if you use firefox with flash fullscreen videos.
#DEFINES += -DFIX_FIREFOX

# Uncomment to enable SHAPE extension support
DEFINES += -DUSE_SHAPE
EXTRALIBS += -lXext # Required for SHAPE extension

# Titlebar Xft support:
#include xft.mk

# Uncomment to enable parsing command line arguments.
#  Saves ~2030 bytes
DEFINES += -DUSE_ARGV 

# Uncomment to enable titlebars
DEFINES += -DUSE_TBAR
EXTRASRCS += titlebar.c 

# Uncomment to enable window snapping. ~4k
include snap.mk

# Uncomment to enable STDIO
DEFINES += -DSTDIO

# Uncomment to enable EWMH ~8k
DEFINES += -DEWMH
EXTRASRCS += ewmh.c

# Uncomment to enable MWM hints
include mwm.mk

include jbwm.mk

