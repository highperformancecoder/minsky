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

#ifndef EVENTINTERFACE_H
#define EVENTINTERFACE_H
#include <string>

namespace minsky
{
  // enforces a uniform mouse and keyboard event handling interface
  struct EventInterface
  {
    virtual void mouseDown(float x, float y) {}
    virtual void controlMouseDown(float x, float y) {mouseDown(x,y);}
    virtual void mouseUp(float x, float y) {}
    virtual void mouseMove(float x, float y) {}
    virtual void zoom(double x, double y, double z) {}

    
    /// handle key press over current itemFocus,
    /// @param keySym the X key sym code
    /// @param utf8 utf8 encoded character
    /// @param state modifer state 1=shift, 2=caps lock, 4=ctrl, 8=alt
    /// @param x & y contain mouse coordinates
    /// @return true if event handled
    virtual bool keyPress(int keySym, const std::string& utf8, int state, float x, float yn)
    {return false;}

  };
}

#include "eventInterface.cd"
#endif
