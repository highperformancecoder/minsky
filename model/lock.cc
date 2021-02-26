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

#include "lock.h"
#include "wire.h"
#include "selection.h"
#include <cairo_base.h>
#include "minsky_epilogue.h"
using ecolab::cairo::CairoSave;

namespace minsky
{
  SVGRenderer Lock::lockedIcon;
  SVGRenderer Lock::unlockedIcon;

  Lock::Lock()
  {
    tooltip="Double click to lock/unlock";
    iWidth(30);
    iHeight(30);
    m_ports.emplace_back(new Port(*this,Port::noFlags));
    m_ports[0]->moveTo(15,0);
    m_ports.emplace_back(new Port(*this, Port::inputPort));
    m_ports[1]->moveTo(-15,0);
  }

  Ravel* Lock::ravelInput() const
  {
    if (auto inputPort=ports(1).lock())
      if (inputPort->wires().size())
        if (auto fromPort=inputPort->wires()[0]->from())
          return dynamic_cast<Ravel*>(&fromPort->item());
    return nullptr;
  }

  
  void Lock::toggleLocked()
  {
    if (locked())
      lockedState.clear();
    else
      if (auto r=ravelInput())
        lockedState=r->getState();
  }

  void Lock::draw(cairo_t* cairo) const 
  {
    SVGRenderer* icon=locked()? &lockedIcon: &unlockedIcon;
    float z=zoomFactor()*scaleFactor();
    float w=iWidth()*z, h=iHeight()*z;

    // Windows flubs rendering the padlock icon correctly, so temporily replace by simple text representation
//    {
//      CairoSave cs(cairo);
//      cairo_translate(cairo,-0.5*w,-0.5*h);
//      cairo_scale(cairo, w/icon->width(), h/icon->height());
//      icon->render(cairo);
//    }
    cairo_rectangle(cairo,-0.5*w,-0.5*h,w,h);
    if (locked())
      {
        cairo_move_to(cairo,-0.5*w,-0.5*h);
        cairo_line_to(cairo,0.5*w,0.5*h);
        cairo_move_to(cairo,-0.5*w,0.5*h);
        cairo_line_to(cairo,0.5*w,-0.5*h);
      }
    cairo_stroke(cairo);
    
    if (mouseFocus)
      { 		  
        drawPorts(cairo);
        displayTooltip(cairo,tooltip);
        if (onResizeHandles) drawResizeHandles(cairo);
      }	       

    // add 8 pt margin to allow for ports
    cairo_rectangle(cairo,-0.5*w-8,-0.5*h-8,w+16,h+8);
    cairo_clip(cairo);
    if (selected) drawSelected(cairo);
  }

  Units Lock::units(bool check) const
  {
    if (locked())
      {
        if (auto r=ravelInput())
          if (auto p=r->ports(1).lock())
            {
              Units inputUnits=p->units(check);
              if (inputUnits.empty()) return inputUnits;
              size_t multiplier=1;
              // at this stage, gross up exponents by the handle size of each
              // reduced by product handles
              for (auto& h: lockedState.handleStates)
                if (h.collapsed && h.reductionOp==ravel::Op::prod)
                  {
                    // find which handle number this is
                    // TODO - is there a way of avoiding this second loop?
                    size_t i=0;
                    for (; i<r->numHandles(); ++i)
                      if (r->handleDescription(i)==h.description)
                        {
                          multiplier*=r->numSliceLabels(i);
                          break;
                        }
                  }
              if (multiplier>1)
                for (auto& u: inputUnits)
                  u.second*=multiplier;
              return inputUnits;
            }
        return {};
      }
    else
      return m_ports[1]->units(check);
  }

}
