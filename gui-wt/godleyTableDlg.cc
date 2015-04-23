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

#include "godleyTableDlg.h"
#include "canvasView.h"
#include "godleyCanvasItem.h"
#include "minskyApp.h"
#include <ecolab_epilogue.h>
#include <Wt/WTableView>
#include <Wt/Ext/Button>
#include <Wt/WAbstractItemDelegate>
#include <Wt/WComboBox>
#include <Wt/WScrollArea>
#include <Wt/WLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WCheckBox>

namespace minsky { namespace gui {

using namespace Wt;

GodleyTableDlg::GodleyTableDlg(MinskyDoc::ObjectId _tableId, CanvasView* _parentView)
  : AbstractView(_parentView->getDoc()),
    tableId(_tableId),
    parentView(_parentView)
{
  GodleyIcon& godleyIcon = MinskyApp::getDoc()->getGodleyTable(tableId);

  setWindowTitle(WString::fromUTF8(godleyIcon.table.title));
  setMinimumSize(440, 110);

  Wt::WRectF clientRect = parentView->modelToScreen(parentView->getViewport());
  contents()->setMaximumSize(.9 * clientRect.width(), (.9 * clientRect.height()) - 65);
  contents()->setOverflow(WContainerWidget::OverflowAuto, Vertical | Horizontal);
  contents()->setSelectable(false);

  tableWidget = new GodleyTableWidget(tableId, *MinskyApp::getDoc(), contents());

  //  footer contains a checkbox a spacer and a button

  WHBoxLayout* layout = new WHBoxLayout;
  footer()->setLayout(layout, Wt::AlignJustify);

  std::unique_ptr<WCheckBox> checkBox(new WCheckBox(WString::tr("dialog.double_entry")));
  checkBox->setChecked(getDoc().getGodleyTable(tableId).table.doubleEntryCompliant);
//  checkBox->setReadOnly(getDoc().simulationRunning());
  checkBox->setDisabled(getDoc().simulationRunning());

  checkBox->checked().connect(
      [this](NoClass)
      {
        getDoc().getGodleyTable(tableId).table.doubleEntryCompliant = true;
        MinskyApp::getApp()->docHasChanged(getDoc(), this,
                                           MinskyDoc::hintEditGodleyTable, AnyParams(tableId));
      });

  checkBox->unChecked().connect(
      [this](NoClass)
      {
        getDoc().getGodleyTable(tableId).table.doubleEntryCompliant = false;
        MinskyApp::getApp()->docHasChanged(getDoc(), this,
                                           MinskyDoc::hintEditGodleyTable, AnyParams(tableId));
      });

  layout->addWidget(checkBox.release(), 0, Wt::AlignLeft);
  layout->addStretch();

  std::unique_ptr<WPushButton> button(new WPushButton(WString::tr("dialog.close")));
  button->setWidth(160);
  button->setMaximumSize(160, WLength::Auto);
  button->clicked().connect(this, &WDialog::reject);

  layout->addWidget(button.release(), 0, Wt::AlignRight);

  setClosable(true);
  setResizable(true);
  setModal(false);
  show();
}

GodleyTableDlg::~GodleyTableDlg()
{ }

void GodleyTableDlg::docHasChanged(AbstractView* source, MinskyDoc::Hint hint, const AnyParams& data)
{
  if (source == this)
  {
    return;
  }

  switch(hint)
  {
  case MinskyDoc::hintSimulationStep:
    {
      WCheckBox* checkBox = dynamic_cast<WCheckBox*>(footer()->widget(0));
      if (checkBox)
      {
        //readonly does not work with chrome
//        checkBox->setReadOnly(true);
        checkBox->setDisabled(true);
      }
    }
    break;

  case MinskyDoc::hintSimulationReset:
    {
      WCheckBox* checkBox = dynamic_cast<WCheckBox*>(footer()->widget(0));
      if (checkBox)
      {
//        checkBox->setReadOnly(false);
        checkBox->setDisabled(false);
      }
    }
    break;

  case MinskyDoc::hintEditGodleyTable:
    if (data[0].type() == typeid(MinskyDoc::ObjectId)
        && boost::any_cast<MinskyDoc::ObjectId>(data[0]) == tableId)
    {
      WCheckBox* checkBox = dynamic_cast<WCheckBox*>(footer()->widget(0));
      if (checkBox)
      {
        checkBox->setChecked(getDoc().getGodleyTable(tableId).table.doubleEntryCompliant);
      }
    }
    break;
  default: break;
 }
}

}}  // namespace minsky::gui
