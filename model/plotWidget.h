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
#include <TCL_obj_base.h>
#include "classdesc_access.h"
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
  public:
    using Item::x;
    using Item::y;
    using ecolab::CairoSurface::surface;

    /// variable port attached to (if any)
    std::vector<std::shared_ptr<VariableValue>> yvars;
    std::vector<std::shared_ptr<VariableValue>> xvars;

    /// variable ports specifying plot size
    VariableValue xminVar, xmaxVar, yminVar, ymaxVar, y1minVar, y1maxVar;
    /// number of ticks to show in canvas item
    unsigned displayNTicks{3};
    double displayFontSize{3};


    std::string title;

    /// automatic means choose line or bar depending on the x-vector type.
    enum PlotType {line, bar, automatic};
    PlotType plotType=automatic;
    
    PlotWidget();
    void addPorts();
    PlotWidget(const PlotWidget& x): PlotWidgetSuper(x) {addPorts();}
    PlotWidget(PlotWidget&& x): PlotWidgetSuper(x) {addPorts();}
    
    // pick the Item width method, not ecolab::Plot's
    float width() const {return Item::width();}
    float height() const {return Item::height();}
    
    const PlotWidget* plotWidgetCast() const override {return this;}
    PlotWidget* plotWidgetCast() override {return this;}          
    
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
    
    bool plotTabDisplay=true; // ensure plots persisted on plot tab, but can optionally be made hidden. for ticket 1298
    void togglePlotTabDisplay() {plotTabDisplay=!plotTabDisplay;}     
    bool plotOnTab() const {return plotTabDisplay;}     
    
    /// add this as a display plot to its group
    void makeDisplayPlot();
          
    void resize(const LassoBox&) override;
    ClickType::Type clickType(float x, float y) override;

    /// set autoscaling
    void autoScale() {xminVar=xmaxVar=yminVar=ymaxVar=y1minVar=y1maxVar=VariableValue();}
    /// sets the plot scale and pen labels
    void scalePlot();

    /// @{ handle mouse events
    void mouseDown(double,double);
    void mouseMove(double,double);
    /// @}

    /// export the plotted data as a CSV file
    // implemented as a single argument function here for exposure to TCL
    void exportAsCSV(const string& filename) {ecolab::Plot::exportAsCSV(filename);}

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
