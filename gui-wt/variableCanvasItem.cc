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

#include "variableCanvasItem.h"
#include "variablePropertiesDlg.h"
#include "displaySettings.h"
#include "canvasPainter.h"
#include "canvasView.h"
#include "linkCanvasItem.h"
#include "minskyDoc.h"
#include <ecolab_epilogue.h>
#include <Wt/WPolygonArea>

namespace minsky { namespace gui {

VariableCanvasItem::VariableCanvasItem(MinskyDoc::ObjectId id, VariablePtr var, CanvasView* parent)
    : _base_class(id, parent),
      variable(var)
{
  setPositionScheme(Absolute);
  adjustWidgetPos();
  resize(10, 10);                 //  must give a (dummy) size for Wt.
  initInteractive();

  std::stringstream str;
  str << "var" << id;
  setId(str.str());
}

VariableCanvasItem::~VariableCanvasItem()
{ }

void VariableCanvasItem::getModelPos(double& x, double& y) const
{
  x = variable->x();
  y = variable->y();
}

bool VariableCanvasItem::editProperties()
{
  bool result = false;

  std::shared_ptr<VariablePropertiesDlg> dlgPtr(new VariablePropertiesDlg(variable));

  if (Ext::Dialog::Accepted == dlgPtr->exec())
  {
    result = true;

    AnyParams params;
    params << getId();
    MinskyApp::getApp()->docHasChanged(parent()->getDoc(), parent(),
                                        MinskyDoc::hintEditVariable, params);
    dlgPtr->save();
    //  Renaming variables can erase them and their connected wires,
    //  so rescan model.
    parent()->auditModel();
    update();

    //  refresh attached wires.
    CanvasView::WiresList wires;
    parent()->getAttachedWires(wires, *this);
    for_each(wires.begin(), wires.end(),
        [](LinkCanvasItem* link)
        {
          link->update();
        });
  }
  return result;
}

void VariableCanvasItem::getPortsList(PortsList& ports) const
{
  auto& portsArray = variable->ports();
  ports.clear();

  std::for_each(portsArray.begin(), portsArray.end(),
      [&ports](int port)
      {
        ports.push_back(port);
      });
}

void VariableCanvasItem::draw(CanvasPainter& painter, Polygon& outline)
{
  painter.draw(*this, outline);
}

}}  // namespace minsky::gui
