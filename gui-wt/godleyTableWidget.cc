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

#include "godleyTableWidget.h"
#include "minskyApp.h"
#include <ecolab_epilogue.h>
#include <Wt/WTableRow>
#include <Wt/WComboBox>
#include <Wt/Ext/Button>
#include <Wt/WHBoxLayout>
#include <Wt/WLineEdit>
#include <Wt/WString>
#include <Wt/WText>
#include <Wt/Ext/Menu>
#include <Wt/Ext/ToolBar>
#include <algorithm>

namespace minsky { namespace gui {

using namespace Wt;

GodleyTableWidget::GodleyTableWidget(MinskyDoc::ObjectId _tableId, MinskyDoc& doc, WContainerWidget* _parent)
  : _base_class(_parent),
    AbstractView(doc),
    tableId(_tableId),
    godley(doc.getGodleyTable(_tableId))
{
  loadTable();
  setSelectable(false);
}

GodleyTableWidget::~GodleyTableWidget()
{ }

void GodleyTableWidget::loadTable()
{
  for(int row = 0; row < widgetRow(godley.table.rows()); ++row)
  {
    for (int col = 0; col < widgetCol(godley.table.cols() + 1); ++col)
    {
      initCell(row, col);   // will create row/column as needed
    }
  }
}

void GodleyTableWidget::addRow(int insertAfter)
{
  int row = insertAfter + 1;

  godley.table.insertRow(tableRow(row));
  insertRow(row);
  for (int col = 0; col < columnCount(); ++col)
  {
    initCell(row, col);
  }
}

void GodleyTableWidget::addColumn(int insertAfter)
{
  int col = insertAfter + 1;
  godley.table.insertCol(tableCol(col));
  insertColumn(col);
  for (int row = 0; row < rowCount(); ++row)
  {
    initCell(row, col);
  }
  for (size_t row = 1; row < godley.table.rows(); ++row)
  {
    updateRowSum(widgetRow(row));
  }
}

void GodleyTableWidget::removeRow(int row)
{
  godley.deleteRow(tableRow(row) + 1);
  _base_class::deleteRow(row);
}

void GodleyTableWidget::removeColumn(int col)
{
  godley.table.deleteCol(tableCol(col) + 1);
  _base_class::deleteColumn(col);
  for (size_t row = 1; row < godley.table.rows(); ++row)
  {
    updateRowSum(widgetRow(row));
  }
}

void GodleyTableWidget::initCell(int row, int column)
{
  WTableCell* cell = elementAt(row, column);
  cell->setContentAlignment(AlignCenter | AlignMiddle);
  cell->setPadding(0, Left | Top | Right | Bottom);
  cell->setSelectable(false);
  if (column == 1)
  {
    cell->setWidth(200);
    cell->setMinimumSize(100, WLength::Auto);
  }

  if (row == 0 && column == 0)
  {
    //  nothing
  }
  else if (row == 0 && tableCol(column) < int(godley.table.cols()))
  {
    createColumnHeader(cell, column);
  }
  else if (column == 0)
  {
    createAddRemoveRowButtons(cell, row);
  }
  else if (tableCol(column) < int(godley.table.cols()))
  {
    createLineEditControl(cell, row, column);
  }
  else if (tableCol(column) == int(godley.table.cols()))
  {
    createTextControl(cell, row);
  }
}

const Wt::WString GodleyTableWidget::getCellValue(int row, int column) const
{
  int rowTable = tableRow(row);
  int colTable = tableCol(column);

  if (rowTable < 0 || colTable < 0)
  {
    return "";
  }
  if (rowTable < int(godley.table.rows()) && colTable < int(godley.table.cols()))
  {
    return godley.table.cell(rowTable, colTable);
  }
  else if (colTable == int(godley.table.cols()))
  {
    return godley.table.rowSum(rowTable);
  }
  return "";
}

void GodleyTableWidget::simulationStep()
{
  refreshDisplay();
}

void GodleyTableWidget::simulationReset()
{
  refreshDisplay();
}

void GodleyTableWidget::createColumnHeader(WTableCell* cell, int column)
{
  Ext::ToolBar* toolbar = new Ext::ToolBar(cell);

  if (0 < tableCol(column) && tableCol(column) < int(godley.table.cols()))
  {
    // asset class combo box
    WComboBox* comboBox = new WComboBox;
    toolbar->add(comboBox);

    comboBox->setHiddenKeepsGeometry(true);
    if(!godley.table.doubleEntryCompliant)
    {
      comboBox->hide();
    }
    comboBox->addItem(WString::tr("dialog.godley.noassetclass"));
    comboBox->addItem(WString::tr("dialog.godley.asset"));
    comboBox->addItem(WString::tr("dialog.godley.liability"));
    comboBox->addItem(WString::tr("dialog.godley.equity"));

    comboBox->activated().connect(
        [this, comboBox, cell](int, NoClass, NoClass, NoClass, NoClass, NoClass) -> void
        {
          GodleyAssetClass::AssetClass assetClass;
          switch(comboBox->currentIndex())
          {
          default:
          case 0: assetClass = GodleyAssetClass::noAssetClass;  break;
          case 1: assetClass = GodleyAssetClass::asset;         break;
          case 2: assetClass = GodleyAssetClass::liability;     break;
          case 3: assetClass = GodleyAssetClass::equity;        break;
          }
          godley.table._assetClass(tableCol(cell->column()), assetClass);
          MinskyApp::getApp()->docHasChanged(getDoc(), this, MinskyDoc::hintEditGodleyTable, AnyParams(tableId));
          refreshDisplay();
        });

    switch(godley.table._assetClass(tableCol(column)))
    {
    default:
    case GodleyTable::noAssetClass:
      comboBox->setCurrentIndex(0);
      break;
    case GodleyTable::asset:
      comboBox->setCurrentIndex(1);
      break;
    case GodleyTable::liability:
      comboBox->setCurrentIndex(2);
      break;
    case GodleyTable::equity:
      comboBox->setCurrentIndex(3);
      break;
    }
  }

  Ext::Button* addButton = new Ext::Button();
  toolbar->add(addButton);

  addButton->setText("+");
  addButton->setInline(true);
  addButton->clicked().connect(
      [this, cell](NoClass, NoClass, NoClass, NoClass, NoClass, NoClass) -> void
      {
        addColumn(cell->column());
        MinskyApp::getApp()->docHasChanged(getDoc(), this, MinskyDoc::hintEditGodleyTable, AnyParams(tableId));
      });

  if (tableCol(column) > 0)
  {
    Ext::Button* removeButton = new Ext::Button();
    toolbar->add(removeButton);
    removeButton->setText("-");
    removeButton->setInline(true);
    removeButton->clicked().connect(
        [this, cell](NoClass, NoClass, NoClass, NoClass, NoClass, NoClass) -> void
        {
          removeColumn(cell->column());
          MinskyApp::getApp()->docHasChanged(getDoc(), this, MinskyDoc::hintEditGodleyTable, AnyParams(tableId));
        });
  }
}

void GodleyTableWidget::createAddRemoveRowButtons(WTableCell* cell, int row)
{
  //  add/remove row control
  Ext::ToolBar* toolbar = new Ext::ToolBar(cell);
  Ext::Button* addButton = new Ext::Button();
  toolbar->add(addButton);
  addButton->setText("+");
  addButton->setInline(true);
  addButton->clicked().connect(
      [this, cell](NoClass, NoClass, NoClass, NoClass, NoClass, NoClass) ->void
      {
        addRow(cell->row());
        MinskyApp::getApp()->docHasChanged(getDoc(), this, MinskyDoc::hintEditGodleyTable, AnyParams(tableId));
      });


  if (tableRow(row) > 0)
  {
    Ext::Button* removeButton = new Ext::Button();
    toolbar->add(removeButton);
    removeButton->setText("-");
    removeButton->setInline(true);
    removeButton->clicked().connect(
        [this, cell](NoClass, NoClass, NoClass, NoClass, NoClass, NoClass) ->void
        {
          removeRow(cell->row());
          MinskyApp::getApp()->docHasChanged(getDoc(), this, MinskyDoc::hintEditGodleyTable, AnyParams(tableId));
        });
  }
}

void GodleyTableWidget::createLineEditControl(Wt::WTableCell* cell, int row, int column)
{
  if ((tableRow(row) < int(godley.table.rows())) && (tableCol(column) < int(godley.table.cols())))
  {
    WLineEdit* edit = new WLineEdit(cell);
    edit->setVerticalAlignment(AlignMiddle);
    edit->setWidth(WLength(100, WLength::Percentage));
    edit->setHeight(WLength(100, WLength::Percentage));
    if (tableCol(column) == 0)
    {
//      edit->setMinimumSize(200, WLength::Auto);
    }
    else
    {
      edit->addStyleClass("align-center");
    }

    WString text;
    WColor color;
    getCellText(tableRow(row), tableCol(column), text, color);

    WCssDecorationStyle style;
    style.setForegroundColor(color);

    edit->setText(text);
    edit->setDecorationStyle(style);

    edit->changed().connect(
        [this, edit, cell](NoClass) -> void
        {
          godley.setCell(tableRow(cell->row()), tableCol(cell->column()), edit->text().toUTF8());
          //  no row sum update for row 0
          if (tableRow(cell->row()) > 0)
          {
            updateRowSum(cell->row());
            MinskyApp::getApp()->docHasChanged(getDoc(), this, MinskyDoc::hintEditGodleyTable, AnyParams(tableId));
          }
        });
  }
}

void GodleyTableWidget::createTextControl(Wt::WTableCell* cell, int row)
{
  if (tableRow(row) < 0)
  {
    cell->setMinimumSize(100, WLength::Auto);
  }
  else if (tableRow(row) == 0)
  {
    WText* text = new WText(cell);
    text->setVerticalAlignment(AlignMiddle);
    text->setText(WString::tr("dialog.godley.rowsum"));
  }
  else
  {
    WText* text = new WText(cell);
    text->setVerticalAlignment(AlignMiddle);
    text->setText(WString::fromUTF8(godley.table.rowSum(tableRow(row))));
  }
}

void GodleyTableWidget::updateRowSum(int row)
{
  WTableCell* cell = elementAt(row, columnCount() - 1);
  if (cell)
  {
    WText* text = dynamic_cast<WText*>(cell->widget(0));
    if (text)
    {
      text->setText(WString::fromUTF8(godley.table.rowSum(tableRow(row))));
    }
  }
}

void GodleyTableWidget::docHasChanged(AbstractView* source, MinskyDoc::Hint hint, const AnyParams& data)
{
  if (source == this)
  {
    return;
  }

  switch(hint)
  {
  case MinskyDoc::hintSimulationReset:
    simulationReset();
    break;

  case MinskyDoc::hintSimulationStep:
    simulationStep();
    break;

  case MinskyDoc::hintRemoveGodleyTable:
    if (data[0].type() == typeid(MinskyDoc::ObjectId)
        && boost::any_cast<MinskyDoc::ObjectId>(data[0]) == tableId)
    {
      delete parent();
    }
    break;

  case MinskyDoc::hintEditGodleyTable:
    if (data[0].type() == typeid(MinskyDoc::ObjectId)
        && boost::any_cast<MinskyDoc::ObjectId>(data[0]) == tableId)
    {
      refreshDisplay();
    }
    break;

  case MinskyDoc::hintDocumentClose:
    delete parent();
    break;
  default: break;
  }
}

void GodleyTableWidget::refreshDisplay()
{
  refreshColumnHeaders();

  for (size_t row = 0; row < godley.table.rows(); ++row)
  {
    // update data cells text values.
    for (size_t col = 0; col < godley.table.cols(); ++col)
    {
      WTableCell* cell = elementAt(widgetRow(row), widgetCol(col));
      if (cell)
      {
        WString wstr;
        WColor color;
        getCellText(row, col, wstr, color);
        WLineEdit* lineEdit = dynamic_cast<WLineEdit*>(cell->widget(0));
        if (lineEdit)
        {
          WCssDecorationStyle style;
          style.setForegroundColor(color);
          lineEdit->setText(wstr);
          lineEdit->setDecorationStyle(style);
        }
      }
    }
  }
}

void GodleyTableWidget::refreshColumnHeaders()
{
  for (size_t col = 0; col < godley.table.cols(); ++col)
  {
    WTableCell* cell = elementAt(0, widgetCol(col));
    if (cell)
    {
      Wt::Ext::ToolBar* toolbar = dynamic_cast<Wt::Ext::ToolBar*>(cell->widget(0));
      if (toolbar)
      {
        const std::vector<WWidget*>& widgets = toolbar->children();
        Wt::WComboBox* comboBox = dynamic_cast<Wt::WComboBox*>(widgets[0]);
        if (comboBox)
        {
          if (godley.table.doubleEntryCompliant)
          {
            comboBox->show();
          }
          else
          {
            comboBox->hide();
          }
        }
      }
    }
  }
}

void GodleyTableWidget::getCellText(int row, int col, WString& text, WColor& color)
{
  text = WString();
  color = WColor(0, 0, 0);

  if (0 <= row && row < int(godley.table.rows())
      && 0 <= col && col < int(godley.table.cols()))
  {
    if (col > 0)
    {
      if (godley.table.doubleEntryCompliant
        && godley.table._assetClass(col) == GodleyTable::noAssetClass)
      {
        text = WString::tr("dialog.godley.noassetclass");
      }
      else if (getDoc().simulationRunning())
      {
        WString wstr = getCellValue(widgetRow(row), widgetCol(col));
        wstr = wstr.trim();
        std::string varName = wstr.toUTF8();
        if (!varName.empty())
        {
          bool sign = (varName[0] == '-');
          if (sign)
          {
            varName = varName.substr(1);
          }
          if (getDoc().variableExists(varName))
          {
            std::stringstream sstr;
            sstr << wstr << "=" << (getDoc().getVariableValue(varName) * ((sign) ? -1 : 1));
            text = WString(sstr.str());
          }
          else
          {
            text = wstr;
          }
        }
      }
      else
      {
        WString wstr = getCellValue(widgetRow(row), widgetCol(col));
        text = wstr.trim();
      }
    }
    else
    {
      WString wstr = getCellValue(widgetRow(row), widgetCol(col));
      text = wstr.trim();
    }
  }

  if (!text.empty())
  {
    if (text.toUTF8()[0] == '-')
    {
      color = WColor(255, 0, 0);
    }
  }
}

}}  // namespace minsky::gui
