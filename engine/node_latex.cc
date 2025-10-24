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
  string latex(double x)
  {
    if (abs(x)>0 && (abs(x)>=1e5 || abs(x)<=1e-4))
      {
        int exponent=static_cast<int>(log10(abs(x)));
        if (exponent<0) exponent++;
        return str(x/pow(10.0,exponent))+"\\times10^{"+str(exponent)+"}";
      }
    return str(x);
  }

  // named constants for group identities

  string mathrm(const string& nm)
  {
    // process super/sub scripts
    string::size_type ss;
    if ((ss=nm.find_first_of("_^"))!=string::npos)
      return mathrm(nm.substr(0, ss)) + nm[ss] + "{"+mathrm(nm.substr(ss+1))+"}";
      
    // process % chars
    string::size_type pc;
    if ((pc=nm.find_first_of('%'))!=string::npos)
      if (pc>0 && nm[pc-1]!='\\')
        return mathrm(nm.substr(0, pc)) + "\\" + nm[pc] + mathrm(nm.substr(pc+1));      
    
    // if its a single letter variable, or contains LaTeX codes, process as is
    if (nm.length()==1 || nm.find('\\')!=string::npos)
      return nm;
    return "\\mathrm{"+nm+"}";
  }

  namespace
  {
    // RAII class that conditionally writes a left parenthesis on
    // construction, and right parenthesis on destruction
    struct ParenIf
    {
      ostream& o;
      bool c;
      ParenIf(ostream& o, bool c): o(o), c(c)  {if (c) o<<"\\left(";}
      ~ParenIf() {if (c) o<<"\\right)";}
    };
  }
 
  string latexInit(const string& init)
  {
    if (init.empty()) return "0";
    VariableValue v;
    v.init(init);
    auto t=cminsky().variableValues.initValue(v);
    string r;
    switch (t.rank())
      {
      case 0: return str(t[0]);
      case 1: r="(";
        for (size_t i=0; i<5 && i<t.size(); ++i)
          {
            if (i>0) r+=' ';
            r+=str(t[i]);
          }
        if (t.size()>5)
          r+="\\ldots";
        return r+")";
      default:
        return "\\ldots"; //TODO can we represent this stuff reasonably?
      }
  }
      

  ostream& VariableDAG::latex(ostream& o) const
  {
    if (type==constant)
      return o<<init;
    return o<<mathrm(name);
  }

  template <>
  ostream& OperationDAG<OperationType::constant>::latex(ostream& o) const
  {
    return o<<mathrm(name);
  }

  template <>
  ostream& OperationDAG<OperationType::data>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<mathrm(name)<<"("<<arguments[0][0]->latex()<<")";
  }

  template <>
  ostream& OperationDAG<OperationType::ravel>::latex(ostream& o) const
  {
    o<<mathrm(name);
    if (!arguments.empty() && !arguments[0].empty())
      return o<<"("<<arguments[0][0]->latex()<<")";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::add>::latex(ostream& o) const
  {
    for (size_t i=0; !arguments.empty() && i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"+";
        o<<arguments[0][i]->latex();
      }
    if (arguments.size()>1 && !arguments[0].empty() && !arguments[1].empty()) o<<"+";
    for (size_t i=0; arguments.size()>1 && i<arguments[1].size(); ++i)
      {
        checkArg(1,i);
        if (i>0) o<<"+";
        o<<arguments[1][i]->latex();
      }
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::subtract>::latex(ostream& o) const
  {
    for (size_t i=0; !arguments.empty() && i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"+";
        o<<arguments[0][i]->latex();
      }
    if (arguments.size()>1 && !arguments[1].empty()) 
      {
        checkArg(1,0);
        o<<"-";
        const ParenIf p(o, (arguments[1].size()>1 || 
                      BODMASlevel() == arguments[1][0]->BODMASlevel()));
        for (size_t i=0; i<arguments[1].size(); ++i)
          {
            checkArg(1,i);
            if (i>0) o<<"+";
            o<<arguments[1][i]->latex();
          }
      }
    return o;
  }
        
  template <>
  ostream& OperationDAG<OperationType::multiply>::latex(ostream& o) const
  {
    for (size_t i=0; !arguments.empty() && i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"\\times ";
        const ParenIf p(o, arguments[0][i]->BODMASlevel()>BODMASlevel());
        o<<arguments[0][i]->latex();
      }
    if (arguments.size()>1 && !arguments[0].empty() && !arguments[1].empty()) o<<"\\times ";
    for (size_t i=0; arguments.size()>1 && i<arguments[1].size(); ++i)
      {
        checkArg(1,i);
        if (i>0) o<<"\\times ";
        const ParenIf p(o, arguments[1][i]->BODMASlevel()>BODMASlevel());
        o<<arguments[1][i]->latex();
      }
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::divide>::latex(ostream& o) const
  {
    if (arguments.empty()) return o;
    if (arguments.size()>1) o<< "\\frac{";
    if (arguments[0].empty()) o<<"1";
    for (size_t i=0; i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"\\times ";
        const ParenIf p(o, i>0 && arguments[0][i]->BODMASlevel()>BODMASlevel());
        o<<arguments[0][i]->latex();
      }
    if (arguments.size()>1) 
      {
        o<<"}{";
        if (arguments[1].empty()) o<<"1";
        for (size_t i=0; i<arguments[1].size(); ++i)
          {
            checkArg(1,i);
            if (i>0) o<<"\\times ";
            const ParenIf p(o, i>0 && arguments[1][i]->BODMASlevel()>BODMASlevel());
            o<<arguments[1][i]->latex();
          }
        o<<"}";
      }
    return o;
  }
  
  template <>
  ostream& OperationDAG<OperationType::log>::latex(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    return o<<"\\log_{"<<arguments[1][0]->latex()<<"}\\left("<<
      arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::pow>::latex(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    {
      const ParenIf p(o, arguments[0][0]->BODMASlevel()>BODMASlevel());
      o<<arguments[0][0]->latex();
    }
    return o<<"^{"<<arguments[1][0]->latex()<<"}";
  }

  template <>
  ostream& OperationDAG<OperationType::lt>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        o<<"\\theta\\left(";
        if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
          o<<arguments[1][0]->latex()<<"-";
        else
          o<<"-";
        {
          const ParenIf p(o, arguments[0][0]->BODMASlevel()>1);
          o<<arguments[0][0]->latex();
        }
        o<<"\\right)";
      }
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\theta\\left("<<arguments[1][0]->latex()<<"\\right)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::eq>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        {
          o<<"\\delta\\left("<<arguments[0][0]->latex()<<"-";
          {
            const ParenIf p(o, arguments[1][0]->BODMASlevel()>BODMASlevel());
            o<<arguments[1][0]->latex();
          }
          o <<"\\right)";
        }
      else
        o<<"\\delta\\left("<<arguments[0][0]->latex()<<"\\right)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\delta\\left("<<arguments[1][0]->latex()<<"\\right)";
      else
        o<<"1";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::le>::latex(ostream& o) const
  {
    if ((!arguments.empty() && !arguments[0].empty() && arguments[0][0]) ||
        (arguments.size()>1 && !arguments[1].empty() && arguments[1][0]))
      {
        OperationDAG<OperationType::lt> lt; lt.arguments=arguments;
        OperationDAG<OperationType::eq> eq; eq.arguments=arguments;
        lt.latex(o);
        o<<"+";
        return eq.latex(o);
      }
    return o<<"1"<<endl;
  }


  template <>
  ostream& OperationDAG<OperationType::min>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\min\\left("<<arguments[0][0]->latex()<<"," <<
          arguments[1][0]->latex()<<"\\right)";
      else
        o<<"\\min\\left("<<arguments[0][0]->latex()<<",0\\right)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\min\\left("<<arguments[1][0]->latex()<<",0\\right)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::max>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\max\\left("<<arguments[0][0]->latex()<<"," <<
          arguments[1][0]->latex()<<"\\right)";
      else
        o<<"\\max\\left("<<arguments[0][0]->latex()<<",0\\right)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\max\\left("<<arguments[1][0]->latex()<<",0\\right)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::and_>::latex(ostream& o) const
  {
    if (arguments.size()>1 && !arguments[0].empty() && arguments[0][0] && 
        !arguments[1].empty() && arguments[1][0])
      o<<"\\theta\\left("<<arguments[0][0]->latex()<<"-0.5\\right)\\theta\\left(" <<
        arguments[1][0]->latex()<<"-0.5\\right)";
    else
      o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::or_>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\max\\left(\\theta\\left("<<arguments[0][0]->latex()<<"-0.5\\right)," <<
          "\\theta\\left("<<arguments[1][0]->latex()<<"\\right)\\right)";
      else
        o<<"\\theta\\left("<<arguments[0][0]->latex()<<"-0.5\\right)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\theta\\left("<<arguments[1][0]->latex()<<"-0.5\\right)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::not_>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      o<<"\\left(1-\\theta\\left(0.5-"<<arguments[0][0]->latex()<<"\\right)\\right)";
    else
      o<<"1";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::covariance>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0] &&
        arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
      return o<<"{\\mathrm cov}\\left("<<arguments[0][0]->latex()<<
        ","<<arguments[1][0]<<"\\right)";
    return o<<"0";
  }

  template <>
  ostream& OperationDAG<OperationType::correlation>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0] &&
        arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
      return o<<"\\rho\\left("<<arguments[0][0]->latex()<<
        ","<<arguments[1][0]<<"\\right)";
    return o<<"0";
  }

  template <>
  ostream& OperationDAG<OperationType::linearRegression>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0] &&
        arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
      return o<<"{\\mathrm{linReg}\\left("<<arguments[0][0]->latex()<<
        ","<<arguments[1][0]<<"\\right)";
    return o<<"0";
  }

  
  
  template <>
  ostream& OperationDAG<OperationType::size>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        size_t dim=numeric_limits<size_t>::max();
        if (auto op=dynamic_cast<OperationBase*>(state.get()))
          if (auto vv=arguments[0][0]->result)
            {
              for (auto& i: vv->hypercube().xvectors)
                if (i.name==op->axis)
                  {
                    dim=&i-&vv->hypercube().xvectors.front();
                    break;
                  }
              if (dim<vv->rank())
                return o<<"\\dim\\left("<<arguments[0][0]->latex()<<","<<dim<<"\\right)";
            }
        return o<<"\\prod_i(\\dim("<<arguments[0][0]->latex()<<",i))";
      }
    return o<<"0";
  }

  template <>
  ostream& OperationDAG<OperationType::shape>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        return o<<"shape("<<arguments[0][0]->latex()<<")";
      }
    return o<<"0";
  }

  template <>
  ostream& OperationDAG<OperationType::mean>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        return o<<"\\left\\langle"<<arguments[0][0]->latex()<<"\\right\\rangle";
      }
    return o<<"0";
  }

  template <>
  ostream& OperationDAG<OperationType::median>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        return o<<"{\\mathrm median}\\left("<<arguments[0][0]->latex()<<"\\right)";
      }
    return o<<"0";
  }

  template <>
  ostream& OperationDAG<OperationType::stdDev>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        return o<<"\\sigma("<<arguments[0][0]->latex()<<")";
      }
    return o<<"0";
  }

  template <>
  ostream& OperationDAG<OperationType::moment>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        double exponent=1;
        if (auto op=dynamic_cast<OperationBase*>(state.get()))
          exponent=op->arg;
        return o<<"\\left\\langle\\Delta\\left("<<arguments[0][0]->latex()<<"\\right)^"<<exponent<<"\\right\\rangle";
      }
    return o<<"0";
  }

  template <>
  ostream& OperationDAG<OperationType::histogram>::latex(ostream& o) const
  {
    if (!arguments.empty() && !arguments[0].empty() && arguments[0][0])
      {
        size_t nBins=1;
        if (auto op=dynamic_cast<OperationBase*>(state.get()))
          nBins=op->arg;
        return o<<"{\\mathrm histogram}\\left("<<arguments[0][0]->latex()<<","<<nBins<<"\\right)";
      }
    return o<<"0";
  }

  
  template <>
  ostream& OperationDAG<OperationType::time>::latex(ostream& o) const
  {
    return o<<" t ";
  }
  
  template <>
  ostream& OperationDAG<OperationType::euler>::latex(ostream& o) const
  {
    return o<<" e ";
  }
 
  template <>
  ostream& OperationDAG<OperationType::pi>::latex(ostream& o) const
  {
    return o<<"\\pi ";
  }    
  
  template <>
  ostream& OperationDAG<OperationType::zero>::latex(ostream& o) const
  {
    return o<<" 0 ";
  }
 
  template <>
  ostream& OperationDAG<OperationType::one>::latex(ostream& o) const
  {
    return o<<" 1 ";
  }        
  
  template <>
  ostream& OperationDAG<OperationType::inf>::latex(ostream& o) const
  {
    return o<<"\\infty ";
  }
  
  template <>
  ostream& OperationDAG<OperationType::percent>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\left("<<arguments[0][0]->latex()<<"\\right)\\%";
  } 

  template <>
  ostream& OperationDAG<OperationType::copy>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::integrate>::latex(ostream&) const
  {
    throw error("shouldn't be executed");
  }
        
  template <>
  ostream& OperationDAG<OperationType::differentiate>::latex(ostream&) const
  {
    throw error("derivative operator should not appear in LaTeX output");
  }

  template <>
  ostream& OperationDAG<OperationType::sqrt>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\sqrt{"<<arguments[0][0]->latex()<<"}";
  }

  template <>
  ostream& OperationDAG<OperationType::exp>::latex(ostream& o) const
  {
    checkArg(0,0);
    o<<"\\exp\\left("<<arguments[0][0]->latex()<<"\\right)";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::ln>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\ln\\left("<<arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::sin>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\sin\\left("<<arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::cos>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\cos\\left("<<arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::tan>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\tan\\left("<<arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::asin>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\arcsin\\left("<<arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::acos>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\arccos\\left("<<arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::atan>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\arctan\\left("<<arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::sinh>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\sinh\\left("<<arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::cosh>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\cosh\\left("<<arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::tanh>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\tanh\\left("<<arguments[0][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::abs>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\left|"<<arguments[0][0]->latex()<<"\\right|";
  }

  template <>
  ostream& OperationDAG<OperationType::floor>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\left\\lfloor"<<arguments[0][0]->latex()<<"\\right\\rfloor)";
  }

  template <>
  ostream& OperationDAG<OperationType::frac>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\mathrm{frac}("<<arguments[0][0]->latex()<<")";
  }
  
  template <>
  ostream& OperationDAG<OperationType::Gamma>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\Gamma\\left("<<arguments[0][0]->latex()<<"\\right)";
  }  
  
  template <>
  ostream& OperationDAG<OperationType::polygamma>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\psi^{\\left("<<arguments[1][0]->latex()<<"\\right)}\\left("<<arguments[0][0]->latex()<<"\\right)";
  }            
  
  template <>
  ostream& OperationDAG<OperationType::fact>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\left("<<arguments[0][0]->latex()<<"\\right)!";
  }    

  template <>
  ostream& OperationDAG<OperationType::userFunction>::latex(ostream& o) const
  {
    auto desc=mathrm(dynamic_cast<UserFunction&>(*state).description());
    if (arguments.empty() || arguments[0].empty())
      return o<<desc<<"()";
    if (arguments.size()<2 || arguments[1].empty())
      return o<<desc<<"\\left("<<arguments[0][0]->latex()<<"\\right)";
    return o<<desc<<"\\left("<<arguments[0][0]->latex()<<","<<arguments[1][0]->latex()<<"\\right)";
  }    

  template <>
  ostream& OperationDAG<OperationType::sum>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\sum_i("<<arguments[0][0]->latex()<<")_i";
  }

  template <>
  ostream& OperationDAG<OperationType::product>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\prod_i("<<arguments[0][0]->latex()<<")_i";
  }

  template <>
  ostream& OperationDAG<OperationType::infimum>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\min_i("<<arguments[0][0]->latex()<<")_i";
  }

  template <>
  ostream& OperationDAG<OperationType::supremum>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\max_i("<<arguments[0][0]->latex()<<")_i";
  }
  
  template <>
  ostream& OperationDAG<OperationType::infIndex>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\mathrm{indexOf}(\\min_i("<<arguments[0][0]->latex()<<")_i)";
  }

  template <>
  ostream& OperationDAG<OperationType::supIndex>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\mathrm{indexOf}(\\max_i("<<arguments[0][0]->latex()<<")_i)";
  }

  template <>
  ostream& OperationDAG<OperationType::any>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\theta\\left(\\sum_i\\theta\\left(\\left("<<arguments[0][0]->latex()<<"\\right)_i-0.5\\right)\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::all>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\prod_i\\theta\\left(\\left("<<arguments[0][0]->latex()<<"\\right)_i-0.5\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::runningSum>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\left[\\sum_{j=0}^i\\left("<<arguments[0][0]->latex()<<"\\right)_j\\right]";
  }
  
  template <>
  ostream& OperationDAG<OperationType::runningAv>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\left[\\frac1i\\sum_{j=0}^i\\left("<<arguments[0][0]->latex()<<"\\right)_j\\right]";
  }

  template <>
  ostream& OperationDAG<OperationType::runningProduct>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\left[\\prod_{j=0}^i\\left("<<arguments[0][0]->latex()<<"\\right)_j\\right]";
  }

  template <>
  ostream& OperationDAG<OperationType::difference>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\left[\\Delta^-\\left("<<arguments[0][0]->latex()<<"\\right)_i\\right]";
  }

  template <>
  ostream& OperationDAG<OperationType::differencePlus>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\left[\\Delta^+\\left("<<arguments[0][0]->latex()<<"\\right)_i\\right]";
  }

  template <>
  ostream& OperationDAG<OperationType::innerProduct>::latex(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    return o<<arguments[0][0]->latex()<<"\\cdot"<<arguments[1][0]->latex();
  }

  template <>
  ostream& OperationDAG<OperationType::outerProduct>::latex(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    return o<<arguments[0][0]->latex()<<"\\otimes"<<arguments[1][0]->latex();
  }

  template <>
  ostream& OperationDAG<OperationType::index>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o<<"\\left\\{i: \\left("<<arguments[0][0]->latex()<<"\\right)_i>0.5\\right\\}";
  }

  template <>
  ostream& OperationDAG<OperationType::gather>::latex(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    return o<<"\\left[\\left("<<arguments[0][0]->latex()<<"\\right)_j : j=\\left("<<arguments[1][0]->latex()<<"\\right)_i\\right]";
  }
  template <>
  ostream& OperationDAG<OperationType::meld>::latex(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    return o<<"\\mathrm{meld}\\left("<<arguments[0][0]->latex()<<","<<arguments[1][0]->latex()<<"\\right)";
  }
  template <>
  ostream& OperationDAG<OperationType::merge>::latex(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    return o<<"\\mathrm{merge}\\left("<<arguments[0][0]->latex()<<","<<arguments[1][0]->latex()<<"\\right)";
  }

  template <>
  ostream& OperationDAG<OperationType::slice>::latex(ostream& o) const
  {
    checkArg(0,0); checkArg(1,0);
    double slice=0;
    if (state)
      if (auto o=state->operationCast())
        slice=o->arg;
    return o<<"\\mathrm{slice}\\left("<<arguments[0][0]->latex()<<","<<slice<<"\\right)";
  }

  
  
}
