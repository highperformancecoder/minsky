/*
  @copyright Steve Keen 2021
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

#define OPNAMEDEF
#include "pango.h"
#include "operation.h"
#include "lasso.h"
#include "itemT.rcd"
#include "operationType.rcd"
#include "operation.rcd"
#include "minsky_epilogue.h"

template <minsky::OperationType::Type T>
using OpItemT=minsky::ItemT<minsky::Operation<T>,minsky::OperationBase>;

#define DEFOP(type) \
  CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Operation<minsky::OperationType::type>); \
  CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(OpItemT<minsky::OperationType::type>);

DEFOP(constant)
DEFOP(time)
DEFOP(integrate)
DEFOP(differentiate)
DEFOP(data)
DEFOP(ravel)
DEFOP(euler)
DEFOP(pi)
DEFOP(zero)
DEFOP(one)
DEFOP(inf)
DEFOP(percent)
DEFOP(add)
DEFOP(subtract)
DEFOP(multiply)
DEFOP(divide)
DEFOP(min)
DEFOP(max)
DEFOP(and_)
DEFOP(or_)
DEFOP(log)
DEFOP(pow)
DEFOP(polygamma)
DEFOP(lt)
DEFOP(le)
DEFOP(eq)
DEFOP(covariance)
DEFOP(correlation)
DEFOP(linearRegression)
DEFOP(userFunction)
