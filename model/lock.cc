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
#include "lasso.h"
#include <cairo_base.h>
#include "itemT.rcd"
#include "lock.rcd"
#include "lock.xcd"
#include "minsky_epilogue.h"
using namespace std;

namespace minsky
{
  SVGRenderer Lock::lockedIcon;
  SVGRenderer Lock::unlockedIcon;

  Lock::Lock()
  {
    tooltip="Double click to lock/unlock";
    iWidth(30);
    iHeight(30);
    addPorts();
  }

  void Lock::addPorts()
  {
    m_ports.clear();
    m_ports.emplace_back(make_shared<Port>(*this));
    m_ports[0]->moveTo(x()+15,y());
    m_ports.emplace_back(make_shared<InputPort>(*this));
    m_ports[1]->moveTo(x()-15,y());
  }

  Ravel* Lock::ravelInput() const
  {
    if (auto inputPort=ports(1).lock())
      if (!inputPort->wires().empty())
        if (auto fromPort=inputPort->wires()[0]->from())
          return dynamic_cast<Ravel*>(&fromPort->item());
    return nullptr;
  }

  
  void Lock::toggleLocked()
  {
    if (locked())
      lockedState.clear();
    else
      if (auto* r=ravelInput())
        {
          lockedState=r->getState();
          tooltip=ravel::Ravel::description(lockedState);
        }
      else
        throw_error("Locks can only be applied to Ravels");
  }

  void Lock::draw(cairo_t* cairo) const 
  {
    const float z=zoomFactor()*scaleFactor();
    const float w=iWidth()*z, h=iHeight()*z;

    {
      const ecolab::cairo::CairoSave cs(cairo);
      cairo_translate(cairo,-0.5*w,-0.5*h);
      SVGRenderer* icon=locked()? &lockedIcon: &unlockedIcon;
      cairo_scale(cairo, w/icon->width(), h/icon->height());
      icon->render(cairo);
    }
    
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
        if (auto* r=ravelInput())
          if (auto p=r->ports(1).lock())
            {
              Units inputUnits=p->units(check);
              if (inputUnits.empty()) return inputUnits;
              size_t multiplier=1;
              // at this stage, gross up exponents by the handle size of each
              // reduced by product handles
              for (const auto& h: lockedState.handleStates)
                if (h.collapsed && h.reductionOp==ravel::Op::prod)
                  {
                    // find which handle number this is
                    // TODO - is there a way of avoiding this second loop?
                    size_t i=0;
                    for (; i<r->maxRank(); ++i)
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
    return m_ports[1]->units(check);
  }

  void Lock::applyLockedStateToRavel() const
  {
    if (auto ravel=ravelInput())
      ravel->applyState(lockedState);
  }


}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Lock);
