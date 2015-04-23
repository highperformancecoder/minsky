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

#include "groupCanvasItem.h"
#include "canvasPainter.h"
#include <ecolab_epilogue.h>
#include <Wt/WRectArea>

namespace minsky { namespace gui {

GroupCanvasItem::GroupCanvasItem(MinskyDoc::ObjectId id, GroupIcon& _icon, CanvasView* parent)
    : _base_class(id, parent),
      icon(_icon)
{
  setPositionScheme(Absolute);
  adjustWidgetPos();
  resize(10, 10);                 //  must give a (dummy) size for Wt.
  initInteractive();

  std::stringstream str;
  str << "group" << id;
  setId(str.str());
}

GroupCanvasItem::~GroupCanvasItem()
{ }

void GroupCanvasItem::getModelPos(double& x, double& y) const
{
  x = icon.x();
  y = icon.y();
}

bool GroupCanvasItem::editProperties()
{
//  GodleyTableDlg dlg(icon, *parent());
//  bool retry = true;
//
//  while (retry)
//  {
//    retry = false;
//    if (Ext::Dialog::Accepted == dlg.exec())
//    {
//      try
//      {
//        icon.update();
//      }
//      catch(...)
//      {
//        //TODO. message for user.
//        retry = true;
//      }
//      update();
//    }
//  }
  return true;
}

void GroupCanvasItem::getPortsList(PortsList& ports) const
{
  ports = icon.ports();
}

void GroupCanvasItem::draw(CanvasPainter& painter, Polygon& outline)
{
  painter.draw(*this, outline);
}

}} // namespace minsky::gui
