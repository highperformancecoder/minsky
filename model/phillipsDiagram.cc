/*
  @copyright Steve Keen 2023
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

#include "cairoItems.h"
#include "phillipsDiagram.h"
#include "phillipsDiagram.rcd"
#include "phillipsDiagram.xcd"
#include "minsky.h"
#include "minsky_epilogue.h"
using ecolab::cairo::CairoSave;

namespace minsky
{
  std::map<Units, double> PhillipsFlow::maxFlow;
  std::map<Units, double> PhillipsStock::maxStock;
  
  void PhillipsFlow::draw(cairo_t* cairo)
  {
    CairoSave cs(cairo);
    double value=this->value();
    double& maxV=maxFlow[units()];
    if (abs(value)>maxV) maxV=abs(value);
    double lineWidth=1;
    if (maxV>0)
      lineWidth=std::max(1.0, 10*abs(value)/maxV);
    cairo_set_line_width(cairo, lineWidth);
    Wire::draw(cairo,value>=0);
  }

  void PhillipsStock::draw(cairo_t* cairo) const
  {
    StockVar::draw(cairo);
    // colocate input and output ports on the input side
    m_ports[0]->moveTo(m_ports[1]->x(), m_ports[1]->y());
    auto maxV=maxStock[units()];
    if (maxV>0)
      {
        CairoSave cs(cairo);
        auto w=width()*zoomFactor(); 
        auto h=height()*zoomFactor();
        auto f=value()/maxV;
        if (f>=0)
          {
            cairo_set_source_rgba(cairo,0,0,1,0.3);
            cairo_rectangle(cairo,-0.6*w,0.5*h,1.2*w,-f*h);
          }
        else
          {
            cairo_set_source_rgba(cairo,1,0,0,0.3);
            cairo_rectangle(cairo,-0.6*w,-0.5*h,1.2*w,-f*h);
          }
        cairo_fill(cairo);
      }
  }

  
  bool PhillipsDiagram::redraw(int, int, int width, int height)
  {
    if  (!surface.get()) return false;
    auto cairo=surface->cairo();
    for (auto& i: stocks)
      {
        CairoSave cs(cairo);
        cairo_identity_matrix(cairo);
        cairo_translate(cairo,i.second.x(), i.second.y());
        i.second.draw(cairo);
      }
    for (auto& i: flows)
      i.second.draw(cairo);
    return true;
  }

  void PhillipsDiagram::init()
  {
    stocks.clear();
    flows.clear();
    cminsky().model->recursiveDo
      (&GroupItems::items, [this](const Items&,Items::const_iterator i) {
        if (auto g=dynamic_cast<GodleyIcon*>(i->get())) {
          for (auto& v: g->stockVars())
            stocks.emplace(v->valueId(), static_cast<Variable<VariableType::stock>&>(*v));
          for (unsigned r=1; r<g->table.rows(); ++r) {
            if (g->table.initialConditionRow(r)) continue;
            std::map<std::string, std::vector<FlowCoef>> sources, destinations; 
            for (size_t c=1; c<g->table.cols(); c++)
              {
                FlowCoef fc(g->table.cell(r,c));
                if (fc.coef)
                  {
                    if (fc.coef>0 && !g->table.signConventionReversed(c) || fc.coef<0 && g->table.signConventionReversed(c))
                      sources[fc.name].emplace_back(fc.coef,g->table.cell(0,c));
                    else 
                      destinations[fc.name].emplace_back(fc.coef,g->table.cell(0,c));
                  }
              }
            for (auto& i: sources)
              for (auto& s: i.second)
                for (auto& d: destinations[i.first])
                  {
                    auto& source=stocks[g->valueId(s.name)];
                    auto& dest=stocks[g->valueId(d.name)];
                    auto key=make_pair(s.name,d.name);
                    bool swapped=false;
                    if (s.name<d.name)// canonicalise flow by inserting in reverse direction
                      {
                        auto flow=flows.emplace(make_pair(s.name,d.name), PhillipsFlow(source.ports(0), dest.ports(1))).first;
                        flow->second.addTerm(s.coef*d.coef, i.first); 
                      }
                    else
                      {
                        auto flow=flows.emplace(make_pair(d.name,s.name), PhillipsFlow(dest.ports(0), source.ports(1))).first;
                        flow->second.addTerm(s.coef*d.coef, i.first); 
                      }
                  }
          }
        }
        return false;
      });

        // now layout the diagram
    double angle=0, delta=2*M_PI/stocks.size();
    
    for (auto& i: stocks)
      {
        i.second.moveTo(100*cos(angle)+500,100*sin(angle)+500);
        i.second.rotation(angle*180.0/M_PI);
        angle+=delta;
      }

  }

  void PhillipsDiagram::updateMaxValues()
  {}
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PhillipsDiagram);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PhillipsFlow);
