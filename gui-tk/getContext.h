#ifndef GETCONTEXT_H
#define GETCONTEXT_H
#include <tk.h>
#include <Carbon/Carbon.h>

struct NSContext
{
  CGContextRef context;
  void* nscontext;
  NSContext(Drawable win);
  ~NSContext();
  NSContext(const NSContext&)=delete;
  void operator=(const NSContext&)=delete;
};
#endif
