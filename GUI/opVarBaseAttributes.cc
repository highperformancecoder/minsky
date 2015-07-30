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

#include "opVarBaseAttributes.h"
#include "minsky.h"
#include <ecolab_epilogue.h>

namespace minsky
{
  float OpVarBaseAttributes::x() const
  {
    if (group>=0)
      return m_x + minsky().groupItems[group].x();
    else
      return m_x;
  }

  float OpVarBaseAttributes::y() const
  {
    if (group>=0)
      return m_y + minsky().groupItems[group].y();
    else
      return m_y;
  }

  void OpVarBaseAttributes::move(float x1, float y1)
  {
    m_x+=x1; m_y+=y1;
    for (int i: ports())
      minsky().movePort(i, x1, y1);
  }

  void OpVarBaseAttributes::zoom(float xOrigin, float yOrigin,float factor)
  {
    if (visible)
      {
        if (group==-1)
          {
            minsky::zoom(m_x,xOrigin,factor);
            minsky::zoom(m_y,yOrigin,factor);
          }
        else
          {
            m_x*=factor;
            m_y*=factor;
          }
        zoomFactor*=factor;
        setZoomOnAttachedVariable(zoomFactor);
      }
  }

 
}
