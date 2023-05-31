/*
  @copyright Steve Keen 2021
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

#include "parameterTab.h"
#include "selection.h"
#include "lasso.h"
#include "plotWidget.h"

#include "itemTab.rcd"
#include "itemTab.xcd"
#include "pannableTab.rcd"
#include "pannableTab.xcd"
#include "pango.rcd"
#include "pango.xcd"
#include "parameterTab.rcd"
#include "parameterTab.xcd"
#include "minsky_epilogue.h"
using namespace std;
namespace minsky
{
  ecolab::Pango& ParameterTab::cell(unsigned row, unsigned col)
  {
    if (row>0 && col==1) // override the definition column
      {
        if (auto v=itemVector[row-1]->variableCast())
          {
            cellPtr.reset(surface->cairo());
            string dimString;
            auto dims=v->dims();
            for (size_t i=0; i<dims.size(); ++i)
              dimString+=(i?",":"")+to_string(dims[i]);
            cellPtr->setText(dimString);
          }
        return *cellPtr;
      }
    return ItemTab::cell(row,col);
  }
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::ParameterTab);
