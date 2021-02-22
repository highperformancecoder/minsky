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
#include "plotWidget.h"
#include "variable.h"
#include "cairoItems.h"
#include "minsky.h"
#include "latexMarkup.h"
#include "pango.h"
#include <timer.h>
#include <cairo/cairo-ps.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>

#include "minsky_epilogue.h"
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
    const unsigned numLines = 4; // number of simultaneous variables to plot, on a side

    const unsigned nBoundsPorts=6;
    // orientation of bounding box ports
    const double orient[nBoundsPorts]={-0.4*M_PI, -0.6*M_PI, -0.2*M_PI, 0.2*M_PI, 1.2*M_PI, 0.8*M_PI};
    // x coordinates of bounding box ports
    const float boundX[nBoundsPorts]={-0.46,0.45,-0.49,-0.49, 0.48, 0.48};
    // y coordinates of bounding box ports
    const float boundY[nBoundsPorts]={0.49,0.49,0.47,-0.49, 0.47, -0.49};

    // height of title, as a fraction of overall widget height
    const double titleHeight=0.07;

  }

  PlotWidget::PlotWidget()
  {
    m_width=m_height=150;
    nxTicks=nyTicks=10;
    fontScale=2;
    leadingMarker=true;
    grid=true;
    legendLeft=0.1; // override ecolab's default value

    float w=iWidth(), h=iHeight();
    float dx=w/(2*numLines+1); // x location of ports
    float dy = h/(numLines);

    // xmin, xmax, ymin, ymax ports
    m_ports.emplace_back(new Port(*this, Port::inputPort)); //xmin
    m_ports.emplace_back(new Port(*this, Port::inputPort));  //xmax
    m_ports.emplace_back(new Port(*this, Port::inputPort)); //ymin
    m_ports.emplace_back(new Port(*this, Port::inputPort)); //ymax
    m_ports.emplace_back(new Port(*this, Port::inputPort)); //y1min
    m_ports.emplace_back(new Port(*this, Port::inputPort)); //y1max

    // y variable ports
    for (float y=0.5*(dy-h); y<0.5*h; y+=dy)
      m_ports.emplace_back(new Port(*this, Port::inputPort));

    // RHS y variable ports
    for (float y=0.5*(dy-h); y<0.5*h; y+=dy)
      m_ports.emplace_back(new Port(*this, Port::inputPort));

    // add in the x variable ports
    for (float x=2*dx-0.5*w; x<0.5*w; x+=dx)
      m_ports.emplace_back(new Port(*this, Port::inputPort));

    yvars.resize(2*numLines);
    xvars.resize(numLines);
   }

  void PlotWidget::draw(cairo_t* cairo) const
  {
    double z=zoomFactor();
    double w=iWidth()*z, h=iHeight()*z;

    // if any titling, draw an extra bounding box (ticket #285)
    if (!title.empty()||!xlabel.empty()||!ylabel.empty()||!y1label.empty())
      {
        cairo_rectangle(cairo,-0.5*w+10,-0.5*h,w-20,h-10);
        cairo_set_line_width(cairo,1);
        cairo_stroke(cairo);
      }

    cairo_save(cairo);
    cairo_translate(cairo,-0.5*w,-0.5*h);

    double yoffs=0; // offset to allow for title
    if (!title.empty())
      {
        double fx=0, fy=titleHeight*iHeight()*z;
        cairo_user_to_device_distance(cairo,&fx,&fy);
        
        Pango pango(cairo);
        pango.setFontSize(fabs(fy));
        pango.setMarkup(latexToPango(title));   
        cairo_set_source_rgb(cairo,0,0,0);
        cairo_move_to(cairo,0.5*(w-pango.width()), 0/*pango.height()*/);
        pango.show();

        // allow some room for the title
        yoffs=1.2*pango.height();
        h-=1.2*pango.height();
      }

    // draw bounding box ports
    float x = -0.5*w, dx=w/(2*numLines+1); // x location of ports
    float y=0.5*h, dy = h/(numLines);
    
    size_t i=0;
    // draw bounds input ports
    for (; i<nBoundsPorts; ++i)
      {
        float x=boundX[i]*w, y=boundY[i]*h;
        if (!justDataChanged)
          m_ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i/2)%palette.size()].colour, orient[i]);
        
      }
        
    // draw y data ports
    for (; i<numLines+nBoundsPorts; ++i)
      {
        float y=0.5*(dy-h) + (i-nBoundsPorts)*dy;
        if (!justDataChanged)
          m_ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-nBoundsPorts)%palette.size()].colour, 0);
      }
    
    // draw RHS y data ports
    for (; i<2*numLines+nBoundsPorts; ++i)
      {
        float y=0.5*(dy-h) + (i-numLines-nBoundsPorts)*dy, x=0.5*w;
        if (!justDataChanged)
          m_ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-nBoundsPorts)%palette.size()].colour, M_PI);
      }

    // draw x data ports
    for (; i<4*numLines+nBoundsPorts; ++i)
      {
        float x=dx-0.5*w + (i-2*numLines-nBoundsPorts)*dx;
        if (!justDataChanged)
          m_ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-2*numLines-nBoundsPorts)%palette.size()].colour, -0.5*M_PI);
      }

    cairo_translate(cairo, portSpace, yoffs);
    cairo_set_line_width(cairo,1);
    double gw=w-2*portSpace, gh=h-portSpace;;
    if (!title.empty()) gh=h-portSpace-titleHeight;  // take into account room for the title
    //TODO Urgh - fix up the const_casts here. Maybe pass plotType as parameter to draw
    auto& pt=const_cast<Plot*>(static_cast<const Plot*>(this))->plotType;
    switch (plotType)
      {
      case line: pt=Plot::line; break;
      case bar:  pt=Plot::bar;  break;
      default: break;
      }

    Plot::draw(cairo,gw,gh); 
    cairo_restore(cairo);
    if (mouseFocus)
      {
        drawPorts(cairo);
        displayTooltip(cairo,tooltip);
        // draw legend tags for move/resize
        if (legend)
          {
            double width,height;
            legendSize(width,height,gh);
            width+=legendOffset*gw;

            
            double x=legendLeft*gw-0.5*(w-width)+portSpace;
            double y=-legendTop*gh+0.5*(h+height)-portSpace;
            if (!title.empty()) y=-legendTop*gh+0.5*(h+height)-portSpace+titleHeight*h; // take into account room for the title
            double arrowLength=6;
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
    if (xminVar.idx()>-1)
      {
        if (xIsSecsSinceEpoch && xminVar.units==Units("year"))
          minx=yearToPTime(xminVar.value());
        else
          minx=xminVar.value();
      }

    if (xmaxVar.idx()>-1)
      {
        if (xIsSecsSinceEpoch && xmaxVar.units==Units("year"))
          maxx=yearToPTime(xmaxVar.value());
        else
          maxx=xmaxVar.value();
      }

    if (yminVar.idx()>-1) {miny=yminVar.value();}
    if (ymaxVar.idx()>-1) {maxy=ymaxVar.value();}
    if (y1minVar.idx()>-1) {miny1=y1minVar.value();}
    if (y1maxVar.idx()>-1) {maxy1=y1maxVar.value();}
    autoscale=false;

    if (!justDataChanged)
      // label pens
      for (size_t i=0; i<yvars.size(); ++i)
        if (yvars[i] && !yvars[i]->name.empty())
          labelPen(i, latexToPango(yvars[i]->name));
  }

  void PlotWidget::mouseDown(double x,double y)
  {
    clickX=x;
    clickY=y;
    ct=clickType(x,y);
    double z=zoomFactor();
    double gw=iWidth()*z-2*portSpace;
    double gh=iHeight()*z-portSpace;
    if (!title.empty()) gh=iHeight()*z-portSpace-titleHeight;
    oldLegendLeft=legendLeft*gw+portSpace;
    oldLegendTop=legendTop*gh;
    oldLegendFontSz=legendFontSz;
  }
  
  void PlotWidget::mouseMove(double x,double y)
  {
    double z=zoomFactor();
    double w=0.5*iWidth()*z, h=0.5*iHeight()*z;
    double dx=x-this->x(), dy=y-this->y();
    double gw=iWidth()*z-2*portSpace;
    double gh=iHeight()*z-portSpace;
    if (!title.empty()) gh=iHeight()*z-portSpace-titleHeight;
    double yoffs=this->y()-(legendTop-0.5)*iHeight()*z;
    switch (ct)
      {
      case ClickType::legendMove:
        if (abs(dx)<w && abs(dy)<h) {  //Ensure plot legend cannot be moved off plot widget  
           legendLeft = (oldLegendLeft + x - clickX-portSpace)/gw;
           legendTop = (oldLegendTop + clickY - y)/gh;
           if (!title.empty()) legendTop = (oldLegendTop + clickY - y + titleHeight)/gh;
	    } else {
			legendLeft = oldLegendLeft/gw;
			legendTop = oldLegendTop/gh;
		}
        break;
      case ClickType::legendResize:
        if (abs(dx)<w && abs(dy)<h) { //Ensure plot legend cannot be resized beyond extent of plot widget  
           legendFontSz = oldLegendFontSz * (y-yoffs)/(clickY-yoffs);
           if (!title.empty()) legendFontSz = oldLegendFontSz * (y-yoffs+titleHeight)/(clickY-yoffs);
	    } else legendFontSz = oldLegendFontSz;
        break;
      default:
        break;
      }
  }

  extern Tk_Window mainWin;

  void PlotWidget::requestRedraw()
  {
    justDataChanged=true; // assume plot same size, don't do unnecessary stuff
    // store previous min/max values to determine if plot scale changes
    scalePlot();
    if (surface.get())
      surface->requestRedraw();
  }

  void PlotWidget::makeDisplayPlot() {
    if (auto g=group.lock())
      g->displayPlot=dynamic_pointer_cast<PlotWidget>(g->findItem(*this));
  }

  void PlotWidget::resize(const LassoBox& x)
  {
    float invZ=1/zoomFactor();
    iWidth(abs(x.x1-x.x0)*invZ);
    iHeight(abs(x.y1-x.y0)*invZ);
    moveTo(0.5*(x.x0+x.x1), 0.5*(x.y0+x.y1));
    bb.update(*this);
  }

  // specialisation to avoid rerendering plots (potentially expensive)
  ClickType::Type PlotWidget::clickType(float x, float y)
  {
    // firstly, check whether a port has been selected
    double z=zoomFactor();  
    for (auto& p: m_ports)
      {
        if (hypot(x-p->x(), y-p->y()) < portRadius*z)
          return ClickType::onPort;
      }

    double legendWidth, legendHeight;
    legendSize(legendWidth, legendHeight, iHeight()*z-portSpace);
    double xx= x-this->x() - portSpace +(0.5-legendLeft)*iWidth()*z;
    double yy= y-this->y() + (legendTop-0.5)*iHeight()*z;
    if (xx>0 && xx<legendWidth)
      {
        if (yy>0 && yy<0.8*legendHeight)
          return ClickType::legendMove;
        else if (yy>=0.8*legendHeight && yy<legendHeight)
          return ClickType::legendResize;
      }

    if (onResizeHandle(x,y)) return ClickType::onResize;         
	
    double dx=x-this->x(), dy=y-this->y();
    double w=0.5*iWidth()*z, h=0.5*iHeight()*z;
    return (abs(dx)<w && abs(dy)<h)?
      ClickType::onItem: ClickType::outside;
  }
  
  static ptime epoch=microsec_clock::local_time(), accumulatedBlitTime=epoch;

  static const size_t maxNumTensorElementsToPlot=10;
  
  void PlotWidget::addPlotPt(double t)
  {
    size_t extraPen=2*numLines+1;
    for (size_t pen=0; pen<2*numLines; ++pen)
      if (pen<yvars.size() && yvars[pen])
        for (size_t i=0; i<min(maxNumTensorElementsToPlot,yvars[pen]->size()); ++i)
          {
            double x,y;
            switch (xvars.size())
              {
              case 0: // use t, when x variable not attached
                x=t;
                y=(*yvars[pen])[i];
                break;
              case 1: // use the value of attached variable
                assert(xvars[0] && xvars[0]->idx()>=0);  // xvars also vector of shared pointers and null derefencing error can likewise cause crash. for ticket 1248
                if (xvars[0]->size()>1)
                  throw_error("Tensor valued x inputs not supported");
                x=(*xvars[0])[0];
                y=(*yvars[pen])[i];
                break;
              default:
                if (pen < xvars.size() && xvars[pen] && xvars[pen]->idx()>=0) // xvars also vector of shared pointers and null derefencing error can likewise cause crash. for ticket 1248
                  {
                    if (xvars[pen]->size()>1)
                      throw_error("Tensor valued x inputs not supported");
                    x=(*xvars[pen])[0];
                    y=(*yvars[pen])[i];
                  }
                else
                  throw_error("x input not wired for pen "+to_string(pen+1));
                break;
              }
            size_t p=pen;
            if (i>0)
              {
                // ensure next pen is a different colour
                if (extraPen%(2*numLines)==pen%(2*numLines))
                  extraPen++;
                p+=extraPen++;
              }
            addPt(p, x, y);
          }
    
    // throttle plot redraws
    static time_duration maxWait=milliseconds(1000);
    if ((microsec_clock::local_time()-(ptime&)lastAdd) >
        min((accumulatedBlitTime-(ptime&)lastAccumulatedBlitTime) * 2, maxWait))
      {
        ptime timerStart=microsec_clock::local_time();
        requestRedraw();
        lastAccumulatedBlitTime = accumulatedBlitTime;
        lastAdd=microsec_clock::local_time();
        accumulatedBlitTime += lastAdd - timerStart;
      }
  }

  void PlotWidget::addConstantCurves()
  {
    size_t extraPen=2*numLines;

    // determine if any of the incoming vectors has a ptime-based xVector
    xIsSecsSinceEpoch=false;
    for (auto& i: yvars)
      if (i && xvars[&i-&yvars[0]] && i->hypercube().xvectors.size())
        {
          auto& xv=i->hypercube().xvectors[0];
          if (xv.dimension.type==Dimension::time)
            xIsSecsSinceEpoch=true;
        }
    
    for (size_t pen=0; pen<2*numLines; ++pen)
      if (pen<yvars.size() && yvars[pen] && yvars[pen]->size()>1)
        {
          auto& yv=yvars[pen];
          auto d=yv->hypercube().dims();
          if (d.empty()) continue;
          
          // work out a reference to the x data
          vector<double> xdefault;
          double* x;
          if (pen<xvars.size() && xvars[pen])
            {
              if (xvars[pen]->hypercube().xvectors[0].size()!=d[0])
                throw error("x vector not same length as y vectors");
              x=xvars[pen]->begin();
            }
          else
            {
              xdefault.reserve(d[0]);
              xticks.clear();
              if (yv->hypercube().rank()) // yv carries its own x-vector
                {
                  auto& xv=yv->hypercube().xvectors[0];
                  assert(xv.size()==d[0]);
                  switch (xv.dimension.type)
                    {
                    case Dimension::string:
                      for (size_t i=0; i<xv.size(); ++i)
                        {
                          xticks.emplace_back(i, str(xv[i]));
                          xdefault.push_back(i);
                        }
                      if (plotType==automatic)
                        Plot::plotType=Plot::bar;
                      break;
                    case Dimension::value:
                      if (xIsSecsSinceEpoch && xv.dimension.units=="year")
                        // interpret "year" as years since epoch (1/1/1970)
                        for (auto& i: xv)
                          xdefault.push_back(yearToPTime(any_cast<double>(i)));
                      else
                        for (auto& i: xv)
                          xdefault.push_back(any_cast<double>(i));
                      if (plotType==automatic)
                        Plot::plotType=Plot::line;
                      break;
                    case Dimension::time:
                      {
                        string format=xv.timeFormat();
                        for (auto& i: xv)
                          {
                            double tv=(any_cast<ptime>(i)-ptime(date(1970,Jan,1))).total_microseconds()*1E-6;
                            xticks.emplace_back(tv,str(i,format));
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
              x=&xdefault[0];
            }
          
          // higher rank y objects treated as multiple y vectors to plot
          auto startPen=extraPen;
          auto& idx=yv->index();
          if (idx.empty())
            for (size_t j=0 /*d[0]*/; j<std::min(maxNumTensorElementsToPlot*d[0], yv->size()); j+=d[0])
              {
                setPen(extraPen, x, yv->begin()+j, d[0]);
                extraPen++;
              }
          else // data is sparse
            for (size_t j=0; j<idx.size(); ++j)
              {
                auto div=lldiv(idx[j], d[0]);
                if (size_t(div.quot)<maxNumTensorElementsToPlot)
                  {
                    addPt(startPen+div.quot, x[div.rem], (*yv)[j]);
                    if (extraPen<=startPen+div.quot) extraPen=startPen+div.quot+1;
                  }
              }
          // compute the pen labels
          for (int j=0; startPen<extraPen; ++startPen, ++j)
            {
              string label;
              size_t stride=1;
              for (size_t i=1; i<yv->hypercube().rank(); ++i)
                {
                  label+=str(yv->hypercube().xvectors[i][(j/stride)%d[i]])+" ";
                  stride*=d[i];
                }
              if (pen>=numLines)
                assignSide(extraPen,Side::right);
              labelPen(startPen,defang(label));
            }
        }
    scalePlot();
  }

  
  void PlotWidget::connectVar(const shared_ptr<VariableValue>& var, unsigned port)
  {
    assert(var);
    if (port<nBoundsPorts)
      switch (port)
        {
        case 0: xminVar=*var; return;
        case 1: xmaxVar=*var; return;
        case 2: yminVar=*var; return;
        case 3: ymaxVar=*var; return;
        case 4: y1minVar=*var; return;
        case 5: y1maxVar=*var; return;
        }
    unsigned pen=port-nBoundsPorts;
    if (pen<2*numLines)
      {
        yvars.resize(pen+1);
        yvars[pen]=var;
        if (pen>=numLines)
          assignSide(pen,Side::right);
      }
    else if (pen<4*numLines)
      {
        xvars.resize(pen-2*numLines+1);
        xvars[pen-2*numLines]=var;
      }
    scalePlot();
  }

  void PlotWidget::disconnectAllVars()
  {
    xvars.clear();
    yvars.clear();
    xminVar=xmaxVar=yminVar=ymaxVar=y1minVar=y1maxVar=VariableValue();
  }

}
