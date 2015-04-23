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

#ifndef GROUPCANVASITEM_H_
#define GROUPCANVASITEM_H_

#include "abstractCanvasItem.h"

namespace minsky { namespace gui {

class GroupCanvasItem: public AbstractCanvasItem
{
private:
  typedef AbstractCanvasItem _base_class;

private:
  GroupIcon& icon;      ///< Model icon this item renders.

public:
  /**
   * Constructor
   */
  GroupCanvasItem(MinskyDoc::ObjectId id, GroupIcon& icon, CanvasView* parent);

  /**
   * Destructor
   */
  virtual ~GroupCanvasItem();

  /**
   *  returns the position of the object in model coordinates.
   */
  virtual void getModelPos(double& x, double& y) const;

  /**
   *  Accesses model icon object.
   */
  /** @{ */
  inline GroupIcon& getIcon()
  {
    return icon;
  }

  inline const GroupIcon& getIcon() const
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

#endif /* GROUPCANVASITEM_H_ */
