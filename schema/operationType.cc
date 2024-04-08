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
#include "minsky_epilogue.h"


using namespace classdesc;

namespace minsky
{
  string OperationType::typeName(int type) 
  {return enumKey<Type>(type);}

  OperationType::Group OperationType::classify(Type t)
  {
    if (t<euler) return general;
    if (t<add) return constop; 
    if (t<copy) return binop;     
    if (t<sum) return function;
    if (t<runningSum) return reduction;
    if (t<innerProduct) return scan;
    if (t<mean) return tensor;
    return statistics;
  }

  
  namespace OperationTypeInfo
  {
    template <> int numArguments<OperationType::constant>() {return 0;}
    template <> int numArguments<OperationType::euler>() {return 0;}
    template <> int numArguments<OperationType::pi>() {return 0;}
    template <> int numArguments<OperationType::zero>() {return 0;}
    template <> int numArguments<OperationType::one>() {return 0;}                
    template <> int numArguments<OperationType::inf>() {return 0;}    
    template <> int numArguments<OperationType::percent>() {return 1;}  
    template <> int numArguments<OperationType::add>() {return 2;}
    template <> int numArguments<OperationType::subtract>() {return 2;}
    template <> int numArguments<OperationType::multiply>() {return 2;}
    template <> int numArguments<OperationType::divide>() {return 2;}
    template <> int numArguments<OperationType::log>() {return 2;}
    template <> int numArguments<OperationType::pow>() {return 2;}
    template <> int numArguments<OperationType::polygamma>() {return 2;}       
    template <> int numArguments<OperationType::lt>() {return 2;}
    template <> int numArguments<OperationType::le>() {return 2;}
    template <> int numArguments<OperationType::eq>() {return 2;}
    template <> int numArguments<OperationType::min>() {return 2;}
    template <> int numArguments<OperationType::max>() {return 2;}
    template <> int numArguments<OperationType::and_>() {return 2;}
    template <> int numArguments<OperationType::or_>() {return 2;}
    template <> int numArguments<OperationType::covariance>() {return 2;}
    template <> int numArguments<OperationType::rho>() {return 2;}
    template <> int numArguments<OperationType::linearRegression>() {return 2;}
    template <> int numArguments<OperationType::userFunction>() {return 2;}
    template <> int numArguments<OperationType::not_>() {return 1;}
    template <> int numArguments<OperationType::time>() {return 0;}    
    template <> int numArguments<OperationType::copy>() {return 1;} 
    template <> int numArguments<OperationType::integrate>() {return 2;}
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
    template <> int numArguments<OperationType::floor>() {return 1;}
    template <> int numArguments<OperationType::frac>() {return 1;}
    template <> int numArguments<OperationType::Gamma>() {return 1;}           
    template <> int numArguments<OperationType::fact>() {return 1;}        
    template <> int numArguments<OperationType::sum>() {return 1;}
    template <> int numArguments<OperationType::product>() {return 1;}
    template <> int numArguments<OperationType::infimum>() {return 1;}
    template <> int numArguments<OperationType::supremum>() {return 1;}
    template <> int numArguments<OperationType::infIndex>() {return 1;}
    template <> int numArguments<OperationType::supIndex>() {return 1;}
    template <> int numArguments<OperationType::size>() {return 1;}
    template <> int numArguments<OperationType::shape>() {return 1;}
    template <> int numArguments<OperationType::mean>() {return 1;}
    template <> int numArguments<OperationType::median>() {return 1;}
    template <> int numArguments<OperationType::stdDev>() {return 1;}
    template <> int numArguments<OperationType::moment>() {return 1;}
    template <> int numArguments<OperationType::histogram>() {return 1;}
    
    template <> int numArguments<OperationType::any>() {return 1;}
    template <> int numArguments<OperationType::all>() {return 1;}
    template <> int numArguments<OperationType::runningSum>() {return 1;}
    template <> int numArguments<OperationType::runningProduct>() {return 1;}
    template <> int numArguments<OperationType::difference>() {return 1;}
    template <> int numArguments<OperationType::innerProduct>() {return 2;}
    template <> int numArguments<OperationType::outerProduct>() {return 2;}
    template <> int numArguments<OperationType::index>() {return 1;}
    template <> int numArguments<OperationType::gather>() {return 2;} 
    template <> int numArguments<OperationType::meld>() {return 2;} 
    template <> int numArguments<OperationType::merge>() {return 2;} 
    template <> int numArguments<OperationType::slice>() {return 1;} 
    template <> int numArguments<OperationType::ravel>() {return 1;}     
    template <> int numArguments<OperationType::numOps>() {return -1;} //no output port as well
  }
}
