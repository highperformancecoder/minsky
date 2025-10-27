/*
  @copyright Steve Keen 2021
  @author Janak Porwal
  This file is part of Minsky.

  Minsky is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Minsky is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WINDOW_INFORMATION_H
#define WINDOW_INFORMATION_H

#include <cairoSurfaceImage.h>
#if defined(CAIRO_HAS_WIN32_SURFACE) && !defined(__CYGWIN__)
#define USE_WIN32_SURFACE
#include <windows.h>
#include <wingdi.h>
#elif defined(MAC_OSX_TK)
#include "getContext.h"
#elif defined(CAIRO_HAS_XLIB_SURFACE)
#define USE_X11
#include <cairo/cairo-xlib.h>
#include <X11/Xlib.h>
#endif

#include <atomic>
#include <functional>
#include <mutex>
#include <future>
#include <thread>

namespace minsky
{
  struct Winfo
  {
#ifdef USE_WIN32_SURFACE
    HWND parentWindowId, childWindowId;
    HBITMAP hbmMem; // backing buffer pixmap
    HDC hdcMem; // backing buffer bitmap device context
    HANDLE hOld;    // 
#elif defined(MAC_OSX_TK)
    NSContext nsContext;
    std::shared_ptr<std::lock_guard<std::recursive_mutex>> lock;
    Winfo(NSContext&& nsContext): nsContext(std::move(nsContext)) {}
#elif defined(USE_X11)
    Window parentWindowId;
    Window childWindowId, bufferWindowId;
    
    Display*	display; // Weak reference, returned by system
    GC graphicsContext;
    XWindowAttributes wAttr;
#endif
    int childWidth;
    int childHeight;
    int offsetLeft;
    int offsetTop;
  };
  
  class WindowInformation: public Winfo
  {
    friend class RenderNativeWindow;
#if defined(MAC_OSX_TK)
    friend struct NSContext;
#elif defined(USE_X11)
    struct EventThread: public Winfo, public  std::thread
    {
      void run();
      EventThread(const Winfo& w): Winfo(w), thread([this]{run();}) {}
      ~EventThread() {XDestroyWindow(display, childWindowId); join();}
    };

    std::unique_ptr<EventThread> eventThread;
#endif

  public: 
      
    const bool hasScrollBars=false;
    ecolab::cairo::SurfacePtr bufferSurface;
    std::function<void()> draw;
    void copyBufferToMain();
      
  public:
    ~WindowInformation();
    // TODO refactor all these classes to avoid this dependency inversion
    WindowInformation(uint64_t parentWin, int left, int top, int cWidth, int cHeight,double scalingFactor,bool hasScrollBars,const std::function<void()>& draw);
    
    const ecolab::cairo::SurfacePtr& getBufferSurface() const;
    void requestRedraw();
    
    WindowInformation(const WindowInformation&)=delete;
    void operator=(const WindowInformation&)=delete;
  };
} // namespace minsky

#endif
