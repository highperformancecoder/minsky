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

#ifndef PLOTOPTIONS_H
#define PLOTOPTIONS_H
#include "optional.h"
#include "plotWidget.h"

namespace minsky
{
  struct LegendGeometry
  {
    double legendLeft=0.9, legendTop=0.95, legendFontSz=0.03;
    LegendGeometry()=default;
    LegendGeometry(const ecolab::Plot& plot):
      legendLeft(plot.legendLeft), legendTop(plot.legendTop), legendFontSz(plot.legendFontSz) {}
    void setLegendGeometry(ecolab::Plot& plot) const {
      plot.legendLeft=legendLeft;
      plot.legendTop=legendTop;
      plot.legendFontSz=legendFontSz;
    }
  };

  struct DummyBase {};

  // template used to stack inheritance, as a fake multiple
  // inheritance feature. XML doesn't support multiple inheritance
  template <class Base=DummyBase>
  struct PlotOptions: public Base
  {
    PlotOptions()=default;
    PlotOptions(const Base& x): Base(x) {}
    Optional<std::string> name; //name, description or title
    Optional<bool> logx, logy, ypercent, plotTabDisplay, grid, subgrid;
    Optional<minsky::PlotWidget::PlotType> plotType;
    Optional<std::string> xlabel, ylabel, y1label;
    Optional<int> nxTicks, nyTicks;
    Optional<double> xtickAngle, exp_threshold;
    Optional<ecolab::Plot::Side> legend;
    Optional<LegendGeometry> legendGeometry;
    PlotOptions& operator=(const PlotWidget& plot) {
      name=plot.title;
      logx=plot.logx;
      logy=plot.logy;
      ypercent=plot.percent;
      plotTabDisplay=plot.plotTabDisplay;
      grid=plot.grid;
      subgrid=plot.subgrid;
      plotType=plot.plotType;
      xlabel=plot.xlabel();
      ylabel=plot.ylabel();
      y1label=plot.y1label();
      nxTicks=plot.nxTicks;
      nyTicks=plot.nyTicks;
      xtickAngle=plot.xtickAngle;
      exp_threshold=plot.exp_threshold;
      if (plot.legend) legend=plot.legendSide;
      legendGeometry=LegendGeometry(plot);
      return *this;
    }
    void applyPlotOptions(PlotWidget& plot) const
    {
      if (name) plot.title=*name;
      if (logx) plot.logx=*logx;
      if (logy) plot.logy=*logy;
      if (ypercent) plot.percent=*ypercent;
      if (plotTabDisplay) plot.plotTabDisplay=*plotTabDisplay;
      if (grid) plot.grid=*grid;
      if (subgrid) plot.subgrid=*subgrid;
      if (plotType) plot.plotType=*plotType;
      if (xlabel) plot.xlabel(*xlabel);
      if (ylabel) plot.ylabel(*ylabel);
      if (y1label) plot.y1label(*y1label);
      if (nxTicks) plot.nxTicks=*nxTicks;
      if (nyTicks) plot.nyTicks=*nyTicks;
      if (xtickAngle) plot.xtickAngle=*xtickAngle;
      if (exp_threshold) plot.exp_threshold=*exp_threshold;
      plot.legend=legend.get();
      if (legend) plot.legendSide=*legend;
      if (legendGeometry)
        legendGeometry->setLegendGeometry(plot);
    }
  };
}

#include "plotOptions.cd"
#include "plotOptions.xcd"
#endif
