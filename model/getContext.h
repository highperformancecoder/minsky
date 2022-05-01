/*
  @copyright Steve Keen 2021
  @author Russell Standish
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

#ifndef GETCONTEXT_H
#define GETCONTEXT_H
#include <Carbon/Carbon.h>
#include <cairo/cairo.h>
#include <functional>

namespace minsky
{
  struct ViewImpl;
  class WindowInformation;
  
  struct NSContext
  {
    std::unique_ptr<ViewImpl> impl;
    NSContext(void* nativeHandle,int xoffs,int yoffs,int width,int height,WindowInformation&);
    ~NSContext();
    void requestRedraw();
    NSContext(const NSContext&)=delete;
    NSContext(NSContext&&);
    void operator=(const NSContext&)=delete;
  };
}
#endif
