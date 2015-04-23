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

#ifndef GODLEYTABLEWIDGET_H_
#define GODLEYTABLEWIDGET_H_

#include "abstractView.h"
#include <Wt/WTable>
#include "GUI/godleyIcon.h"
#undef None
#include <string>
#include <vector>

namespace minsky { namespace gui {
/**
 * A representation of a godley table.  Permits viewing and editing
 * of a godley table data.
 */
class GodleyTableWidget:
    public Wt::WTable,
    public AbstractView

{
private:
  typedef Wt::WTable _base_class;

  enum
  {
    numRowHeaders = 1,
    numColumnHeaders = 1,
  };

private:
  MinskyDoc::ObjectId tableId;  ///< Id of the godley table being viewed/edited
  GodleyIcon& godley;           ///< The godley table being viewed/edited.

public:
  /**
   * Constructor
   * @param tableId Model tale ID.
   * @param doc     Reference to the model
   * @param parent  Parent container widget.
   */
  GodleyTableWidget(MinskyDoc::ObjectId tableId, MinskyDoc& doc, Wt::WContainerWidget* parent);

  /**
   * Destructor.
   */
  virtual ~GodleyTableWidget();

protected:
  /**
   * Loads the table.
   */
  void loadTable();

  /**
   * Add a new row in the table.
   * @param insertAfter The new row will be inserted after this row.
   */
  void addRow(int insertAfter);

  /**
   * Adds a new column to the table.
   * @param insertAfter The new column will be inserted after this row.
   */
  void addColumn(int insertAfter);

  /**
   * Removes a row from the table.
   * @param row Index of the row to delete.
   */
  void removeRow(int row);

  /**
   * Removes a column from the table.
   * @param column Index of the column to delete.
   */
  void removeColumn(int column);

  /**
   * Initializes a cell during buildup widget build up.
   * @param row     Cell row.
   * @param column  Cell column.
   */
  void initCell(int row, int column);

  /**
   * Returns the cell value from model table data.
   * @param row     Cell row.
   * @param column  Cell column.
   * @return A string containing the cell value.
   */
  const Wt::WString getCellValue(int row, int column) const;

  /**
   * Converts a widget row index to a model row index.
   * @param rowWidget Widget row index
   * @return  The corresponding model table row index.
   */
  inline int tableRow(int rowWidget) const
  {
    return (rowWidget - numColumnHeaders);
  }

  /**
   * Converts a widget column index to a model column index.
   * @param colWidget Widget column index
   * @return  The corresponding model table column index.
   */
  inline int tableCol(int colWidget) const
  {
    return (colWidget - numRowHeaders);
  }

  inline int widgetRow(int rowTable) const
  {
    return (rowTable + numColumnHeaders);
  }

  inline int widgetCol(int colTable) const
  {
    return (colTable + numRowHeaders);
  }

  /**
   * Creates and populates the column header cells,
   * @param cell  The column header cell.
   * @param column  Widget index of the column
   */
  void createColumnHeader(Wt::WTableCell* cell, int column);

  /**
   * Creates the small +/- buttons letting the user add and remove rows.
   * @param cell  The row header cell.
   * @param row   Widget index of the row
   */
  void createAddRemoveRowButtons(Wt::WTableCell* cell, int row);

  /**
   * Creates the line edit control that allow editing and display of a cell value.
   * @param cell    Table cell where insertion will take place.
   * @param row     Widget row of the cell.
   * @param column  Widget column of the cell.
   */
  void createLineEditControl(Wt::WTableCell* cell, int row, int column);

  /**
   * Create a simplke text widget to print read-only information in a cell.
   * @param cell    Table cell where insertion will take place.
   * @param row     Widget row of the cell.
   */
  void createTextControl(Wt::WTableCell* cell, int row);
  void updateRowSum(int row);
  void simulationStep();
  void simulationReset();

  /**
   *  Called when the document has changed and needs refresh
   *  @param The View that caused the modification, or  0 if not applicable.
   */
  void docHasChanged(AbstractView* source, MinskyDoc::Hint hint, const AnyParams& data);

  /**
   * Refreshes displayed data.
   * @param showValues If true, variables values will be shown.
   */
  void refreshDisplay();

  /**
   * Redraws the column headers.
   */
  void refreshColumnHeaders();

  void getCellText(int row, int col, WString& text, WColor& color);
};

}}  // namespace minsky::gui

#endif /* GODLEYTABLEWIDGET_H_ */
