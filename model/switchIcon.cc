/*
  @copyright Steve Keen 2015
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
#include "minsky.h"
#include "cairoItems.h"
#include "itemT.rcd"
#include "switchIcon.h"
#include "switchIcon.rcd"
#include "../engine/cairoShimCairo.h"
#include "minsky_epilogue.h"
using namespace ecolab::cairo;
using namespace ecolab;
using namespace std;

namespace minsky
{
  void SwitchIcon::setNumCases(unsigned n)
  {
    if (n<2) throw error("switches need at least two cases");

    m_ports.clear();
    m_ports.emplace_back(make_shared<Port>(*this));
    // output port, selector port and n case ports
    for (unsigned i=m_ports.size(); i<n+2; ++i)
      m_ports.emplace_back(make_shared<InputPort>(*this));
    const float width=8*numCases();
    if (width>iWidth()) iWidth(width);
    if (width>iHeight()) iHeight(width);
  }

  unsigned SwitchIcon::switchValue() const
  {
    const double x=m_ports[1]->value();
    if (x<1)
      return 0;
    if (x>=numCases()-1)
      return numCases()-1;
    return unsigned(x);
  }

  Units SwitchIcon::units(bool check) const 
  {
    bool inputFound=false;
    Units r;
    for (size_t i=2; i<m_ports.size(); ++i)
      for (auto w: m_ports[i]->wires())
        if (check && inputFound)
          {
            auto tmp=w->units(check);
            if (tmp!=r)
              throw_error("incompatible units: "+tmp.str()+"≠"+r.str());
          }
        else
          {
            inputFound=true;
            r=w->units(check);
          }
    return r;
  } 

  void SwitchIcon::draw(cairo_t* cairo) const
  {
    auto z=zoomFactor();
    // following the draw method in the Sheet class, iWidth() and iHeight() have been changed to m_width and m_height,
    // since the former largely play a role in the VariableBase and OperationBase classes. for ticket 1250
    const float width=m_width*z, height=m_height*z;
    cairo_set_line_width(cairo,1);
    cairo_rectangle(cairo,-0.5*width,-0.5*height,width,height);
    cairo_stroke(cairo);     	 

    const float w=flipped? -width: width;
    const float o=flipped? -8: 8;
    // output port
    drawTriangle(cairo, 0.5*w, 0, palette[0], flipped? M_PI: 0);
    m_ports[0]->moveTo(x()+0.5*w, y());
    // control port
    drawTriangle(cairo, 0, -0.5*height-8, palette[0], M_PI/2);
    m_ports[1]->moveTo(x(), y()-0.5*height-8);
    const float dy=height/numCases();
    float y1=-0.5*height+0.5*dy;
    // case ports
    for (size_t i=2; i<m_ports.size(); ++i, y1+=dy)
      {
        drawTriangle(cairo, -0.5*w-o, y1, palette[(i-2)%paletteSz], flipped? M_PI: 0);
        m_ports[i]->moveTo(x()+-0.5*w-o, y()+y1);
      }
    // draw indicating arrow
    cairo_move_to(cairo,0.5*w, 0);
    try
      {
        y1=-0.5*width+0.5*dy+switchValue()*dy;
      }
    catch (const std::exception&)
      {
        y1=-0.5*width+0.5*dy;
      }
    cairo_line_to(cairo,-0.45*w,0.9*y1);
    cairo_stroke(cairo);

    if (mouseFocus)
      { 		  
        drawPorts(cairo);
        displayTooltip(cairo,tooltip());
        if (onResizeHandles) drawResizeHandles(cairo);
      }	       

    // add 8 pt margin to allow for ports
    cairo_rectangle(cairo,-0.5*width-8,-0.5*height-8,width+16,height+8);
    cairo_clip(cairo);
    if (selected) drawSelected(cairo);
  }

  void SwitchIcon::draw(const ICairoShim& cairoShim) const
  {
    // TODO: Implement properly - this uses drawTriangle which needs refactoring
    auto& shimImpl = dynamic_cast<const CairoShimCairo&>(cairoShim);
    draw(shimImpl._internalGetCairoContext());
  }
  
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::SwitchIcon);
