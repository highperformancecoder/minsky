/*
  @copyright Steve Keen 2013
  @author Russell Standish
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
#include "note.h"
#include "clickType.h"

#ifndef OPVARBASEATTRIBUTES_H
#define OPVARBASEATTRIBUTES_H
namespace minsky 
{
  /// attributes common between variables and operations
  struct OpVarBaseAttributes: public Note
  {
    float m_x, m_y; ///< position in canvas, or within group
    float zoomFactor;
    double rotation; ///< rotation of icon, in degrees
    int group;
  
    bool mouseFocus; ///<true if target of a mouseover
    bool selected; ///<true if selected for cut, copy or group operation

    /**
       whether variable is visible on Canvas (note godley variables are
       never visible, as they appear as part of the godley icon 
    */
    bool visible;
    OpVarBaseAttributes(): m_x(0), m_y(0), zoomFactor(1), rotation(0), 
                           group(-1),
                           mouseFocus(false), selected(false), visible(true)
    {}

    virtual ~OpVarBaseAttributes() {}

    /// x -coordinate of items position on canvas
    float x() const;
    /// y -coordinate of items position on canvas
    float y() const;

    /// move to an absolute location
    void moveTo(float x1, float y1) {
      float dx=x1-x(), dy=y1-y();
      move(dx,dy);
    }
    // TODO: can be better handled by generic code within this class,
    // but need to wait for big item refactor
    /// move relative
    virtual void move(float x1, float y1);

    /// zoom by \a factor, scaling all widget's coordinates, using (\a
    /// xOrigin, \a yOrigin) as the origin of the zoom transformation
    void zoom(float xOrigin, float yOrigin,float factor);
    void setZoom(float factor) {zoomFactor=factor;}

    /// set zoom factor on attached variable (if any) - eg IntOp
    virtual void setZoomOnAttachedVariable(float) {}
    virtual ClickType::Type clickType(float x, float y) const {
      return ClickType::onItem;
    };
  };
}

#include "opVarBaseAttributes.cd"
#endif
