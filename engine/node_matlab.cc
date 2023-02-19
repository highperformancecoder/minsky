/*
  @copyright Steve Keen 2018
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

#include "minsky.h"
#include "equations.h"
#include "userFunction.h"
#include "minsky_epilogue.h"
using namespace minsky;

namespace MathDAG
{
  namespace
  {
   struct InvalidChar
    {
      bool operator()(char c) const {return !isalnum(c) && c!='_';}
    };

    string validMatlabIdentifier(string name)
    {
      name.erase(remove_if(name.begin(), name.end(), InvalidChar()), name.end());
      if (name.empty() || isdigit(name[0]))
        name="_"+name;
      return name;
    }

  }

  string matlabInit(const string& init)
  {
    if (init.empty()) return "0";
    VariableValue v;
    v.init=init;
    auto t=cminsky().variableValues.initValue(v);
    string r;
    switch (t.rank())
      {
      case 0: return str(t[0]);
      case 1: r="[";
        for (auto i: t) r+=str(i)+",";
        return r+"]";
      case 2:
        r="[";
        for (size_t i=0; i<t.hypercube().xvectors[1].size(); ++i)
          {
            for (size_t j=0; j<t.hypercube().xvectors[0].size(); ++j)
              r+=str(t[i*t.hypercube().xvectors[0].size()+j])+",";
            r+=";";
          }
        return r+"]";
      default:
        throw error("high rank tensors not supported in Matlab");
      }
  }
      
  ostream& VariableDAG::matlab(ostream& o) const
  {
    if (type==constant)
      return o<<init;
    return o<<validMatlabIdentifier(name);
  }

  template <>
  ostream& OperationDAG<OperationType::constant>::matlab(ostream& o) const
  {
    return o<<init;
  }

  template <>
  ostream& OperationDAG<OperationType::add>::matlab(ostream& o) const
  {
    if (arguments.empty()||
        (arguments[0].empty() &&
         (arguments.size()<2||arguments[1].empty())))
      return o<<0;
    for (size_t i=0; i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"+";
        o<<arguments[0][i]->matlab();
      }
    if (arguments.size()>1)
      {
        if (!arguments[0].empty() && !arguments[1].empty()) o<<"+";
        for (size_t i=0; i<arguments[1].size(); ++i)
          {
            checkArg(1,i);
            if (i>0) o<<"+";
            o<<arguments[1][i]->matlab();
          }
      }
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::subtract>::matlab(ostream& o) const
  {
    if (arguments.empty()||
        (arguments[0].empty() &&
         (arguments.size()<2||arguments[1].empty())))
      return o<<0;
    for (size_t i=0; i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"+";
        o<<arguments[0][i]->matlab();
      }
    if (arguments.size()>1 && !arguments[1].empty()>0) 
      {
        o<<"-(";
        for (size_t i=0; i<arguments[1].size(); ++i)
          {
            checkArg(1,i);
            if (i>0) o<<"+";
            o<<arguments[1][i]->matlab();
          }
        o<<")";
      }
    return o;
  }
        
  template <>
  ostream& OperationDAG<OperationType::multiply>::matlab(ostream& o) const
  {
    if (arguments.empty()||
        (arguments[0].empty() &&
         (arguments.size()<2||arguments[1].empty())))
      return o<<1;
    for (size_t i=0; i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"*";
        o<<"("<<arguments[0][i]->matlab()<<")";
      }
    if (!arguments[0].empty() && !arguments[1].empty()) o<<"*";
    for (size_t i=0; i<arguments[1].size(); ++i)
      {
        checkArg(1,i);
        if (i>0) o<<"*";
        o<<"("<<arguments[1][i]->matlab()<<")";
      }
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::divide>::matlab(ostream& o) const
  {
    if (arguments.empty()||
        (arguments[0].empty() &&
         (arguments.size()<2||arguments[1].empty())))
      return o<<1;
    if (arguments[0].empty()) 
      o<<"1";
    for (size_t i=0; i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"*";
        o<<"("<<arguments[0][i]->matlab()<<")";
      }
    if (arguments.size()>1 && !arguments[1].empty()) 
      {
        o<<"/(";
        for (size_t i=0; i<arguments[1].size(); ++i)
          {
            checkArg(1,i);
            if (i>0) o<<"*";
            o<<"("<<arguments[1][i]->matlab()<<")";
          }
        o<<")";
      }
    return o;
  }
  
  template <>
  ostream& OperationDAG<OperationType::log>::matlab(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    return o<<"log("<<arguments[0][0]->matlab()<<")/log("<<
      arguments[1][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::pow>::matlab(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    return  o<<"("<<arguments[0][0]->matlab()<<")^("<<arguments[1][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::lt>::matlab(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      o<<"(("<<arguments[0][0]->matlab()<<")";
    else
      o<<"(0";
    if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
      o<<"<("<<arguments[1][0]->matlab()<<")";
    else
      o<<"<0";
    return o<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::le>::matlab(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      o<<"(("<<arguments[0][0]->matlab()<<")";
    else
      o<<"(0";
    if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
      o<<"<=("<<arguments[1][0]->matlab()<<")";
    else
      o<<"<=0";
    return o<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::eq>::matlab(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      o<<"(("<<arguments[0][0]->matlab()<<")";
    else
      o<<"(0";
    if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
      o<<"==("<<arguments[1][0]->matlab()<<")";
    else
      o<<"==0";
    return o<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::min>::matlab(ostream& o) const
  {
    if (!arguments.empty()  && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"min("<<arguments[0][0]->matlab()<<"," <<
          arguments[1][0]->matlab()<<")";
      else
        o<<"min("<<arguments[0][0]->matlab()<<",0)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"min(0,"<<arguments[1][0]->matlab()<<")";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::max>::matlab(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"max("<<arguments[0][0]->matlab()<<"," <<
          arguments[1][0]->matlab()<<")";
      else
        o<<"max("<<arguments[0][0]->matlab()<<",0)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"max(0,"<<arguments[1][0]->matlab()<<")";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::and_>::matlab(ostream& o) const
  {
    if (arguments.size()>1 && !arguments[0].empty() && arguments[0][0] &&
        !arguments[1].empty() && arguments[1][0])
        o<<"(("<<arguments[0][0]->matlab()<<")>=0.5 && (" <<
          arguments[1][0]->matlab()<<")>=0.5)";
    else
      o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::or_>::matlab(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"(("<<arguments[0][0]->matlab()<<")>=0.5 || (" <<
          arguments[1][0]->matlab()<<")>=0.5)";
      else
        o<<"(("<<arguments[0][0]->matlab()<<")>=0.5)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"(("<<arguments[1][0]->matlab()<<")>=0.5)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::not_>::matlab(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      o<<"(("<<arguments[0][0]->matlab()<<")<0.5)";
    else
      o<<"1";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::time>::matlab(ostream& o) const
  {
    return o<<"t";
  }    
 
  template <>
  ostream& OperationDAG<OperationType::euler>::matlab(ostream& o) const
  {
    return o<<"e";
  }

  template <>
  ostream& OperationDAG<OperationType::pi>::matlab(ostream& o) const
  {
    return o<<"pi";
  }
  
  template <>
  ostream& OperationDAG<OperationType::zero>::matlab(ostream& o) const
  {
    return o<<"0";
  }

  template <>
  ostream& OperationDAG<OperationType::one>::matlab(ostream& o) const
  {
    return o<<"1";
  }            
 
  template <>
  ostream& OperationDAG<OperationType::inf>::matlab(ostream& o) const
  {
    return o<<"inf";
  }
  
  template <>
  ostream& OperationDAG<OperationType::percent>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"100*("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::copy>::matlab(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty())
      {
        assert(arguments[0][0]);
        o<<arguments[0][0]->matlab();
      }
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::integrate>::matlab(ostream& o) const
  {
    throw error("shouldn't be executed");
  }
        
  template <>
  ostream& OperationDAG<OperationType::differentiate>::matlab(ostream& o) const
  {
    throw error("derivative operator should not appear in matlab output");
  }

  template <>
  ostream& OperationDAG<OperationType::data>::matlab(ostream& o) const
  {
    throw error("data blocks not yet supported in Matlab mode");
    return o;
  }
        
  template <>
  ostream& OperationDAG<OperationType::ravel>::matlab(ostream& o) const
  {
    throw error("data blocks not yet supported in Matlab mode");
    return o;
  }
        
  template <>
  ostream& OperationDAG<OperationType::sqrt>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"sqrt("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::exp>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"exp("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::ln>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"log("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::sin>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"sin("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::cos>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"cos("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::tan>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"tan("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::asin>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"asin("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::acos>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"acos("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::atan>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"atan("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::sinh>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"sinh("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::cosh>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"cosh("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::tanh>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"tanh("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::abs>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"abs("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::floor>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"floor("<<arguments[0][0]->matlab()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::frac>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"frac("<<arguments[0][0]->matlab()<<")";
  }
 
  template <>
  ostream& OperationDAG<OperationType::Gamma>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"gamma("<<arguments[0][0]->matlab()<<")";
  }    
  
  template <>
  ostream& OperationDAG<OperationType::polygamma>::matlab(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    return o<<"psi(floor("<<arguments[1][0]->matlab()<<"),("<<arguments[0][0]->matlab()<<"))";
  }       
  
  template <>
  ostream& OperationDAG<OperationType::fact>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"gamma(1+("<<arguments[0][0]->matlab()<<"))";
  }  

  template <>
  ostream& OperationDAG<OperationType::userFunction>::matlab(ostream& o) const
  {
    if (arguments.empty() || arguments[0].empty())
      return o<<dynamic_cast<UserFunction*>(state.get())->name()<<"()";
    if (arguments.size()<2 || arguments[1].empty())
      return o<<dynamic_cast<UserFunction*>(state.get())->name()<<"("<<arguments[0][0]->matlab()<<")";
    return o<<dynamic_cast<UserFunction*>(state.get())->name()<<"("<<arguments[0][0]->matlab()<<","<<arguments[1][0]->matlab()<<")";
  }    

  
  template <>
  ostream& OperationDAG<OperationType::sum>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"sum("<<arguments[0][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::product>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"prod("<<arguments[0][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::supremum>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"max("<<arguments[0][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::infimum>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"min("<<arguments[0][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::supIndex>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"find(("<<arguments[0][0]->matlab()<<")==max("<<arguments[0][0]->matlab()<<"))";
  }
  template <>
  ostream& OperationDAG<OperationType::infIndex>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"find(("<<arguments[0][0]->matlab()<<")==min("<<arguments[0][0]->matlab()<<"))";
  }
  template <>
  ostream& OperationDAG<OperationType::any>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"any(("<<arguments[0][0]->matlab()<<")>0.5)";
  }
  template <>
  ostream& OperationDAG<OperationType::all>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"all(("<<arguments[0][0]->matlab()<<")>0.5)";
  }
  template <>
  ostream& OperationDAG<OperationType::runningSum>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"cumsum("<<arguments[0][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::runningProduct>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"cumprod("<<arguments[0][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::difference>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"diff("<<arguments[0][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::innerProduct>::matlab(ostream& o) const
  {
    checkArg(0,0);
    checkArg(1,0);
    return o<<"("<<arguments[0][0]->matlab()<<")*("<<arguments[1][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::outerProduct>::matlab(ostream& o) const
  {
    checkArg(0,0);
    checkArg(1,0);
    // note matlab can only deal with rank 2 tensors  - this works if both are vectors, or one is a scalar and the other a matrix
    return o<<"("<<arguments[0][0]->matlab()<<").*transpose("<<arguments[1][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::index>::matlab(ostream& o) const
  {
    checkArg(0,0);
    return o<<"find(("<<arguments[0][0]->matlab()<<")>0.5)";
  }
  template <>
  ostream& OperationDAG<OperationType::gather>::matlab(ostream& o) const
  {
    checkArg(0,0);checkArg(1,0);
    return o<<"("<<arguments[0][0]->matlab()<<")("<<arguments[1][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::meld>::matlab(ostream& o) const
  {
    checkArg(0,0);checkArg(1,0);
    return o<<"meld("<<arguments[0][0]->matlab()<<","<<arguments[1][0]->matlab()<<")";
  }
  template <>
  ostream& OperationDAG<OperationType::merge>::matlab(ostream& o) const
  {
    checkArg(0,0);checkArg(1,0);
    return o<<"meld("<<arguments[0][0]->matlab()<<","<<arguments[1][0]->matlab()<<")";
  }

}
