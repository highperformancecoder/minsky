/*
  @copyright Steve Keen 2012
  @author Michael Roy
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

#include "plotCanvasItem.h"
#include "minskyApp.h"
#include "canvasPainter.h"
#include "plotDlg.h"
#include "canvasView.h"
#include <Wt/WRectArea>
#include <ecolab_epilogue.h>

namespace minsky { namespace gui {

PlotCanvasItem::PlotCanvasItem(int _name,
                               ::minsky::PlotWidget& _plot,
                                CanvasView* parent)
  : AbstractCanvasItem(MinskyDoc::invalidId, parent),
    plot(_plot),
    name(_name)
{
  setId(str(getName()));
  setPositionScheme(Absolute);
  adjustWidgetPos();
  resize(10, 10);                 //  must give a (dummy) size for Wt.
  update();
}


PlotCanvasItem::~PlotCanvasItem()
{ }

void PlotCanvasItem::getModelPos(double& x, double& y) const
{
  x = plot.x();
  y = plot.y();
}

bool PlotCanvasItem::editProperties()
{
  new PlotDlg(name, parent()->getDoc());
  return false;
}

void PlotCanvasItem::getPortsList(PortsList& ports) const
{
  ports.clear();

  std::for_each(plot.ports().begin(), plot.ports().end(),
      [&ports](int port)
      {
        ports.push_back(port);
      });
}

void PlotCanvasItem::reset()
{
  update();
}

void PlotCanvasItem::draw(CanvasPainter& painter, Polygon& outline)
{
  painter.draw(*this, outline);
}

}}  // namespace minsky::gui
