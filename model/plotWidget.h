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
#include <TCL_obj_base.h>
#include "classdesc_access.h"
#include "plot.h"
#include "variable.h"
#include "zoom.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

namespace minsky
{
  using namespace ecolab;
  // a container item for a plot widget
  class PlotWidget: public Item, public ecolab::Plot
  {
    CLASSDESC_ACCESS(PlotWidget);
    friend class SchemaHelper;
    // timestamp of last time this widget was blitted and also the
    // accumulated blit time at last add.
    classdesc::Exclude<boost::posix_time::ptime> 
      lastAdd{boost::posix_time::microsec_clock::local_time()},
      lastAccumulatedBlitTime{boost::posix_time::microsec_clock::local_time()};
    // overrides placement of ports etc when just data has changed
    bool justDataChanged=false;
    classdesc::Exclude<Tk_Canvas> canvas; // canvas this widget will be displayed on
    friend struct PlotItem;
  public:
    using Item::x;
    using Item::y;

    /// variable port attached to (if any)
    std::vector<VariableValue> yvars;
    std::vector<VariableValue> xvars;

    /// variable ports specifying plot size
    VariableValue xminVar, xmaxVar, yminVar, ymaxVar, y1minVar, y1maxVar;
    /// number of ticks to show in canvas item
    unsigned displayNTicks{3};
    double displayFontSize{3};


    //std::vector<std::string> images;

    std::string title;
 
    int width{150}, height{150};

    PlotWidget();

    void addPlotPt(double t); ///< add another plot point
    void updateIcon(double t) override {addPlotPt(t);}
    /// connect variable \a var to port \a port. 
    void connectVar(const VariableValue& var, unsigned port);
    // draw canvas widget
    void draw(ecolab::cairo::Surface&);
    void draw(cairo_t* cairo) const override;
    /// surfaces to draw into for redraw. expandedPlot refers to
    /// separate popup plot window
    Exclude<cairo::SurfacePtr> cairoSurface, expandedPlot; 
    void redraw(); // redraw plot using current data to all open windows

    /// set autoscaling
    void autoScale() {xminVar=xmaxVar=yminVar=ymaxVar=y1minVar=y1maxVar=VariableValue();}
    /// sets the plot scale and pen labels
    void scalePlot();

    /// add an additional image surface to a PlotWidget
    /// @param image - image name for new surface to be added
    /// TODO - move to minskyTCL?
    void addImage(const std::string& args);
  };

}

#ifdef CLASSDESC
#pragma omit pack PlotItem
#pragma omit unpack PlotItem
#endif

inline void xml_pack(classdesc::xml_pack_t&,const ecolab::string&,ecolab::Plot&) {}
inline void xml_unpack(classdesc::xml_unpack_t&,const ecolab::string&,ecolab::Plot&) {}

#include "plotWidget.cd"
#endif
