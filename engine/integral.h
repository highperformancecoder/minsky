/*
  @copyright Steve Keen 2016
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

#ifndef INTEGRAL_H
#define INTEGRAL_H
#include "intOp.h"
#include "variableValues.h"

namespace minsky
{
  /// An integral is an additional stock variable, that integrates its flow variable
  class Integral
  {
    VariableValuePtr m_input;
  public:
    VariableValuePtr stock;
    VariableValue& input() {assert(m_input->size()==stock->size()); return *m_input;}
    IntOp* operation; //< reference to the internal operation object
    void setInput(const VariableValuePtr& input) {
      m_input=input;
      if (stock)
        {
          // redimension stock to that of the input
          stock->index(m_input->index());
          stock->hypercube(m_input->hypercube());
        }
    }
  };

}

#endif
