#if defined(MAC_OSX_TK)
#include "getContext.h"
#include <tk.h>
#import <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
#undef TextStyle

// Undocumented internal Tk function. NB you have to compile TCL/Tk
// from source statically in order to access this function, as the
// dylib version makes this symbol private.
// Use configure --prefix=$HOME/usr --enable-aqua --disable-shared in the unix directory of Tcl/Tk
extern "C" NSWindow* TkMacOSXDrawableWindow(Drawable drawable);

NSContext::NSContext(Drawable win)
{
  NSWindow* nswin=TkMacOSXDrawableWindow(win);
  nscontext=[[NSGraphicsContext graphicsContextWithWindow:nswin] retain];                                 
  context=((NSGraphicsContext*)nscontext).CGContext;
}

NSContext::~NSContext()
{
  [(NSGraphicsContext*)nscontext release];
}

#endif
