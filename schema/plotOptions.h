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
    Optional<bool> logx, logy, ypercent, grid, subgrid;
    Optional<minsky::PlotWidget::PlotType> plotType;
    Optional<std::string> xlabel, ylabel, y1label;
    Optional<int> nxTicks, nyTicks;
    Optional<double> xtickAngle, exp_threshold;
    double xmin=nan(""), xmax=nan(""), ymin=nan(""), ymax=nan(""), y1min=nan(""), y1max=nan("");
    unsigned symbolEvery=1;
    Optional<ecolab::Plot::Side> legend;
    Optional<LegendGeometry> legendGeometry;
    Optional<std::vector<ecolab::Plot::LineStyle>> palette;
    Optional<std::vector<std::string>> horizontalMarkers, verticalMarkers;
    PlotOptions& operator=(const PlotWidget& plot) {
      name=plot.title;
      logx=plot.logx;
      logy=plot.logy;
      ypercent=plot.percent;
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
      xmin=plot.xmin;
      xmax=plot.xmax;
      ymin=plot.ymin;
      ymax=plot.ymax;
      y1min=plot.y1min;
      y1max=plot.y1max;
      symbolEvery=plot.symbolEvery;
      if (plot.legend) legend=plot.legendSide;
      legendGeometry=LegendGeometry(plot);
      palette=plot.palette;
      horizontalMarkers=plot.horizontalMarkers;
      verticalMarkers=plot.verticalMarkers;
      return *this;
    }
    void applyPlotOptions(PlotWidget& plot) const
    {
      if (name) plot.title=*name;
      if (logx) plot.logx=*logx;
      if (logy) plot.logy=*logy;
      if (ypercent) plot.percent=*ypercent;
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
      plot.xmin=xmin;
      plot.xmax=xmax;
      plot.ymin=ymin;
      plot.ymax=ymax;
      plot.y1min=y1min;
      plot.y1max=y1max;
      plot.symbolEvery=symbolEvery;
      plot.legend=legend.get();
      if (legend) plot.legendSide=*legend;
      if (legendGeometry)
        legendGeometry->setLegendGeometry(plot);
      if (palette) plot.palette=*palette;
      if (horizontalMarkers) plot.horizontalMarkers=*horizontalMarkers;
      if (verticalMarkers) plot.verticalMarkers=*verticalMarkers;
    }
  };
}

#include "plotOptions.cd"
#include "plotOptions.xcd"
#endif
