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

#ifndef _GODLEYTABLEDLG_H
#define _GODLEYTABLEDLG_H

#include "guiDefs.h"
#include <vector>
#include "GUI/godleyIcon.h"
#undef None
#include "abstractView.h"
#include "minskyDoc.h"
#include "godleyTableWidget.h"
#include <Wt/WDialog>

namespace minsky { namespace gui {

class CanvasView;

/**
 *	Godley table configuration dialog
 *
 */
class GodleyTableDlg :
    public Wt::WDialog,
    public AbstractView
{
private:

  /**
   *	Default size constants, TODO: temp, use actual cell sizes, if possible.
   */
  enum SizeConstants {
    defaultMinWidth = 100,
    defaultMinHeight = 100,
  };

private:
  MinskyDoc::ObjectId tableId;      ///< The table ID.
  GodleyTableWidget* tableWidget;   ///< table edit widget. Pointer owned by Wt
  CanvasView* parentView;           ///< Parent view.

public:
  ///	Constructor.
  GodleyTableDlg(MinskyDoc::ObjectId tableID, CanvasView* parentView);

  virtual ~GodleyTableDlg();

protected:
  /**
   *  Called when the document has changed and needs refresh
   *  @param The View that caused the modification, or  0 if not applicable.
   */
  virtual void docHasChanged(AbstractView* source, MinskyDoc::Hint hint, const AnyParams& data);
};

}}  // namespace minsky::gui

#endif
