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

#ifndef VARIABLECANVASITEM_H_
#define VARIABLECANVASITEM_H_

#include "abstractCanvasItem.h"
#include "wtGeometry.h"

namespace minsky { namespace gui {

/**
 * A canvas item representing a variable.
 * */
class VariableCanvasItem: public minsky::gui::AbstractCanvasItem
{
public:
  typedef AbstractCanvasItem _base_class;

private:
  VariablePtr variable;         ///<  The model object this widget represents.

public:
  /**
   * Constructor.
   */
  VariableCanvasItem(MinskyDoc::ObjectId id, VariablePtr var, CanvasView* parent);

  /**
   * Destructor
   */
  virtual ~VariableCanvasItem();

  /**
   * Accesses the underlying model variable.
   */
  inline VariablePtr getVar() const
  {
    return variable;
  }

  /**
   *  returns the position of the object in model coordinates.
   */
  virtual void getModelPos(double& x, double& y) const;

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

}}  // namespace minsky::gui

#endif /* VARIABLECANVASITEM_H_ */
