/*
  @copyright Steve Keen 2012
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
#include "plotWidget.h"
#include "variable.h"
#include "cairoItems.h"
#include "latexMarkup.h"
#include "pango.h"
#include <timer.h>
#include <cairo/cairo-ps.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>

#include "itemT.rcd"
#include "plotOptions.rcd"
#include "plot.rcd"
#include "plot.xcd"
#include "tensorInterface.rcd"
#include "tensorInterface.xcd"
#include "tensorVal.rcd"
#include "tensorVal.xcd"
#include "plotWidget.rcd"
#include "minsky_epilogue.h"

#include <algorithm>
#include <numeric>
using namespace ecolab::cairo;
using namespace ecolab;
using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace minsky
{
  namespace
  {
    // orientation of bounding box ports
    const double orient[PlotWidget::nBoundsPorts]={-0.4*M_PI, -0.6*M_PI, -0.2*M_PI, 0.2*M_PI, 1.2*M_PI, 0.8*M_PI};
    // x coordinates of bounding box ports
    const float boundX[PlotWidget::nBoundsPorts]={-0.46,0.45,-0.49,-0.49, 0.48, 0.48};
    // y coordinates of bounding box ports
    const float boundY[PlotWidget::nBoundsPorts]={0.49,0.49,0.47,-0.49, 0.47, -0.49};

    // height of title, as a fraction of overall widget height
    const double titleHeight=0.05;

  }

  PlotWidget::PlotWidget()
  {
    m_width=m_height=150;
    nxTicks=nyTicks=10;
    fontScale=2;
    leadingMarker=true;
    grid=true;
    legendLeft=0.1; // override ecolab's default value
    legendSide=boundingBox;
    addPorts();
    
    yvars.resize(2*m_numLines);
    xvars.resize(m_numLines);
  }

  void PlotWidget::addPorts()
  {
    m_ports.clear();
    unsigned i=0;
    for (; i<nBoundsPorts; ++i) // bounds ports
      m_ports.emplace_back(make_shared<InputPort>(*this));
    for (; i<2*m_numLines+nBoundsPorts; ++i) // y data input ports
      m_ports.emplace_back(make_shared<MultiWireInputPort>(*this));
    for (; i<4*m_numLines+nBoundsPorts; ++i) // x data input ports
      m_ports.emplace_back(make_shared<InputPort>(*this));
  }
  
  void PlotWidget::draw(cairo_t* cairo) const
  {
    CairoSave cs(cairo);
    const double z=Item::zoomFactor();
    cairo_scale(cairo,z,z);
    const double w=iWidth();
    double h=iHeight();

    // if any titling, draw an extra bounding box (ticket #285)
    if (!title.empty()||!xlabel().empty()||!ylabel().empty()||!y1label().empty())
      {
        cairo_rectangle(cairo,-0.5*w+10,-0.5*h,w-20,h-10);
        cairo_set_line_width(cairo,1);
        cairo_stroke(cairo);
      }

    cairo_translate(cairo,-0.5*w,-0.5*h);

    yoffs=0;
    if (!title.empty())
      {
        const CairoSave cs(cairo);
        const double fy=titleHeight*iHeight();
        
        Pango pango(cairo);
        pango.setFontSize(fabs(fy));
        pango.setMarkup(latexToPango(title));   
        cairo_set_source_rgb(cairo,0,0,0);
        cairo_move_to(cairo,0.5*(w-pango.width()), 0);
        pango.show();

        // allow some room for the title
        yoffs=pango.height();
        h-=yoffs;
      }

    // draw bounding box ports
    
    size_t i=0;
    // draw bounds input ports
    for (; i<nBoundsPorts; ++i)
      {
        const float x=boundX[i]*w, y=boundY[i]*h;
        if (!justDataChanged)
          m_ports[i]->moveTo(x*z + this->x(), y*z + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i/2)%palette.size()].colour, orient[i]);
        
      }

    const float xLeft = -0.5*w, dx=w/(2*m_numLines+1); // x location of ports
    const float dy = h/m_numLines;
    // draw y data ports
    for (; i<m_numLines+nBoundsPorts; ++i)
      {
        const float y=0.5*(dy-h) + (i-nBoundsPorts)*dy;
        if (!justDataChanged)
          m_ports[i]->moveTo(xLeft*z + this->x(), y*z + this->y()+0.5*yoffs);
        drawTriangle(cairo, xLeft+0.5*w, y+0.5*h+yoffs, palette[(i-nBoundsPorts)%palette.size()].colour, 0);
      }
    
    // draw RHS y data ports
    for (; i<2*m_numLines+nBoundsPorts; ++i)
      {
        const float y=0.5*(dy-h) + (i-m_numLines-nBoundsPorts)*dy, x=0.5*w;
        if (!justDataChanged)
          m_ports[i]->moveTo(x*z + this->x(), y*z + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-nBoundsPorts)%palette.size()].colour, M_PI);
      }

    // draw x data ports
    const float yBottom=0.5*h;
    for (; i<4*m_numLines+nBoundsPorts; ++i)
      {
        const float x=dx-0.5*w + (i-2*m_numLines-nBoundsPorts)*dx;
        if (!justDataChanged)
          m_ports[i]->moveTo(x*z + this->x(), yBottom*z + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, yBottom+0.5*h+yoffs, palette[(i-2*m_numLines-nBoundsPorts)%palette.size()].colour, -0.5*M_PI);
      }

    cairo_translate(cairo, portSpace, yoffs);
    cairo_set_line_width(cairo,1);
    const double gw=w-2*portSpace, gh=h-portSpace;

    Plot::draw(cairo,gw,gh);
    if (mouseFocus && legend)
      {
        double width,height,x,y;
        legendSize(x,y,width,height,gw,gh);
        // following code puts x,y at centre point of legend
        x+=0.5*width;
        const double arrowLength=6;
        y=(h-portSpace)-y+0.5*height;
        cairo_move_to(cairo,x-arrowLength,y);
        cairo_rel_line_to(cairo,2*arrowLength,0);
        cairo_move_to(cairo,x,y-arrowLength);
        cairo_rel_line_to(cairo,0,2*arrowLength);

        cairo_move_to(cairo,x,y+0.5*height);
        cairo_rel_line_to(cairo,0,arrowLength);
        cairo_stroke(cairo);
        drawTriangle(cairo,x-arrowLength,y,{0,0,0,1},M_PI);
        drawTriangle(cairo,x+arrowLength,y,{0,0,0,1},0);
        drawTriangle(cairo,x,y-arrowLength,{0,0,0,1},3*M_PI/2);
        drawTriangle(cairo,x,y+arrowLength,{0,0,0,1},M_PI/2);
        drawTriangle(cairo,x,y+0.5*height+arrowLength,{0,0,0,1},M_PI/2);

        cairo_rectangle(cairo,x-0.5*width,y-0.5*height,width,height);
      }
    cs.restore();
    if (mouseFocus)
      {
        drawPorts(cairo);
        displayTooltip(cairo,tooltip());
        // Resize handles always visible on mousefocus. For ticket 92.
        drawResizeHandles(cairo);   
      }
    justDataChanged=false;
    
    cairo_new_path(cairo);  
    cairo_rectangle(cairo,-0.5*w,-0.5*h,w,h);
    cairo_clip(cairo);
    if (selected) drawSelected(cairo);

  }
  
  void PlotWidget::scalePlot()
  {
    // set any scale overrides
    setMinMax();
    if (xminVar && xminVar->idx()>-1)
      {
        if (xIsSecsSinceEpoch && (xminVar->units.empty() || xminVar->units==Units("year")))
          minx=yearToPTime(xminVar->value());
        else
          minx=xminVar->value();
      }
    else if (isfinite(xmin))
      {
        if (xIsSecsSinceEpoch && (cminsky().timeUnit.empty() || cminsky().timeUnit=="year"))
          minx=yearToPTime(xmin);
        else
          minx=xmin;
      }

    if (xmaxVar && xmaxVar->idx()>-1)
      {
        if (xIsSecsSinceEpoch && (xmaxVar->units.empty() || xmaxVar->units==Units("year")))
          maxx=yearToPTime(xmaxVar->value());
        else
          maxx=xmaxVar->value();
      }
    else if (isfinite(xmax))
      {
        if (xIsSecsSinceEpoch && (cminsky().timeUnit.empty() || cminsky().timeUnit=="year"))
          maxx=yearToPTime(xmax);
        else
          maxx=xmax;
      }

    if (yminVar && yminVar->idx()>-1)
      miny=yminVar->value();
    else if (isfinite(ymin))
      miny=ymin;
    if (ymaxVar && ymaxVar->idx()>-1)
      maxy=ymaxVar->value();
    else if (isfinite(ymax))
      maxy=ymax;
      
    if (y1minVar && y1minVar->idx()>-1)
      miny1=y1minVar->value();
    else if (isfinite(y1min))
      miny1=ymin;
    if (y1maxVar && y1maxVar->idx()>-1)
      maxy1=y1maxVar->value();
    else if (isfinite(y1max))
      maxy1=y1max;
    autoscale=false;

    if (!justDataChanged)
      {
        // label pens. In order or priority:
        // 1. wire tooltip
        // 2. from item tooltip
        // 3. attached variable tooltip
        // 4. attached variable name
        size_t pen=0;
        penLabels.clear();
        assert(m_ports.size()>=2*m_numLines+nBoundsPorts);
        for (auto portNo=nBoundsPorts; portNo<2*m_numLines+nBoundsPorts; ++portNo)
          {
            if (portNo<m_ports.size())
              for (size_t i=0; i<m_ports[portNo]->wires().size(); ++i, ++pen)
                {
                  auto wire=m_ports[portNo]->wires()[i];
                  if (!wire->tooltip().empty())
                    {
                      labelPen(pen, wire->tooltip());
                      continue;
                    }
                  if (auto from=wire->from(); !from->item().tooltip().empty())
                    {
                      labelPen(pen, from->item().tooltip());
                      continue;
                    }
                  if (portNo-nBoundsPorts<yvars.size() && i<yvars[portNo-nBoundsPorts].size())
                    if (auto v=yvars[portNo-nBoundsPorts][i];  !v->name.empty())
                      labelPen(pen, uqName(v->name));
                }
          }
      }
  }

  void PlotWidget::mouseDown(float x,float y)
  {
    clickX=x;
    clickY=y;
    ct=clickType(x,y);
    const double z=Item::zoomFactor();
    const double gw=iWidth()*z-2*portSpace;
    double gh=iHeight()*z-portSpace;
    if (!title.empty()) gh=iHeight()*z-portSpace-titleHeight;
    oldLegendLeft=legendLeft*gw+portSpace;
    oldLegendTop=legendTop*gh;
    oldLegendFontSz=legendFontSz;
  }
  
  void PlotWidget::mouseMove(float x,float y)
  {
    const double z=Item::zoomFactor();
    //const double w=0.5*iWidth()*z, h=0.5*iHeight()*z;
    //const double dx=x-this->x(), dy=y-this->y();
    const double gw=iWidth()*z-2*portSpace;
    double gh=iHeight()*z-portSpace;
    if (!title.empty()) gh=iHeight()*z-portSpace-titleHeight;
    const double yoffs=this->y()-(legendTop-0.5)*iHeight()*z;
    switch (ct)
      {
      case ClickType::legendMove:
        legendLeft = (oldLegendLeft + x - clickX-portSpace)/gw;
        legendTop = (oldLegendTop + clickY - y)/gh;
        if (!title.empty()) legendTop = (oldLegendTop + clickY - y + titleHeight)/gh;
        break;
      case ClickType::legendResize:
        legendFontSz = oldLegendFontSz * (y-yoffs)/(clickY-yoffs);
        if (!title.empty()) legendFontSz = oldLegendFontSz * (y-yoffs+titleHeight)/(clickY-yoffs);
        break;
      default:
        {
          auto& f=frameArgs();
          if (Plot::mouseMove((x-f.offsetLeft)/f.childWidth, (f.childHeight-f.offsetTop-y)/f.childHeight,
                              10.0/std::max(f.childWidth,f.childHeight)))
            requestRedraw();
        }
        break;
      }
  }

  bool PlotWidget::onMouseOver(float x,float y)
  {
    const double z=Item::zoomFactor();
    // coordinate system runs from bottom left to top right. Vertical coordinates must be flipped
    const double dx=x-this->x()+0.5*iWidth()*z-portSpace;
    const double dy=this->y()-y+0.5*iHeight()*z-portSpace;
    const double gw=iWidth()*z-2*portSpace;
    const double gh=iHeight()*z-portSpace-yoffs*z;
    const double loffx=lh(gw,gh)*!Plot::ylabel.empty(), loffy=lh(gw,gh)*!Plot::xlabel.empty();
    return Plot::mouseMove((dx-loffx)/gw, (dy-loffy)/gh, 10.0/std::max(gw,gh),formatter);
  }

  
  void PlotWidget::requestRedraw()
  {
    justDataChanged=true; // assume plot same size, don't do unnecessary stuff
    // store previous min/max values to determine if plot scale changes
    scalePlot();
    if (surface.get())
      surface->requestRedraw();
  }

  bool PlotWidget::redraw(int x0, int y0, int width, int height) 
  {
    if (surface.get())
      {
        auto sf=RenderNativeWindow::scaleFactor();
        Plot::draw(surface->cairo(),width/sf,height/sf);
        surface->blit();
      }
    return surface.get();
  }

  
  void PlotWidget::makeDisplayPlot() {
    if (auto g=group.lock())
      g->displayPlot=dynamic_pointer_cast<PlotWidget>(g->findItem(*this));
  }

  void PlotWidget::resize(const LassoBox& x)
  {
    const float invZ=1/Item::zoomFactor();
    iWidth(abs(x.x1-x.x0)*invZ);
    iHeight(abs(x.y1-x.y0)*invZ);
    Item::moveTo(0.5*(x.x0+x.x1), 0.5*(x.y0+x.y1));
    bb.update(*this);
  }

  // specialisation to avoid rerendering plots (potentially expensive)
  ClickType::Type PlotWidget::clickType(float x, float y) const
  {
    // firstly, check whether a port has been selected
    const double z=Item::zoomFactor();  
    for (auto& p: m_ports)
      {
        if (hypot(x-p->x(), y-p->y()) < portRadius*z)
          return ClickType::onPort;
      }

    double legendWidth, legendHeight, lx, ly;
    legendSize(lx, ly, legendWidth, legendHeight, z*(iWidth()-2*portSpace), z*(iHeight()-portSpace-yoffs));
    // xx & yy are in plot user coordinates
    const double xx= x-this->x() + z*(0.5*iWidth()-portSpace) - lx;
    const double yy= z*(0.5*iHeight()-portSpace)-y+this->y()  - ly+legendHeight;

    if (legend && xx>0 && xx<legendWidth)
      {
        if (yy>0.2*legendHeight && yy<legendHeight)
          return ClickType::legendMove;
        if (yy>-6*z &&  yy<=0.2*legendHeight) // allow a bit of extra height for resize arrow
          return ClickType::legendResize;
      }

    if (onResizeHandle(x,y)) return ClickType::onResize;         
	
    const double dx=x-this->x(), dy=y-this->y();
    const double w=0.5*iWidth()*z, h=0.5*iHeight()*z;
    return (abs(dx)<w && abs(dy)<h)?
      ClickType::onItem: ClickType::outside;
  }
  
  static ptime epoch=microsec_clock::local_time(), accumulatedBlitTime=epoch;

  static const size_t maxNumTensorElementsToPlot=10;

  size_t PlotWidget::numLines(size_t n)
  {
    if (m_numLines!=n)
      {
        m_numLines=n;
        addPorts();
      }
    return n;
  }

  
  double PlotWidget::barWidth() const
  {
    return accumulate(palette.begin(), palette.end(), 1.0,
                           [](double acc, const LineStyle& ls) {return std::min(acc, ls.barWidth);});
  }
  
  double PlotWidget::barWidth(double w)
  {
    for (auto& ls: palette) ls.barWidth=w;
    return w;
  }
    
  void PlotWidget::addPlotPt(double t)
  {
    size_t pen=0;
    for (size_t port=0; port<yvars.size(); ++port)
      for (auto& yvar: yvars[port])
        for (size_t i=0; i<min(maxNumTensorElementsToPlot,yvar->size()); ++i)
          {
            double x,y;
            switch (xvars.size())
              {
              case 0: // use t, when x variable not attached
                if (xIsSecsSinceEpoch && (cminsky().timeUnit.empty()||cminsky().timeUnit=="year"))
                  x=yearToPTime(t);
                else
                  x=t;
                y=(*yvar)[i];
                break;
              case 1: // use the value of attached variable
                assert(xvars[0] && xvars[0]->idx()>=0);  // xvars also vector of shared pointers and null derefencing error can likewise cause crash. for ticket 1248
                if (xvars[0]->size()>1)
                  throw_error("Tensor valued x inputs not supported");
                x=(*xvars[0])[0];
                y=(*yvar)[i];
                break;
              default:
                if (port < xvars.size() && xvars[port] && xvars[port]->idx()>=0) // xvars also vector of shared pointers and null derefencing error can likewise cause crash. for ticket 1248
                  {
                    if (xvars[port]->size()>1)
                      throw_error("Tensor valued x inputs not supported");
                    x=(*xvars[port])[0];
                    y=(*yvar)[i];
                  }
                else
                  throw_error("x input not wired for port "+to_string(port+1));
                break;
              }
            addPt(pen++, x, y);
          }

    // add markers
    if (isfinite(miny) && isfinite(maxy))
      {
        for (auto& m: horizontalMarkers)
          if (auto v=cminsky().variableValues[valueId(group.lock(), ':'+m)])
            {
              auto eps=1e-4*(maxx-minx);
              // skip marker if outside plot bounds
              if (v->value()<miny||v->value()>maxy) continue;
              double x[]{minx+eps,miny-eps};
              double y[]{v->value(),v->value()};
              setPen(pen,x,y,2);
              assignSide(pen,marker);
              labelPen(pen++,v->tooltip);
            }
        for (auto& m: verticalMarkers)
          if (auto v=cminsky().variableValues[valueId(group.lock(), ':'+m)])
            {
              auto value=v->value();
              if (xIsSecsSinceEpoch && (v->units.empty() || v->units==Units("year")))
                value=yearToPTime(value);
              // skip marker if outside plot bounds
              if (value<minx||value>maxx) continue;
              auto eps=1e-4*(maxy-miny);
              double x[]{value,value};
              double y[]{miny+eps,maxy-eps};
              setPen(pen,x,y,2);
              assignSide(pen,marker);
              labelPen(pen++,v->tooltip);
            }
        removePensFrom(pen);
      }

    
    // throttle plot redraws
    static const time_duration maxWait=milliseconds(1000);
    if ((microsec_clock::local_time()-(ptime&)lastAdd) >
        min((accumulatedBlitTime-(ptime&)lastAccumulatedBlitTime) * 2, maxWait))
      {
        const ptime timerStart=microsec_clock::local_time();
        requestRedraw();
        lastAccumulatedBlitTime = accumulatedBlitTime;
        lastAdd=microsec_clock::local_time();
        accumulatedBlitTime += lastAdd - timerStart;
      }
  }

  namespace {
    struct TimeFormatter
    {
      std::string format;
      TimeFormatter(const std::string& format): format(format) {}
      std::string operator()(const string& label,double x,double y) const
      {
        ostringstream r;
        r.precision(3);
        r<<label<<":("<<str(ptime(date(1970,Jan,1))+microseconds(static_cast<long long>(1E6*x)),format)
         <<","<<y<<")";
        return r.str();
      }
    };
  }
  
  void PlotWidget::addConstantCurves()
  {
    std::vector<std::vector<std::pair<double,std::string>>> newXticks;
    
    // determine if any of the incoming vectors has a ptime-based xVector
    xIsSecsSinceEpoch=false;
    for (auto& yv: yvars)
      for (auto& i: yv)
        {
          if (i && !i->hypercube().xvectors.empty())
            {
              const auto& xv=i->hypercube().xvectors[0];
              if (xv.dimension.type==Dimension::time)
                {
                  xIsSecsSinceEpoch=true;
                  break;
                }
            }
        }

    formatter=defaultFormatter;

    if (plotType!=automatic)
      Plot::plotType=plotType;

    size_t pen=0;
    bool noLhsPens=true; // track whether any left had side ports are connected
    clearPensOnLabelling=true; // arrange for penLabels to be cleared first time an entry is added
    const OnStackExit setClearPensOnLabellingFalse([this]{clearPensOnLabelling=false;});
    
    for (size_t port=0; port<yvars.size(); ++port)
      for (size_t i=0; i<yvars[port].size(); ++i)
        if (yvars[port][i])
        {
          if (port<m_numLines) noLhsPens=false;
          auto& yv=yvars[port][i];
          if (yv->size()>0) (*yv)[0]; // ensure cachedTensors are up to date
          auto d=yv->hypercube().dims();
          if (d.empty())
            {
              if (Plot::plotType==Plot::bar)
                addPlotPt(0);
              pen++;
              continue;
            }
          // work out a reference to the x data
          vector<double> xdefault;
          double* x;
          if (port<xvars.size() && xvars[port])
            {
              if (xvars[port]->hypercube().xvectors[0].size()!=d[0])
                throw error("x vector not same length as y vectors");
              if (xvars[port]->index().empty())
                x=xvars[port]->begin();
              else
                {
                  xdefault.reserve(d[0]);
                  for (size_t i=0; i<d[0]; ++i)
                    xdefault.push_back(xvars[port]->atHCIndex(i));
                  x=xdefault.data();
                }
            }
          else
            {
              xdefault.reserve(d[0]);
              newXticks.emplace_back();
              if (yv->hypercube().rank()) // yv carries its own x-vector
                {
                  const auto& xv=yv->hypercube().xvectors[0];
                  assert(xv.size()==d[0]);
                  switch (xv.dimension.type)
                    {
                    case Dimension::string:
                      for (size_t i=0; i<xv.size(); ++i)
                        {
                          newXticks.back().emplace_back(i, str(xv[i]));
                          xdefault.push_back(i);
                        }
                      if (plotType==automatic)
                        Plot::plotType=Plot::bar;
                      break;
                    case Dimension::value:
                      if (xIsSecsSinceEpoch && xv.dimension.units=="year")
                        // interpret "year" as Gregorian year date
                        for (const auto& i: xv)
                          {
                            xdefault.push_back(yearToPTime(i.value));
                            if (abs(i.value-int(i.value))<0.05) // only label years
                              newXticks.back().emplace_back(xdefault.back(), str(int(i.value)));
                          }
                      else
                        for (const auto& i: xv)
                          xdefault.push_back(i.value);
                      if (plotType==automatic)
                        Plot::plotType=Plot::line;
                      break;
                    case Dimension::time:
                      {
                        const string format=xv.timeFormat();
                        formatter=TimeFormatter(xv.dimension.units);
                        for (const auto& i: xv)
                          {
                            const double tv=(i.time-ptime(date(1970,Jan,1))).total_microseconds()*1E-6;
                            newXticks.back().emplace_back(tv,str(i,format));
                            xdefault.push_back(tv);
                          }
                      }
                      if (plotType==automatic)
                        Plot::plotType=Plot::line;
                      break;
                    }
                }
              else // by default, set x to 0..d[0]-1
                for (size_t i=0; i<d[0]; ++i)
                  xdefault.push_back(i);
              x=xdefault.data();
            }
        
          const auto& idx=yv->index();
          if (yv->rank()==1)
            {
              // 1D data's pen attributes should match the input port
              if (idx.empty())
                setPen(pen, x, yv->begin(), d[0]);
              else
                for (size_t j=0; j<idx.size(); ++j)
                  addPt(pen,x[idx[j]], (*yv)[j]);
              pen++;
            }
          else
            {
              // higher rank y objects treated as multiple y vectors to plot
              size_t startPen=pen;
              if (idx.empty())
                for (size_t j=0 /*d[0]*/; j<std::min(maxNumTensorElementsToPlot*d[0], yv->size()); j+=d[0])
                  {
                    setPen(pen++, x, yv->begin()+j, d[0]);
                  }
              else // data is sparse
                for (size_t j=0; j<idx.size(); ++j)
                  {
                    auto div=lldiv(idx[j], d[0]);
                    if (size_t(div.quot)<maxNumTensorElementsToPlot)
                      {
                        addPt(startPen+div.quot, x[div.rem], (*yv)[j]);
                        if (pen<=startPen+div.quot) pen=startPen+div.quot+1; // track highest pen used
                      }
                  }
              // compute the pen labels
              for (int j=0; startPen<pen; ++startPen, ++j)
                {
                  string label;
                  size_t stride=1;
                  for (size_t i=1; i<yv->hypercube().rank(); ++i)
                    {
                      label+=str(yv->hypercube().xvectors[i][(j/stride)%d[i]])+" ";
                      stride*=d[i];
                    }
                  assignSide(startPen,port<m_numLines? Side::left: Side::right);
                  labelPen(startPen,defang(label));
                }
            }
        }
    justDataChanged=true;
    scalePlot();
    
    if (noLhsPens)
      {
        // set scale to RHS
        miny=miny1;
        maxy=maxy1;
      }
    
    // add markers
    if (isfinite(miny) && isfinite(maxy))
      {
        for (auto& m: horizontalMarkers)
          if (auto v=cminsky().variableValues[valueId(group.lock(), ':'+m)])
            {
              if (v->value()<miny||v->value()>maxy) continue;
              auto eps=1e-4*(maxx-minx);
              addPt(pen,minx+eps,v->value());
              addPt(pen,maxx-eps,v->value());
              assignSide(pen,marker);
              if (!v->tooltip.empty())
                labelPen(pen++,v->tooltip);
              else
                labelPen(pen++,m);
            }
        for (auto& m: verticalMarkers)
          if (auto v=cminsky().variableValues[valueId(group.lock(), ':'+m)])
            {
              auto value=v->value();
              if (xIsSecsSinceEpoch && (v->units.empty() || v->units==Units("year")))
                value=yearToPTime(value);
              if (value<minx||value>maxx) continue;
              auto eps=1e-4*(maxy-miny);
              addPt(pen,value,miny+eps);
              addPt(pen,value,maxy-eps);
              assignSide(pen,marker);
              if (!v->tooltip.empty())
                labelPen(pen++,v->tooltip);
              else
                labelPen(pen++,m);
            }
      }
        
    if (newXticks.size()==1) // nothing to disambiguate
      xticks=std::move(newXticks.front());
    else
      {
        xticks.clear();
        // now work out which xticks we'll use See Ravel #173
        for (auto& i: newXticks)
          if (i.empty())
            {
              xticks.clear();
              break; // value axes trump all
            }
          else if (xticks.empty())
            xticks=std::move(i);
          else
            {// expand range of tick labels by each pen's tick labels in turn
              auto j=i.begin();
              for (; j!=i.end(); ++j)
                if (j->first>=xticks.front().first)
                  break;
              xticks.insert(xticks.begin(), i.begin(), j);
              j=i.end();
              for (; j!=i.begin(); --j)
                if ((j-1)->first<=xticks.back().first)
                  break;
              xticks.insert(xticks.end(), j, i.end());
            }
      }

  }


  size_t PlotWidget::startPen(size_t port) const
  {
    size_t pen=0;
    for (size_t p=0; p<std::min(port, yvars.size()); ++p)
      pen+=yvars[p].size();
    return pen;
  }

  
  void PlotWidget::connectVar(const shared_ptr<VariableValue>& var, unsigned port)
  {
    assert(var);
    if (port<nBoundsPorts)
      switch (port)
        {
        case 0: xminVar=var; return;
        case 1: xmaxVar=var; return;
        case 2: yminVar=var; return;
        case 3: ymaxVar=var; return;
        case 4: y1minVar=var; return;
        case 5: y1maxVar=var; return;
        }
    if (port-nBoundsPorts<2*m_numLines)
      {
        yvars.resize(port-nBoundsPorts+1);
        yvars[port-nBoundsPorts].push_back(var);
        // assign Side::right to pens belonging to the RHS
        assignSide(startPen(port-nBoundsPorts+1)-1,port-nBoundsPorts<m_numLines? Side::left: Side::right);
      }
    else if (port-nBoundsPorts<4*m_numLines)
      {
        xvars.resize(port-nBoundsPorts-2*m_numLines+1);
        xvars[port-nBoundsPorts-2*m_numLines]=var;
      }
    justDataChanged=false;
    scalePlot();
  }

  void PlotWidget::disconnectAllVars()
  {
    xvars.clear();
    yvars.clear();
    xminVar=xmaxVar=yminVar=ymaxVar=y1minVar=y1maxVar=nullptr;
  }

  set<string> PlotWidget::availableMarkers() const
  {
    // search upwards through group heirarchy, looking for variable to add
    set<string> r;
    for (auto g=group.lock(); g; g=g->group.lock())
      for (auto& i: g->items)
        if (auto v=i->variableCast())
          r.insert(uqName(v->rawName()));
    return r;
  }
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PlotWidget);
