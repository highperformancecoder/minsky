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
#include "init.h"
#include "cairoItems.h"
#include "minsky.h"
#include "latexMarkup.h"
#include "pango.h"
#include <timer.h>

#ifdef CAIRO_HAS_WIN32_SURFACE
#include <cairo/cairo-win32.h>
#endif
#if defined(CAIRO_HAS_WIN32_SURFACE) && !defined(__CYGWIN__)
#include <tkPlatDecls.h>
#endif

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

    Plots& plots() {return minsky::minsky().plots;}

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

    // we need some extra fields to handle the additional options
    struct TkMinskyItem: public ImageItem
    {
      int id; // identifier of the C++ object this item represents
    };

    static Tk_CustomOption tagsOption = {
      (Tk_OptionParseProc *) Tk_CanvasTagsParseProc,
      Tk_CanvasTagsPrintProc, (ClientData) NULL
    };

    struct MinskyItemImage: public CairoImage
    {
      static Tk_ConfigSpec configSpecs[];
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    Tk_ConfigSpec MinskyItemImage::configSpecs[] =
    {
      {TK_CONFIG_INT, "-id", NULL, NULL,
       NULL, Tk_Offset(TkMinskyItem, id), 0},
      {TK_CONFIG_CUSTOM, "-tags", NULL, NULL,
       NULL, 0, TK_CONFIG_NULL_OK, &tagsOption},
      {TK_CONFIG_END}
    };
#pragma GCC diagnostic pop
  }

  struct PlotItem: public MinskyItemImage
  {
    PlotWidget* pw; // weak ref to plotWidget controlling this item
    PlotItem(): pw(0) {}
    void init(int id, Tk_Canvas canvas)
    {
      pw=&plots()[id];
      pw->canvas=canvas;
    }
    void draw()
    {
      if (cairoSurface && pw)
        {
          pw->cairoSurface=cairoSurface;
          xScale=yScale=pw->zoomFactor;
          pw->draw(*cairoSurface);
        }
    }
  };

  namespace 
  {
    int creatProc(Tcl_Interp *interp, Tk_Canvas canvas, 
                  Tk_Item *itemPtr, int objc,Tcl_Obj *CONST objv[])
    {
      TkMinskyItem* tkMinskyItem=(TkMinskyItem*)(itemPtr);
      tkMinskyItem->id=-1;
      int r=createImage<PlotItem>(interp,canvas,itemPtr,objc,objv);
      if (r==TCL_OK && tkMinskyItem->id>=0)
        {
          PlotItem* p=static_cast<PlotItem*>(tkMinskyItem->cairoItem);
          if (p)
            {
              p->init(tkMinskyItem->id, canvas);
              TkImageCode::ComputeImageBbox(canvas, tkMinskyItem);
            }
          else
            return TCL_ERROR;
        }
      return r;
    }

  // overrride cairoItem's configureProc to process the extra config options
  int configureProc(Tcl_Interp *interp,Tk_Canvas canvas,Tk_Item *itemPtr,
                    int objc,Tcl_Obj *CONST objv[],int flags)
  {
    return TkImageCode::configureCairoItem
      (interp,canvas,itemPtr,objc,objv,flags, MinskyItemImage::configSpecs);
  }
  
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
    // register PlotItem with Tk for use in canvases.
    int registerPlotItem()
    {
      static Tk_ItemType plotItemType = cairoItemType();
      plotItemType.name="plot";
      plotItemType.itemSize=sizeof(TkMinskyItem);
      plotItemType.createProc=creatProc;
      plotItemType.configProc=configureProc;
      plotItemType.configSpecs=MinskyItemImage::configSpecs;
      Tk_CreateItemType(&plotItemType);
      return 0;
    }
  
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

  }

  static int reg=(initVec().push_back(registerPlotItem), 0);

  PlotWidget::PlotWidget()
  {
    // TODO assignPorts();
    nxTicks=nyTicks=10;
    fontScale=1;
    leadingMarker=true;
    grid=true;
  }

  PlotWidget::~PlotWidget()
  {
    // TODO deletePorts();
  }

  void PlotWidget::assignPorts()
  {
    float w=width, h=height;
    float x = -0.5*w, dx=w/(2*numLines+1); // x location of ports
    float y=0.5*h, dy = h/(numLines);
    m_ports.resize(0);

    // xmin, xmax, ymin, ymax ports
    m_ports<<=minsky().addInputPort(); //xmin
    m_ports<<=minsky().addInputPort();  //xmax
    m_ports<<=minsky().addInputPort(); //ymin
    m_ports<<=minsky().addInputPort(); //ymax
    m_ports<<=minsky().addInputPort(); //ymax
    m_ports<<=minsky().addInputPort(); //ymax

    // y variable ports
    for (float y=0.5*(dy-h); y<0.5*h; y+=dy)
      m_ports<<=minsky().addPort(Port(x+m_x,y+m_y,true));

    // RHS y variable ports
    for (float y=0.5*(dy-h); y<0.5*h; y+=dy)
      m_ports<<=minsky().addPort(Port(0.5*w+m_x,y+m_y,true));

    // add in the x variable ports
    for (float x=2*dx-0.5*w; x<0.5*w; x+=dx)
      m_ports<<=minsky().addPort(Port(x+m_x,y+m_y,true));


  }

  void PlotWidget::draw(cairo::Surface& cairoSurface)
  {
    displayNTicks = min(10.0f, 3*zoomFactor);
    displayFontSize = 9.0/displayNTicks;
    SetTicksAndFontSize stf
      (*this, true, displayNTicks, displayFontSize, false);
    if (!justDataChanged) scalePlot(); // already scaled in redraw

    cairoSurface.clear();
    draw(cairoSurface.cairo());
    justDataChanged=false; // justDataChanged is a single shot
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
          minsky().movePortTo(m_ports[i], x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i/2)%paletteSz], orient[i]);
        
      }
        
    // draw y data ports
    for (; i<numLines+nBoundsPorts; ++i)
      {
        float y=0.5*(dy-h) + (i-nBoundsPorts)*dy;
        if (!justDataChanged)
          minsky().movePortTo(m_ports[i], x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-nBoundsPorts)%paletteSz], 0);
      }
    
    // draw RHS y data ports
    for (; i<2*numLines+nBoundsPorts; ++i)
      {
        float y=0.5*(dy-h) + (i-numLines-nBoundsPorts)*dy, x=0.5*w;
        if (!justDataChanged)
          minsky().movePortTo(m_ports[i],  x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-nBoundsPorts)%paletteSz], M_PI);
      }

    // draw x data ports
    for (; i<4*numLines+nBoundsPorts; ++i)
      {
        float x=dx-0.5*w + (i-2*numLines-nBoundsPorts)*dx;
        if (!justDataChanged)
          minsky().movePortTo(m_ports[i], x + this->x(), y + this->y()+0.5*yoffs);
        drawTriangle(cairo, x+0.5*w, y+0.5*h+yoffs, palette[(i-2*numLines-nBoundsPorts)%paletteSz], -0.5*M_PI);
      }

    cairo_translate(cairo, 10*zoomFactor,yoffs);
    cairo_set_line_width(cairo,1);
    Plot::draw(cairo,w-20*zoomFactor,h-yoffs);
    
    cairo_restore(cairo);
    if (mouseFocus)
      drawPorts(*this, ports(), cairo);
    if (selected) drawSelected(cairo);
  }
  

  /// TODO: better handled in a destructor, but for the moment, the
  /// software architecture requires that this be called explicitly from
  /// Minsky::deletePlot
  void PlotWidget::deletePorts()
  {
    for (int p: m_ports)
      minsky().delPort(p);
    m_ports.resize(0);
  }

  void PlotWidget::moveTo(float x1, float y1)
  {
    float w=width, h=height;
    float dx=x1-x(), dy=y1-y();
    m_x=x1; m_y=y1;
    for (int p: m_ports)
      minsky().movePort(p, dx, dy);
  }

  void PlotWidget::scalePlot()
  {
    // set any scale overrides
    setMinMax();
    if (xminVar.idx()>-1) {minx=xminVar.value();}
    if (xmaxVar.idx()>-1) {maxx=xmaxVar.value();}
    if (yminVar.idx()>-1) {miny=yminVar.value();}
    if (ymaxVar.idx()>-1) {maxy=ymaxVar.value();}
    if (y1minVar.idx()>-1) {miny1=y1minVar.value();}
    if (y1maxVar.idx()>-1) {maxy1=y1maxVar.value();}
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
    double minmax[]={minx,maxx,miny,maxy,miny1,maxy1};
    scalePlot();
#if defined(CAIRO_HAS_WIN32_SURFACE) && !defined(__CYGWIN__)
    // draw directly into the Window for performance
    Tk_Window win=Tk_CanvasTkwin(canvas);
    HWND hwnd=Tk_GetHWND(Tk_WindowId(win));// Hwnd for the canvas
    HDC hdc=GetDC(hwnd); 
    Surface surf(cairo_win32_surface_create(hdc));
    short drawableX, drawableY;
    Tk_CanvasWindowCoords(canvas, x(), y(), &drawableX, &drawableY);
//    ::Rectangle(hdc,drawableX-(width*zoomFactor)/2,drawableY-(height*zoomFactor)/2,drawableX+(width*zoomFactor)/2,
//                drawableY+(height*zoomFactor)/2);
    cairo_surface_set_device_offset(surf.surface(), drawableX, drawableY);

    if (minx!=minmax[0] || maxx!=minmax[1] || miny!=minmax[2] || maxy!=minmax[3]
        || miny1!=minmax[4] || maxy1!=minmax[5])
    {
      // clear the area about to be drawn into
      tclcmd cmd;
      cmd<<".wiring.canvas cget -background\n";
      XColor* colour=Tk_GetColor(interp(),win,cmd.result.c_str());
      cairo_save(surf.cairo());
      cairo_set_source_rgb(surf.cairo(),1.526e-5*colour->red,1.526e-5*colour->green,1.526e-5*colour->blue);
      Tk_FreeColor(colour);
      cairo_rectangle(surf.cairo(),-(width*zoomFactor)/2,-(height*zoomFactor)/2,width*zoomFactor,height*zoomFactor);
      cairo_fill(surf.cairo());
      cairo_restore(surf.cairo());
    }

    draw(surf);
    ReleaseDC(hwnd,hdc);
#else
    if (cairoSurface.get())
      cairoSurface->requestRedraw();
#endif
    if (expandedPlot.get())
      {
        expandedPlot->clear();
        Plot::draw(*expandedPlot);
        expandedPlot->blit();
      }
  }

  static ptime epoch=microsec_clock::local_time(), accumulatedBlitTime=epoch;

  void PlotWidget::addPlotPt(double t)
  {
    for (size_t pen=0; pen<yvars.size(); ++pen)
      if (yvars[pen].idx()>=0)
        {
          double x,y;
          switch (xvars.size())
            {
            case 0: // use t, when x variable not attached
              x=t;
              y=yvars[pen].value();
              break;
            case 1: // use the value of attached variable
              assert(xvars[0].idx()>=0);
              x=xvars[0].value();
              y=yvars[pen].value();
              break;
            default:
              if (pen < xvars.size() && xvars[pen].idx()>=0)
                {
                  x=xvars[pen].value();
                  y=yvars[pen].value();
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

  static VariableValue disconnected;
  constexpr float PlotWidget::rotation;

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
          AssignSide(pen,right);
      }
    else if (pen<4*numLines)
      {
        xvars.resize(2*numLines);
        xvars[pen-2*numLines]=var;
      }
  }

  void PlotWidget::updatePortLocation()
  {
    double w = 150;    //   150 from constructor
    double h = 150;    //   no zoom.

    // bounding box ports
    float x = -0.5 * w, dx = w / numLines; // x location of ports
    float y =  0.5 * h, dy = h / numLines;

    // bounds input ports
    size_t i;
    for (i = 0; i < 4; ++i)
      {
        float x=boundX[i]*w, y=boundY[i]*h;
        minsky().movePortTo(m_ports[i], x + this->x(), y + this->y());
      }

    // y data ports
    for ( ; i < (numLines + 4); ++i)
      {
        float y=0.5*(dy-h) + (i-4)*dy;
        minsky().movePortTo(m_ports[i], x + this->x(), y + this->y());
      }

    // x data ports
    for ( ; i < ((2 * numLines) + 4); ++i)
      {
        float x = 0.5 * (dx - w) + (i - numLines - 4) * dx;
        minsky().movePortTo(m_ports[i], x + this->x(), y + this->y());
      }

  }

  void Plots::addImage(int id, const string& image) 
  {
    PlotWidget& p=(*this)[id];
    p.expandedPlot.reset
      (new TkPhotoSurface(Tk_FindPhoto(interp(), image.c_str()), false));
    cairo_surface_set_device_offset(p.expandedPlot->surface(),p.expandedPlot->width()/2,p.expandedPlot->height()/2);
    p.redraw();
  }

  void Plots::reset()
  {
    for (iterator p=begin(); p!=end(); ++p)
      p->clear();
  }

}
