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

#include "plotDlg.h"
#include "canvasPainter.h"
#include <ecolab_epilogue.h>
#include <Wt/Ext/Button>
#include <Wt/WLayout>
#include <Wt/WBorderLayout>
#include <sstream>

namespace minsky { namespace gui {

LargePlotWidget::LargePlotWidget(int _plotId,
                                 MinskyDoc& _doc,
                                 Wt::WContainerWidget* _parent)
  : _base_class(_parent),
    plotId(_plotId),
    doc(_doc)
{
  std::stringstream str;
  str << "plot" << (unsigned long)(this);
  surfaceName = str.str();

  setMinimumSize(300, 300);
  setLayoutSizeAware(true);
  resize(300, 300);
}

LargePlotWidget::~LargePlotWidget()
{
}

void LargePlotWidget::reset()
{
  update();
}

void LargePlotWidget::layoutSizeChanged(int newWidth,int newHeight)
{
  update();
}

void LargePlotWidget::paintEvent(WPaintDevice *paintDevice)
{
  CanvasPainter painter(paintDevice);
  minsky::Plots::iterator it = doc.getModel().plots.find(plotId);
  if (it != doc.getModel().plots.end())
  {
    painter.draw(*this, *it);
  }
}

PlotDlg::PlotDlg(int _plotId, MinskyDoc& doc)
  : AbstractView(doc),
    plotId(_plotId)
{

  Wt::WBorderLayout* lt = new Wt::WBorderLayout;
  contents()->setLayout(lt);
  plotWidget = new LargePlotWidget(plotId, doc);
  lt->addWidget(plotWidget, Wt::WBorderLayout::Center);

  setWindowTitle(doc.getModel().plots[plotId].title);
  setClosable(true);
  setModal(false);
  setResizable(true);
  setLayoutSizeAware(true);
  show();
//  webWidget()->resized().connect(this, &PlotDlg::layoutSizeChanged);

//  Wt::Ext::Button* closeButton = new Wt::Ext::Button(Wt::WString::tr("dialog.close"));
//  addButton(closeButton);
//
//  closeButton->clicked().connect(
//      [this](Wt::NoClass, Wt::NoClass, Wt::NoClass, Wt::NoClass, Wt::NoClass, Wt::NoClass)
//      {
//        reject();
//      });
}

PlotDlg::~PlotDlg()
{ }

void PlotDlg::done(Wt::WDialog::DialogCode r)
{
  Wt::WDialog::done(r);
  delete this;
}

void PlotDlg::docHasChanged(AbstractView* source, MinskyDoc::Hint hint, const AnyParams& data)
{
  switch(hint)
  {
  case MinskyDoc::hintSimulationReset:
    plotWidget->reset();
    break;

  case MinskyDoc::hintSimulationStep:
    plotWidget->update();
    break;

  case MinskyDoc::hintRemovePlot:
    if (data[0].type() == typeid(int)
        && boost::any_cast<int>(data[0]) == plotId)
    {
      reject();
    }
    break;

  case MinskyDoc::hintDocumentClose:
    reject();
    break;
  default: break;
  }
}

void PlotDlg::layoutSizeChanged(int newWidth,int newHeight)
{
  plotWidget->resize(newWidth, newHeight);
}

}}  // namespace minsky::gui
