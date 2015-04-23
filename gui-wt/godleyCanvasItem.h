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

#ifndef GODLEYCANVASITEM_H_
#define GODLEYCANVASITEM_H_

#include "abstractCanvasItem.h"
#include "GUI/godleyIcon.h"
#include "godleyTableDlg.h"

namespace minsky { namespace gui {

class GodleyCanvasItem: public minsky::gui::AbstractCanvasItem
{
private:
  minsky::GodleyIcon& icon;           ///< Associated model item
  GodleyTableDlg* editDialog = NULL;  ///< Weak pointer to edit dialog if any, the pointer is managed by Wt.

public:
  /**
   * Constructor
   * @param id The TCL object ID.
   * @param icon  The associated icon in the model.
   * @param parent The view to draw on.
   */
  GodleyCanvasItem(MinskyDoc::ObjectId id, minsky::GodleyIcon& icon, CanvasView* parent);

  /**
   * Destructor.
   */
  virtual ~GodleyCanvasItem();

  /**
   *  returns the position of the object in model coordinates.
   */
  virtual void getModelPos(double& x, double& y) const;

  /**
   *  Accesses model icon object.
   */
  /** @{ */
  inline minsky::GodleyIcon& getIcon()
  {
    return icon;
  }

  inline const minsky::GodleyIcon& getIcon() const
  {
    return icon;
  }
  /** @} */

  /**
   *  Opens the properties dialog so the user can edit object.
   *  @returns true if object properties were modified.
   */
  virtual bool editProperties();

  /**
   * Returns a list of ports that provide connection to this item
   */
  virtual void getPortsList(PortsList& ports) const;

protected:
  /**
   * Draw the widget using the painter.
   * @param painter Painter to use for drawing.
   * @param outline On return, contains the shape outline of the widget in model space.
   */
  virtual void draw(CanvasPainter& painter, Polygon& outline);
};

}} // namespace minsky::gui

#endif /* GODLEYCANVASITEM_H_ */
