/*
  @copyright Steve Keen 2019
  @author Russell Standish
  @author Wynand Dednam
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

#ifndef PARAMETERSHEET_H
#define PARAMETERSHEET_H
#include <cairoSurfaceImage.h>
#include <parVarSheet.h>

namespace minsky
{
	 
  class ParameterSheet: public ecolab::CairoSurface, public ParVarSheet
  {  
  public:
    bool variableSelector(ItemPtr i) override {return i->variableCast() && i->variableCast()->type()==VariableType::parameter;} 
    float offsx=0, offsy=0;       
    float width() const override {return m_width;}
    float height() const override {return m_height;}
    void redraw(int, int, int width, int height) override;
    void requestRedraw() {if (surface.get()) surface->requestRedraw();}
  };
  
}
#include "parameterSheet.cd"
#endif
