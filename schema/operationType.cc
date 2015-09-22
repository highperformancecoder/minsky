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

#include <vector>
#include "operationType.h"
#include <ecolab_epilogue.h>


using namespace classdesc;

namespace minsky
{
  string OperationType::typeName(int op) 
  {return enumKey<Type>(op);}

  namespace OperationTypeInfo
  {
    template <> int numArguments<OperationType::constant>() {return 0;}
    template <> int numArguments<OperationType::add>() {return 2;}
    template <> int numArguments<OperationType::subtract>() {return 2;}
    template <> int numArguments<OperationType::multiply>() {return 2;}
    template <> int numArguments<OperationType::divide>() {return 2;}
    template <> int numArguments<OperationType::log>() {return 2;}
    template <> int numArguments<OperationType::pow>() {return 2;}
    template <> int numArguments<OperationType::lt>() {return 2;}
    template <> int numArguments<OperationType::le>() {return 2;}
    template <> int numArguments<OperationType::eq>() {return 2;}
    template <> int numArguments<OperationType::min>() {return 2;}
    template <> int numArguments<OperationType::max>() {return 2;}
    template <> int numArguments<OperationType::and_>() {return 2;}
    template <> int numArguments<OperationType::or_>() {return 2;}
    template <> int numArguments<OperationType::not_>() {return 1;}
    template <> int numArguments<OperationType::time>() {return 0;}
    template <> int numArguments<OperationType::copy>() {return 1;}
    template <> int numArguments<OperationType::integrate>() {return 1;}
    template <> int numArguments<OperationType::differentiate>() {return 1;}
    template <> int numArguments<OperationType::data>() {return 1;}
    template <> int numArguments<OperationType::sqrt>() {return 1;}
    template <> int numArguments<OperationType::exp>() {return 1;}
    template <> int numArguments<OperationType::ln>() {return 1;}
    template <> int numArguments<OperationType::sin>() {return 1;}
    template <> int numArguments<OperationType::cos>() {return 1;}
    template <> int numArguments<OperationType::tan>() {return 1;}
    template <> int numArguments<OperationType::asin>() {return 1;}
    template <> int numArguments<OperationType::acos>() {return 1;}
    template <> int numArguments<OperationType::atan>() {return 1;}
    template <> int numArguments<OperationType::sinh>() {return 1;}
    template <> int numArguments<OperationType::cosh>() {return 1;}
    template <> int numArguments<OperationType::tanh>() {return 1;}
    template <> int numArguments<OperationType::abs>() {return 1;}
    template <> int numArguments<OperationType::numOps>() {return -1;} //no output port as well
  }
}
