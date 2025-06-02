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
#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H
#include <cairo_base.h>
#include "renderNativeWindow.h"
#include "classdesc_access.h"
#include "latexMarkup.h"
#include "plot.h"
#include "variable.h"
#include "variableValue.h"
#include "zoom.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

namespace minsky
{
  // seconds in a year. Gregorian year chosen.
  const double yearLength = 3600*24*365.2525;
  inline double yearToPTime(double x) {return (x-1970)*yearLength;}
  
  using namespace ecolab;

  struct PlotWidgetSuper: public ItemT<PlotWidget>,
                          public ecolab::Plot, public RenderNativeWindow {};

  /// a container item for a plot widget
  class PlotWidget: public PlotWidgetSuper
  {
    static constexpr double portSpace=10; // allow space for ports
    double clickX, clickY, oldLegendLeft, oldLegendTop, oldLegendFontSz;
    ClickType::Type ct;
    CLASSDESC_ACCESS(PlotWidget);
    friend struct SchemaHelper;
    // timestamp of last time this widget was blitted and also the
    // accumulated blit time at last add.
    classdesc::Exclude<boost::posix_time::ptime> 
      lastAdd{boost::posix_time::microsec_clock::local_time()},
      lastAccumulatedBlitTime{boost::posix_time::microsec_clock::local_time()};
    // overrides placement of ports etc when just data has
    // changed. Single shot flag that is reset after the next call to
    // draw(), which is const, so this attribute needs to be mutable.
    mutable bool justDataChanged=false;
    friend struct PlotItem;

    bool xIsSecsSinceEpoch=false;
    bool redraw(int, int, int, int) override;
    using Plot::redraw;

    // shadow labels, so we can interpret as LaTeX code rather than Pango markup
    std::string m_xlabel, m_ylabel, m_y1label;

    /// extra offset for titles
    mutable double yoffs=0;

    Formatter formatter=defaultFormatter;
    size_t m_numLines=1; ///< number of ports on the side

    /// returns to starting pen number for input \a port
    size_t startPen(size_t port) const;

    bool clearPensOnLabelling=false;
    
  public:
    using Item::x;
    using Item::y;
    using RenderNativeWindow::surface;
    using RenderNativeWindow::requestRedraw;
    using RenderNativeWindow::renderToSVG;

    /// number of bounds (xmin/xmax, ymin/ymax, y1min/y1max) ports
    static constexpr unsigned nBoundsPorts=6;

    /// variable port attached to (if any)
    std::vector<std::vector<std::shared_ptr<VariableValue>>> yvars;
    std::vector<std::shared_ptr<VariableValue>> xvars;

    // cache penLabels to label pen styles dialog
    std::vector<std::string> penLabels;
    void labelPen(size_t pen, const std::string& label) {
      if (clearPensOnLabelling) {penLabels.clear(); clearPensOnLabelling=false;}
      if (penLabels.size()<=pen+1) penLabels.resize(pen+1);
      penLabels[pen]=label;
      Plot::labelPen(pen,latexToPango(label));
    }

    /// variable ports specifying plot size
    std::shared_ptr<VariableValue> xminVar, xmaxVar, yminVar, ymaxVar, y1minVar, y1maxVar;
    /// Settable bounds values if above variables not connected
    /// NaN means autocalculate from data
    double xmin=nan(""), xmax=xmin, ymin=xmin, ymax=xmin, y1min=xmin, y1max=xmin;
    /// number of ticks to show in canvas item
    unsigned displayNTicks{3};
    double displayFontSize{3};
    /// markers at a position given by a named variable/parameter 
    std::vector<std::string> horizontalMarkers;
    std::vector<std::string> verticalMarkers;

    std::string title;

    /// automatic means choose line or bar depending on the x-vector type.
    PlotType plotType=automatic;
    
    PlotWidget();
    void addPorts();
    PlotWidget(const PlotWidget& x): PlotWidgetSuper(x) {addPorts();}
    PlotWidget(PlotWidget&& x): PlotWidgetSuper(x) {addPorts();}

    /// @{ number of input ports along a side
    size_t numLines() const {return m_numLines;}
    size_t numLines(size_t n);
    /// @}
    
    // pick the Item width method, not ecolab::Plot's
    float width() const {return Item::width();}
    float height() const {return Item::height();}
    
    const PlotWidget* plotWidgetCast() const override {return this;}
    PlotWidget* plotWidgetCast() override {return this;}          

    /// shadowed label commands to allow latex intepretation
    std::string const& xlabel() const {return m_xlabel;}
    std::string const& xlabel(const std::string& x) {
      ecolab::Plot::xlabel=latexToPangoNonItalicised(x);
      return m_xlabel=x;
    }
    std::string const& ylabel() const {return m_ylabel;}
    std::string const& ylabel(const std::string& x) {
      ecolab::Plot::ylabel=latexToPangoNonItalicised(x);
      return m_ylabel=x;
    }
    std::string const& y1label() const {return m_y1label;}
    std::string const& y1label(const std::string& x) {
      ecolab::Plot::y1label=latexToPangoNonItalicised(x);
      return m_y1label=x;
    }

    /// @{ bar chart bar width control
    // for now, set all bar width to the same value
    double barWidth() const;
    double barWidth(double w);
    /// @}
    
    void addPlotPt(double t); ///< add another plot point
    void updateIcon(double t) override {addPlotPt(t);}
    /// add vector/tensor curves to plot
    void addConstantCurves();
    /// connect variable \a var to port \a port. 
    void connectVar(const std::shared_ptr<VariableValue>& var, unsigned port);
    void disconnectAllVars();
    using ecolab::Plot::draw;
    void draw(cairo_t* cairo) const override;
    void requestRedraw(); ///< redraw plot using current data to all open windows
    void redrawWithBounds() override {redraw(0,0,500,500);}    
    
    /// add this as a display plot to its group
    void makeDisplayPlot();
          
    void resize(const LassoBox&) override;
    ClickType::Type clickType(float x, float y) const override;
    bool contains(float x, float y) const override
    {return clickType(x,y)!=ClickType::outside;}

    /// set autoscaling
    void autoScale() {xminVar=xmaxVar=yminVar=ymaxVar=y1minVar=y1maxVar=nullptr;}
    /// sets the plot scale and pen labels
    void scalePlot();

    /// @{ handle mouse events
    void mouseDown(float,float) override;
    void mouseMove(float,float) override;
    void mouseUp(float x,float y) override {
      mouseMove(x,y);
      ct=ClickType::outside;
    }
    bool onMouseOver(float,float) override;
    void onMouseLeave() override {valueString="";}
    /// @}

    /// export the plotted data as a CSV file
    // implemented as a single argument function here for exposure to TCL
    void exportAsCSV(const string& filename) {ecolab::Plot::exportAsCSV(filename);}

    void destroyFrame() override {RenderNativeWindow::destroyFrame();}

    /// return list of variables that could be attached to markers
    std::set<std::string> availableMarkers() const;
    
  };

}

#ifdef CLASSDESC
#pragma omit pack PlotItem
#pragma omit unpack PlotItem
#endif

inline void xml_pack(classdesc::xml_pack_t&,const ecolab::string&,ecolab::Plot&) {}
inline void xml_unpack(classdesc::xml_unpack_t&,const ecolab::string&,ecolab::Plot&) {}

#include "plotWidget.cd"
#include "plotWidget.xcd"
#endif
