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

#include <ecolab_epilogue.h>
using namespace ecolab::cairo;
using namespace ecolab;
using namespace std;
using namespace boost::posix_time;

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

    /// temporarily sets nTicks and fontScale, restoring them on scope exit
    struct SetTicksAndFontSize
    {
      PlotWidget& p;
      int nxTicks, nyTicks;
      double fontScale;
      bool subgrid;
      SetTicksAndFontSize(PlotWidget& p, bool override, int n, double f, bool g):
        p(p), nxTicks(p.nxTicks), nyTicks(p.nyTicks), 
        fontScale(p.fontScale), subgrid(p.subgrid) 
      {
        if (override)
          {
            p.nxTicks=p.nyTicks=n;
            p.fontScale=f;
            p.subgrid=g;
          }
      }
      ~SetTicksAndFontSize()
      {
        p.nxTicks=nxTicks;
        p.nyTicks=nyTicks;
        p.fontScale=fontScale;
        p.subgrid=subgrid;
      }
    };

  }

  PlotWidget::PlotWidget()
  {
    // TODO assignPorts();
    nxTicks=nyTicks=10;
    fontScale=2;
    leadingMarker=true;
    grid=true;

    float w=width, h=height;
    float dx=w/(2*numLines+1); // x location of ports
    float dy = h/(numLines);

    // xmin, xmax, ymin, ymax ports
    ports.emplace_back(new Port(*this, Port::inputPort)); //xmin
    ports.emplace_back(new Port(*this, Port::inputPort));  //xmax
    ports.emplace_back(new Port(*this, Port::inputPort)); //ymin
    ports.emplace_back(new Port(*this, Port::inputPort)); //ymax
    ports.emplace_back(new Port(*this, Port::inputPort)); //y1min
    ports.emplace_back(new Port(*this, Port::inputPort)); //y1max

    // y variable ports
    for (float y=0.5*(dy-h); y<0.5*h; y+=dy)
      ports.emplace_back(new Port(*this, Port::inputPort));

    // RHS y variable ports
    for (float y=0.5*(dy-h); y<0.5*h; y+=dy)
      ports.emplace_back(new Port(*this, Port::inputPort));

    // add in the x variable ports
    for (float x=2*dx-0.5*w; x<0.5*w; x+=dx)
      ports.emplace_back(new Port(*this, Port::inputPort));

    yvars.resize(2*numLines);
    xvars.resize(numLines);
   }

  void PlotWidget::draw(cairo_t* cairo) const
  {
    double w=width*zoomFactor, h=height*zoomFactor;

    cairo_new_path(cairo);
    cairo_rectangle(cairo,-0.5*w,-0.5*h,w,h);

    cairo_clip(cairo);

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
        double fx=0, fy=titleHeight*h;
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
          ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i/2)%paletteSz], orient[i]);
        
      }
        
    // draw y data ports
    for (; i<numLines+nBoundsPorts; ++i)
      {
        float y=0.5*(dy-h) + (i-nBoundsPorts)*dy;
        if (!justDataChanged)
          ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-nBoundsPorts)%paletteSz], 0);
      }
    
    // draw RHS y data ports
    for (; i<2*numLines+nBoundsPorts; ++i)
      {
        float y=0.5*(dy-h) + (i-numLines-nBoundsPorts)*dy, x=0.5*w;
        if (!justDataChanged)
          ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-nBoundsPorts)%paletteSz], M_PI);
      }

    // draw x data ports
    for (; i<4*numLines+nBoundsPorts; ++i)
      {
        float x=dx-0.5*w + (i-2*numLines-nBoundsPorts)*dx;
        if (!justDataChanged)
          ports[i]->moveTo(x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-2*numLines-nBoundsPorts)%paletteSz], -0.5*M_PI);
      }

    cairo_translate(cairo, 10*zoomFactor,yoffs);
    cairo_set_line_width(cairo,1);
    Plot::draw(cairo,w-20*zoomFactor,h-10); // allow space for ports
    
    cairo_restore(cairo);
    if (mouseFocus)
      {
        drawPorts(cairo);
        displayTooltip(cairo);
      }
    if (selected) drawSelected(cairo);
    justDataChanged=false;
  }
  
  void PlotWidget::scalePlot()
  {
    // set any scale overrides
    setMinMax();
    if (xminVar.idx()>-1) {minx=xminVar.value()[0];}
    if (xmaxVar.idx()>-1) {maxx=xmaxVar.value()[0];}
    if (yminVar.idx()>-1) {miny=yminVar.value()[0];}
    if (ymaxVar.idx()>-1) {maxy=ymaxVar.value()[0];}
    if (y1minVar.idx()>-1) {miny1=y1minVar.value()[0];}
    if (y1maxVar.idx()>-1) {maxy1=y1maxVar.value()[0];}
    autoscale=false;

    if (!justDataChanged)
      // label pens
      for (size_t i=0; i<yvars.size(); ++i)
        if (yvars[i].idx()>=0)
          labelPen(i, latexToPango(yvars[i].name));
  }

  extern Tk_Window mainWin;

  void PlotWidget::redraw()
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
    width=abs(x.x1-x.x0);
    height=abs(x.y1-x.y0);
    bb.update(*this);
  }

  
  static ptime epoch=microsec_clock::local_time(), accumulatedBlitTime=epoch;

  void PlotWidget::addPlotPt(double t)
  {
    for (size_t pen=0; pen<2*numLines; ++pen)
      if (yvars[pen].dims().size()==1 && yvars[pen].dims()[0]==1 && yvars[pen].idx()>=0)
        {
          double x,y;
          switch (xvars.size())
            {
            case 0: // use t, when x variable not attached
              x=t;
              y=yvars[pen].value()[0];
              break;
            case 1: // use the value of attached variable
              assert(xvars[0].idx()>=0);
              x=xvars[0].value()[0];
              y=yvars[pen].value()[0];
              break;
            default:
              if (pen < xvars.size() && xvars[pen].idx()>=0)
                {
                  x=xvars[pen].value()[0];
                  y=yvars[pen].value()[0];
                }
              else
                throw error("x input not wired for pen %d",(int)pen+1);
              break;
            }
          addPt(pen, x, y);
        }
    
    // throttle plot redraws
    static time_duration maxWait=milliseconds(1000);
    if ((microsec_clock::local_time()-(ptime&)lastAdd) >
        min((accumulatedBlitTime-(ptime&)lastAccumulatedBlitTime) * 2, maxWait))
      {
        ptime timerStart=microsec_clock::local_time();
        redraw();
        lastAccumulatedBlitTime = accumulatedBlitTime;
        lastAdd=microsec_clock::local_time();
        accumulatedBlitTime += lastAdd - timerStart;
      }
  }

  void PlotWidget::addConstantCurves()
  {
    for (size_t pen=0; pen<2*numLines; ++pen)
      if (yvars[pen].numElements()>1 && yvars[pen].idx()>=0)
        {
          auto& d=yvars[pen].dims();
          if (pen<xvars.size() && xvars[pen].numElements()==yvars[pen].numElements())
            setPen(pen, xvars[pen].value(), yvars[pen].value());
          else if (d.size()>1 && d[0]==d[1])
            {
              // yvar carries the x data
              auto y=yvars[pen].value();
              setPen(pen, vector<double>(y.begin(),y.begin()+d[0]),
                     vector<double>(y.begin()+d[0],y.begin()+2*d[0]));
            }
        }
  }

  
  void PlotWidget::connectVar(const VariableValue& var, unsigned port)
  {
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
    unsigned pen=port-nBoundsPorts;
    if (pen<2*numLines)
      {
        yvars.resize(2*numLines);
        yvars[pen]=var;
        if (pen>=numLines)
          assignSide(pen,right);
      }
    else if (pen<4*numLines)
      {
        xvars.resize(2*numLines);
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
