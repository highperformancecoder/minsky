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

#include "operationCanvasItem.h"
#include "displaySettings.h"
#include "canvasPainter.h"
#include "canvasView.h"
#include "constantPropertiesDlg.h"
#include "integralPropertiesDlg.h"
#include "linkCanvasItem.h"
#include "sliderWidget.h"
#include <ecolab_epilogue.h>

namespace minsky { namespace gui {

using namespace Wt;

OperationCanvasItem::OperationCanvasItem(MinskyDoc::ObjectId id, OperationPtr op, CanvasView* _parent)
  : AbstractCanvasItem(id, _parent),
    operation(op)
{
  setPositionScheme(Absolute);
  resize(1, 1);
  adjustWidgetPos();

  std::stringstream str;
  str << "op" << id;
  setId(str.str());

  Constant* constOp = dynamic_cast<Constant*>(getOp().get());
  if (constOp)
  {
    if (constOp->sliderVisible)
    {
      sliderWidget = new SliderWidget(*this, parent());
      //  TODO: where is value ?
      sliderWidget->setRangeAndStepSize(constOp->sliderStep, constOp->sliderMin, constOp->sliderMax);
      setSliderWidgetPos();
      sliderWidget->update();
    }
  }
}

OperationCanvasItem::~OperationCanvasItem()
{
  //  destroy the attached widget.
  if (sliderWidget)
  {
    delete sliderWidget;
  }
}

void OperationCanvasItem::moveTo(const double& x, const double& y)
{
  AbstractCanvasItem::moveTo(x, y);
  setSliderWidgetPos();
}

void OperationCanvasItem::getModelPos(double& x, double& y) const
{
  x = operation->x();
  y = operation->y();
}

bool OperationCanvasItem::editProperties()
{
  bool result = false;

  typedef shared_ptr<OperationPropertiesDlg> DlgPtr;

  DlgPtr dlgPtr;

  if (operation->type() == OperationType::constant)
  {
    dlgPtr = DlgPtr(new ConstantPropertiesDlg(getOp()));
  }
  else if (operation->type() == OperationType::integrate)
  {
    dlgPtr = DlgPtr(new IntegralPropertiesDlg(getOp()));
  }
  else
  {
    dlgPtr = DlgPtr(new OperationPropertiesDlg(getOp()));
  }

  if (Ext::Dialog::Accepted == dlgPtr->exec())
  {
    result = true;

    AnyParams params;
    params << getId();
    MinskyApp::getApp()->docHasChanged(parent()->getDoc(), parent(),
                                        MinskyDoc::hintEditOperation, params);
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

void OperationCanvasItem::getPortsList(PortsList& ports) const
{
  ports = operation->ports();
}

/**
 * Called by attached slider to indicate user moved slider thumb.
 */
void OperationCanvasItem::sliderSetValue(const double& value)
{
  if (operation->type() == OperationType::constant)
  {
    parent()->getDoc().setOperationSliderValue(parent(), getId(), value);
  }
}

void OperationCanvasItem::setSliderWidgetPos()
{
  if (sliderWidget)
  {
    double xCenter = offset(Left).value() + (width().value() / 2);
    double yBottom = offset(Top).value();

    sliderWidget->setOffsets(xCenter - (SliderWidget::width / 2.), Left);
    sliderWidget->setOffsets(yBottom - SliderWidget::height, Top);
  }
}

void OperationCanvasItem::draw(CanvasPainter& painter, Polygon& outline)
{
  painter.draw(*this, outline);
}

void OperationCanvasItem::paintEvent (WPaintDevice *paintDevice)
{
  _base_class::paintEvent(paintDevice);

  //  draw slider, if applicable

  Constant* constOp = dynamic_cast<Constant*>(getOp().get());
  if (constOp)
  {
    if (constOp->sliderVisible)
    {
      if (!sliderWidget)
      {
        sliderWidget = new SliderWidget(*this, parent());
      }
      //  TODO: where is value ?
      sliderWidget->setRangeAndStepSize(constOp->sliderStep, constOp->sliderMin, constOp->sliderMax);
      setSliderWidgetPos();
      sliderWidget->update();
    }
    else
    {
      if (sliderWidget)
      {
        delete sliderWidget;
        sliderWidget = NULL;
      }
    }
  }
}

}}  // namespace minsky::gui
