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

#include "godleyCanvasItem.h"
#include "canvasPainter.h"
#include "godleyTableDlg.h"
#include "canvasView.h"
#include "linkCanvasItem.h"
#include <ecolab_epilogue.h>
#include <Wt/WRectArea>

namespace minsky { namespace gui {

GodleyCanvasItem::GodleyCanvasItem(MinskyDoc::ObjectId id, minsky::GodleyIcon& _icon, CanvasView* parent)
  : AbstractCanvasItem(id, parent),
    icon(_icon)
{
  setPositionScheme(Absolute);
  adjustWidgetPos();
  resize(10, 10);                 //  must give a (dummy) size for Wt.
  initInteractive();

  std::stringstream str;
  str << "gt" << id;
  setId(str.str());
}

GodleyCanvasItem::~GodleyCanvasItem()
{
  if (editDialog)
  {
    editDialog->reject();
  }
}

void GodleyCanvasItem::getModelPos(double& x, double& y) const
{
  x = icon.x();
  y = icon.y();
}

bool GodleyCanvasItem::editProperties()
{
  if (!editDialog)
  {
    editDialog = new GodleyTableDlg(getId(), parent());
    editDialog->finished().connect(
        [this](WDialog::DialogCode r, NoClass&, NoClass&, NoClass&, NoClass&, NoClass&) -> void
        {
          delete editDialog;
          editDialog = NULL;
        });
  }
  return true;
}

void GodleyCanvasItem::getPortsList(PortsList& ports) const
{
  auto portsArray = icon.ports();
  ports.clear();

  std::for_each(portsArray.begin(), portsArray.end(),
      [&ports](int port)
      {
        ports.push_back(port);
      });
}

void GodleyCanvasItem::draw(CanvasPainter& painter, Polygon& outline)
{
  painter.draw(*this, outline);
}

}} // namespace minsky::gui
