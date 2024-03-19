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
#include "minsky.h"
#include "minsky_epilogue.h"
using ecolab::cairo::CairoSave;

namespace minsky
{
  std::map<Units, double> PhillipsFlow::maxFlow;
  std::map<Units, double> PhillipsStock::maxStock;
  
  void PhillipsFlow::draw(cairo_t* cairo)
  {
    const CairoSave cs(cairo);
    const double value=this->value();
    double& maxV=maxFlow[units()];
    if (abs(value)>maxV) maxV=abs(value);
    double lineWidth=1;
    if (maxV>0)
      {
        const double lw=5*abs(value)/maxV;
        lineWidth=std::max(1.0, lw);
        static const double dashLength=3;
        if (lw<1)
          cairo_set_dash(cairo,&dashLength,1,0);
      }
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
        const CairoSave cs(cairo);
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
    const CairoSave cs(cairo);
    cairo_translate(cairo,x,y);
    for (auto& i: stocks)
      {
        const CairoSave cs(cairo);
        cairo_identity_matrix(cairo);
        cairo_translate(cairo,i.second.x()+x, i.second.y()+y);
        i.second.draw(cairo);
      }
    for (auto& i: flows)
      i.second.draw(cairo);
    return true;
  }

  void PhillipsDiagram::init()
  {
    decltype(stocks) newStocks;
    decltype(flows) newFlows;
    cminsky().model->recursiveDo
      (&GroupItems::items, [&](const Items&,Items::const_iterator it) {
        if (auto g=dynamic_cast<GodleyIcon*>(it->get())) {
          for (auto& v: g->stockVars())
            {
              auto newStock=newStocks.emplace(v->valueId(), static_cast<Variable<VariableType::stock>&>(*v)).first;
              auto oldStock=stocks.find(v->valueId());
              if (oldStock!=stocks.end())
                {
                  newStock->second.moveTo(oldStock->second.x(), oldStock->second.y());
                  newStock->second.rotation(oldStock->second.rotation());
                }
            }
          for (unsigned r=1; r<g->table.rows(); ++r) {
            if (g->table.initialConditionRow(r)) continue;
            map<string, vector<pair<FlowCoef, string>>> sources, destinations; 
            for (size_t c=1; c<g->table.cols(); c++)
              {
                const FlowCoef fc(g->table.cell(r,c));
                if (fc.coef)
                  {
                    auto payload=make_pair(FlowCoef(fc.coef,g->table.cell(0,c)),g->table.cell(r,0));
                    if ((fc.coef>0 && !g->table.signConventionReversed(c))
		        || (fc.coef<0 && g->table.signConventionReversed(c)))
                      sources[fc.name].emplace_back(payload);
                    else 
                      destinations[fc.name].emplace_back(payload);
                  }
              }
            for (auto& i: sources)
              for (auto& [s,sd]: i.second)
                for (auto& [d,description]: destinations[i.first])
                  {
                    auto ss=s, dd=d;
                    if (s.coef*d.coef<0) swap(ss,dd);
                    auto& source=newStocks[g->valueId(ss.name)];
                    auto& dest=newStocks[g->valueId(dd.name)];
                    auto flow=newFlows.emplace(make_pair(g->valueId(dd.name),g->valueId(ss.name)), PhillipsFlow(dest.ports(0), source.ports(1))).first;
                    flow->second.addTerm(abs(s.coef*d.coef), i.first);
                    flow->second.tooltip=(!flow->second.tooltip.empty()?";":"")+description;
                    if (auto oldFlow=flows.find(flow->first); oldFlow!=flows.end())
                      flow->second.coords(oldFlow->second.coords());
                  }
          }
        }
        return false;
      });

    // now layout the diagram
    if (newStocks.empty())
      {
        stocks.clear();
        flows.clear();
        minsky().pushHistory();
        return;
      }

    double angle=0, delta=2*M_PI/newStocks.size();


    auto h=newStocks.begin()->second.height();
    auto maxW=newStocks.begin()->second.width();
    for (auto& i: newStocks) maxW=max(maxW,i.second.width());
    // calculate radius to ensure vars do not overlap
    auto r=h/delta + 0.5*maxW;
    
    for (auto& i: newStocks)
      {
        if (!stocks.contains(i.first))
          {
            i.second.moveTo(r*(cos(angle)+1)+maxW+50,r*(sin(angle)+1)+maxW+50);
            i.second.rotation(angle*180.0/M_PI);
          }
        angle+=delta;
      }

    flows.swap(newFlows);
    stocks.swap(newStocks);
    minsky().pushHistory();
  }

  void PhillipsDiagram::mouseDown(float x, float y)
  {
    if (stockBeingRotated) return;
    x-=this->x; y-=this->y;
    for (auto& i: stocks)
      if (i.second.contains(x,y))
        {
          stockBeingMoved=&i.second;
          return;
        }

    for (auto& i: flows)
      if (i.second.near(x,y))
        {
          flowBeingEdited=&i.second;
          i.second.mouseFocus=true;
          handleSelected=i.second.nearestHandle(x,y);
          return;
        }
  }
  
  void PhillipsDiagram::mouseUp(float x, float y)
  {
    mouseMove(x,y);
    minsky().pushHistory();
    stockBeingMoved=nullptr;
    stockBeingRotated=nullptr;
    flowBeingEdited=nullptr;
  }
  
  void PhillipsDiagram::mouseMove(float x, float y)
  {
    x-=this->x; y-=this->y;
    if (stockBeingRotated)
      {
        stockBeingRotated->rotate(Point{x,y}, rotateOrigin);
        requestRedraw();
        return;
      }
    if (stockBeingMoved)
      {
        stockBeingMoved->moveTo(x,y);
        requestRedraw();
        return;
      }
    if (flowBeingEdited)
      {
        flowBeingEdited->editHandle(handleSelected,x,y);
        requestRedraw();
        return;
      }
    for (auto& i: flows)
      {
        const bool mf=i.second.near(x,y);
        if (mf!=i.second.mouseFocus)
        {
          i.second.mouseFocus=mf;
          requestRedraw();
        }
      }
  }
  
  void PhillipsDiagram::startRotatingItem(float x, float y)
  {
    x-=this->x; y-=this->y;
      for (auto& i: stocks)
        if (i.second.contains(x,y))
          {
            stockBeingRotated=&i.second;
            rotateOrigin=Point(x,y);
            requestRedraw();
            return;
          }
  }

  
}
//CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PhillipsDiagram);
//CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PhillipsFlow);
