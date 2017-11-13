/*
  @copyright Steve Keen 2012
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

#include "equations.h"
#include "expr.h"
#include "minsky.h"
#include "str.h"
#include "flowCoef.h"
#include <ecolab_epilogue.h>
using namespace minsky;

namespace MathDAG
{

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

    // comparison function for sorting variables into their definition order
    struct VariableDefOrder
    {
      unsigned maxOrder;
      VariableDefOrder(unsigned maxOrder): maxOrder(maxOrder) {}
      bool operator()(const VariableDAG* x, const VariableDAG* y) {
        return x->order(maxOrder)<y->order(maxOrder);
      }
    };

    struct NoArgument: public std::exception
    {
      OperationPtr state;
      unsigned argNum1, argNum2;
      NoArgument(const OperationPtr& s, unsigned a1, unsigned a2): 
        state(s), argNum1(a1), argNum2(a2) {}
      const char* what() const noexcept override {
        string r="missing argument "+to_string(argNum1)+","+to_string(argNum2)+
          " on operation ";
        if (state)
          {
            minsky::minsky().displayErrorItem(*state);
            r+=OperationType::typeName(state->type());
          }
        
        return r.c_str();
      }
    };

  }

  // named constants for group identities

  string mathrm(const string& nm)
  {
    // process super/sub scripts
    string::size_type ss;
    if ((ss=nm.find_first_of("_^"))!=string::npos)
      return mathrm(nm.substr(0, ss)) + nm[ss] + "{"+mathrm(nm.substr(ss+1))+"}";

    // if its a single letter variable, or contains LaTeX codes, process as is
    if (nm.length()==1 || nm.find('\\')!=string::npos)
      return nm;
    else
      return "\\mathrm{"+nm+"}";
  }

  string latex(double x)
  {
    if (abs(x)>0 && (abs(x)>=1e5 || abs(x)<=1e-4))
      {
        int exponent=log10(abs(x));
        if (exponent<0) exponent++;
        return str(x/pow(10.0,exponent))+"\\times10^{"+str(exponent)+"}";
      }
    else
      return str(x);
  }

  VariableValue ConstantDAG::addEvalOps
  (EvalOpVector& ev, const VariableValue& r) const 
  {
    if (result.idx()<0)
      {
        if (r.idx()>=0 && r.isFlowVar())
          result=r;
        else
          result.allocValue();
        result=value;
        // set the initial value of the actual variableValue (if it exists)
        VariableValues& values=minsky::minsky().variableValues;
        auto v=values.find(result.name);
        if (v!=values.end())
          v->second.init=str(value);
      }
    if (r.isFlowVar() && r.idx()>=0 && r.idx()!=result.idx())
      ev.push_back(EvalOpPtr(OperationType::copy, r, result));
    return result;
  }

  ostream& VariableDAG::latex(ostream& o) const
  {
    if (type==constant)
      return o<<init;
    return o<<mathrm(name);
  }

  ostream& VariableDAG::matlab(ostream& o) const
  {
    if (type==constant)
      return o<<init;
    return o<<validMatlabIdentifier(name);
  }

  VariableValue VariableDAG::addEvalOps
  (EvalOpVector& ev, const VariableValue& r) const
  {
    if (result.idx()<0)
      {
        assert(VariableValue::isValueId(valueId));
        auto ri=minsky::minsky().variableValues.find(valueId);
        if (ri!=minsky::minsky().variableValues.end())
          result=ri->second;
        else
          result=VariableValue(VariableType::tempFlow);
        if (result.idx()==-1)
          result.allocValue();
        if (rhs)
          rhs->addEvalOps(ev, result);
      }
    if (r.isFlowVar() && r.idx()>=0 && (r.idx()!=result.idx() || !result.isFlowVar()))
      ev.push_back(EvalOpPtr(OperationType::copy, r, result));
    return result;
  }

  VariableValue IntegralInputVariableDAG::addEvalOps
  (EvalOpVector& ev, const VariableValue& r) const
  {
    if (result.idx()<0)
      {
        if (r.idx()>=0 && r.isFlowVar())
          result=r;
        else
          {
            result=VariableValue(VariableType::tempFlow);
            result.allocValue();
          }
        if (rhs)
          rhs->addEvalOps(ev, result);
      }
    if (r.isFlowVar() && r.idx()>=0 && (r.idx()!=result.idx() || !result.isFlowVar()))
      ev.push_back(EvalOpPtr(OperationType::copy, r, result));
    return result;
  }

  namespace {OperationFactory<OperationDAGBase, OperationDAG, 
                              OperationType::numOps-1> operationDAGFactory;}

  OperationDAGBase* OperationDAGBase::create(Type type, const string& name)
  {
    auto r=operationDAGFactory.create(type);
    r->name=name;
    return r;
  }

  // the definition order of an operation is simply the maximum of all
  // of its arguments
  int OperationDAGBase::order(unsigned maxOrder) const
  {
    if (type()==integrate)
      return 0; //integrals have already been evaluated

    if (maxOrder==0)
      throw error("maximum order recursion reached");

    // constants have order one, as they must be ordered after the
    // "fake" variables have been initialised
    int order=type()==constant? 1: 0;
    for (size_t i=0; i<arguments.size(); ++i)
      for (size_t j=0; j<arguments[i].size(); ++j)
        {
          checkArg(i,j);
          order=std::max(order, arguments[i][j]->order(maxOrder-1));
        }
    return order;
  }

  void OperationDAGBase::checkArg(unsigned i, unsigned j) const
  {
    if (arguments.size()<=i || arguments[i].size()<=j || !arguments[i][j])
      throw NoArgument(state, i, j);
  }


  namespace
  {
    void cumulate(EvalOpVector& ev, VariableValue& r, const vector<vector<VariableValue> >& argIdx,
                  OperationType::Type op, OperationType::Type accum, double groupIdentity)
    {
      if (argIdx.size()>0 && !argIdx[0].empty())
        {
          ev.push_back(EvalOpPtr(OperationType::copy, r, argIdx[0][0]));
          for (size_t i=1; i<argIdx[0].size(); ++i)
            ev.push_back(EvalOpPtr(accum, r, r, argIdx[0][i]));
        }
      else
        {
          //TODO: could be cleaned up if we don't need to support constant operators
          ev.push_back(EvalOpPtr(OperationType::constant, r));
          dynamic_cast<ConstantEvalOp&>(*ev.back()).value=groupIdentity;
        }

      if (argIdx.size()>1)
        {
          if (argIdx[1].size()==1)
            // eliminate redundant copy operation when only one wire
            ev.push_back(EvalOpPtr(op, r, r, argIdx[1][0]));
          else if (argIdx[1].size()>1)
            {
              // multiple wires to second input port
              VariableValue tmp(VariableType::tempFlow);
              tmp.allocValue();
              ev.push_back(EvalOpPtr(OperationType::copy, tmp, argIdx[1][0]));
              for (size_t i=1; i<argIdx[1].size(); ++i)
                ev.push_back(EvalOpPtr(accum, tmp, tmp, argIdx[1][i]));
              ev.push_back(EvalOpPtr(op, r, r, tmp));
            }
        }
    }
  }

  VariableValue OperationDAGBase::addEvalOps
  (EvalOpVector& ev, const VariableValue& r) const
  {
    if (result.idx()<0)
      {
        assert(!dynamic_cast<IntOp*>(state.get()));
        if (r.isFlowVar() && r.idx()>=0)
          result=r;
        else 
          result.allocValue();

        if (state && !state->ports.empty() && state->ports[0]) 
          state->ports[0]->setVariableValue(result);

        // prepare argument expressions
        vector<vector<VariableValue> > argIdx(arguments.size());
        for (size_t i=0; type()!=integrate && i<arguments.size(); ++i)
          for (size_t j=0; j<arguments[i].size(); ++j)
            if (arguments[i][j])
              argIdx[i].push_back(arguments[i][j]->addEvalOps(ev));
            else
              argIdx[i].push_back(VariableValue());
        
        // basic arithmetic is handled in a cumulative fashion
        switch (type())
          {
          case add:
            cumulate(ev, result, argIdx, add, add, 0);
            break;
          case subtract:
            cumulate(ev, result, argIdx, subtract, add, 0);
            break;
          case multiply:
            cumulate(ev, result, argIdx, multiply, multiply, 1);
            break;
          case divide:
            cumulate(ev, result, argIdx, divide, multiply, 1);
            break;
          case min:
            cumulate(ev, result, argIdx, min, min, numeric_limits<double>::max());
            break;
          case max:
            cumulate(ev, result, argIdx, max, max, numeric_limits<double>::min());
            break;
          case and_:
            cumulate(ev, result, argIdx, and_, and_, 1);
            break;
          case or_:
            cumulate(ev, result, argIdx, or_, or_, 0);
            break;
          case constant:
            if (state) minsky::minsky().displayErrorItem(*state);
            throw error("Constant deprecated");
          case lt: case le: case eq:
            for (size_t i=0; i<arguments.size(); ++i)
              if (arguments[i].empty())
                argIdx[i].push_back(VariableValue());
            ev.push_back(EvalOpPtr(type(), result, argIdx[0][0], argIdx[1][0])); 
            break;
          default:
            // sanity check that the correct number of arguments is provided 
            bool correctlyWired=true;
            for (size_t i=0; i<arguments.size(); ++i)
              correctlyWired &= arguments[i].size()==1;
            if (!correctlyWired)
              {
                if (state) minsky::minsky().displayErrorItem(*state);
                throw error("inputs for highlighted operations incorrectly wired");
              }
            
            switch (arguments.size())
              {
              case 0:
                ev.push_back(EvalOpPtr(type(), result));
                break;
              case 1:
                ev.push_back(EvalOpPtr(type(), result, argIdx[0][0])); 
                break;
              case 2:
                ev.push_back(EvalOpPtr(type(), result, argIdx[0][0], argIdx[1][0])); 
                break;
              default:
                throw error("Too many arguments");
              }
          }
        if ((!ev.back()->state || ev.back()->state->type()==numOps) 
            && state && ev.back()->type()==state->type())
          ev.back()->state=state;
      }
    if (type()!=integrate && r.isFlowVar() && r.idx()>=0 && result.idx()!=r.idx())
      ev.push_back(EvalOpPtr(copy, r, result));
    return result;
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
        if (arguments[0].size()>0 && arguments[1].size()) o<<"+";
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
    if (arguments.size()>1 && arguments[1].size()>0) 
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
    if (arguments[0].size()>0 && arguments[1].size()>0) o<<"*";
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
    if (arguments[0].size()==0) 
      o<<"1";
    for (size_t i=0; i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"*";
        o<<"("<<arguments[0][i]->matlab()<<")";
      }
    if (arguments.size()>1 && arguments[1].size()>0) 
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
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
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
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
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
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
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
    if (arguments.size()>0  && !arguments[0].empty() && arguments[0][0])
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
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
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
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
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
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
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
  ostream& OperationDAG<OperationType::copy>::matlab(ostream& o) const
  {
    if (arguments.size()>0 && arguments[0].size()>=1)
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
  ostream& OperationDAG<OperationType::add>::latex(ostream& o) const
  {
    for (size_t i=0; arguments.size()>0 && i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"+";
        o<<arguments[0][i]->latex();
      }
    if (arguments.size()>1 && arguments[0].size()>0 && arguments[1].size()) o<<"+";
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
    for (size_t i=0; arguments.size()>0 && i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"+";
        o<<arguments[0][i]->latex();
      }
    if (arguments.size()>1 && arguments[1].size()>0) 
      {
        checkArg(1,0);
        o<<"-";
        ParenIf p(o, (arguments[1].size()>1 || 
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
    for (size_t i=0; arguments.size()>0 && i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"\\times ";
        ParenIf p(o, arguments[0][i]->BODMASlevel()>BODMASlevel());
        o<<arguments[0][i]->latex();
      }
    if (arguments.size()>1 && arguments[0].size()>0 && arguments[1].size()>0) o<<"\\times ";
    for (size_t i=0; arguments.size()>1 && i<arguments[1].size(); ++i)
      {
        checkArg(1,i);
        if (i>0) o<<"\\times ";
        ParenIf p(o, arguments[1][i]->BODMASlevel()>BODMASlevel());
        o<<arguments[1][i]->latex();
      }
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::divide>::latex(ostream& o) const
  {
    if (arguments.empty()) return o;
    if (arguments.size()>1) o<< "\\frac{";
    if (arguments[0].size()==0) o<<"1";
    for (size_t i=0; i<arguments[0].size(); ++i)
      {
        checkArg(0,i);
        if (i>0) o<<"\\times ";
        ParenIf p(o, i>0 && arguments[0][i]->BODMASlevel()>BODMASlevel());
        o<<arguments[0][i]->latex();
      }
    if (arguments.size()>1) 
      {
        o<<"}{";
        if (arguments[1].size()==0) o<<"1";
        for (size_t i=0; i<arguments[1].size(); ++i)
          {
            checkArg(1,i);
            if (i>0) o<<"\\times ";
            ParenIf p(o, i>0 && arguments[0][i]->BODMASlevel()>BODMASlevel());
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
      ParenIf p(o, arguments[0][0]->BODMASlevel()>BODMASlevel());
      o<<arguments[0][0]->latex();
    }
    return o<<"^{"<<arguments[1][0]->latex()<<"}";
  }

  template <>
  ostream& OperationDAG<OperationType::lt>::latex(ostream& o) const
  {
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
      {
        o<<"\\Theta\\left(";
        if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
          o<<arguments[1][0]->latex()<<"-";
        else
          o<<"-";
        {
          ParenIf p(o, arguments[0][0]->BODMASlevel()>1);
          o<<arguments[0][0]->latex();
        }
        o<<"\\right)";
      }
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\Theta\\left("<<arguments[1][0]->latex()<<"\\right)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::eq>::latex(ostream& o) const
  {
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        {
          o<<"\\delta\\left("<<arguments[0][0]->latex()<<"-";
          {
            ParenIf p(o, arguments[1][0]->BODMASlevel()>BODMASlevel());
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
    if ((arguments.size()>0 && !arguments[0].empty() && arguments[0][0]) ||
        (arguments.size()>1 && !arguments[1].empty() && arguments[1][0]))
      {
        OperationDAG<OperationType::lt> lt; lt.arguments=arguments;
        OperationDAG<OperationType::eq> eq; eq.arguments=arguments;
        lt.latex(o);
        o<<"+";
        return eq.latex(o);
      }
    else return o<<"1"<<endl;
  }


  template <>
  ostream& OperationDAG<OperationType::min>::latex(ostream& o) const
  {
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"min\\left("<<arguments[0][0]->latex()<<"," <<
          arguments[1][0]->latex()<<"\\right)";
      else
        o<<"min\\left("<<arguments[0][0]->latex()<<",0\\right)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"min\\left("<<arguments[1][0]->latex()<<",0\\right)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::max>::latex(ostream& o) const
  {
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"max\\left("<<arguments[0][0]->latex()<<"," <<
          arguments[1][0]->latex()<<"\\right)";
      else
        o<<"max\\left("<<arguments[0][0]->latex()<<",0\\right)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"max\\left("<<arguments[1][0]->latex()<<",0\\right)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::and_>::latex(ostream& o) const
  {
    if (arguments.size()>1 && !arguments[0].empty() && arguments[0][0] && 
        !arguments[1].empty() && arguments[1][0])
      o<<"\\Theta\\left("<<arguments[0][0]->latex()<<"-0.5\\right)\\Theta\\left(" <<
          arguments[1][0]->latex()<<"-0.5\\right)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::or_>::latex(ostream& o) const
  {
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"max\\left(\\Theta\\left("<<arguments[0][0]->latex()<<"-0.5\\right)," <<
          "\\Theta\\left("<<arguments[1][0]->latex()<<"\\right)\\right)";
      else
        o<<"\\Theta\\left("<<arguments[0][0]->latex()<<"-0.5\\right)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\Theta\\left("<<arguments[1][0]->latex()<<"-0.5\\right)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::not_>::latex(ostream& o) const
  {
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
      o<<"\\left(1-\\Theta(0.5-"<<arguments[0][0]->latex()<<"\\right)\right)";
    else
      o<<"1";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::time>::latex(ostream& o) const
  {
    return o<<" t ";
  }

  template <>
  ostream& OperationDAG<OperationType::copy>::latex(ostream& o) const
  {
    checkArg(0,0);
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::integrate>::latex(ostream& o) const
  {
    throw error("shouldn't be executed");
  }
        
  template <>
  ostream& OperationDAG<OperationType::differentiate>::latex(ostream& o) const
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

  SystemOfEquations::SystemOfEquations(const Minsky& m): minsky(m)
  {
    expressionCache.insertAnonymous(zero);
    expressionCache.insertAnonymous(one);
    zero->result=m.variableValues["constant:zero"];
    one->result=m.variableValues["constant:one"];

    // store stock & integral variables for later reordering
    map<string, VariableDAG*> integVarMap;

    vector<pair<VariableDAGPtr,Wire*>> integralInputs;
    
    // search through operations looking for integrals
    minsky.model->recursiveDo
      (&Group::items,
       [&](const Items&, Items::const_iterator it){
        if (IntOp* i=dynamic_cast<IntOp*>(it->get()))
          {
            if (VariablePtr iv=i->intVar)
              {
                // .get() OK here because object lifetime controlled by
                // expressionCache
                VariableDAG* v=integVarMap[iv->valueId()]=
                  dynamic_cast<VariableDAG*>(makeDAG(*iv).get());
                v->intOp=i;
                if (i->ports[1]->wires.size()>0)
                  {
                    // with integrals, we need to create a distinct variable to
                    // prevent infinite recursion of order() in the case of graph cycles
                    VariableDAGPtr input(new IntegralInputVariableDAG);
                    input->name=iv->name();
                    variables.push_back(input.get());
                    // manage object's lifetime with expressionCache
                    expressionCache.insertIntegralInput(iv->valueId(), input);
                    try
                      {input->rhs=getNodeFromWire(*(i->ports[1]->wires[0]));}
                    catch (...)
                      {
                        // try again later
                        integralInputs.emplace_back(input,i->ports[1]->wires[0]);
                      }
                  }
              }
          }
        return false;
      });

    // wire up integral inputs, now that all integrals are defined, do that derivative works. See #511
    for (auto& i: integralInputs)
      i.first->rhs=getNodeFromWire(*i.second);

    // process the Godley tables
    map<string, GodleyColumnDAG> godleyVars;
    m.model->recursiveDo
      (&Group::items,
       [&](const Items&, Items::const_iterator i)
       {
         if (auto g=dynamic_cast<GodleyIcon*>(i->get()))
           processGodleyTable(godleyVars, *g);
         return false;
       });

    for (auto& g: godleyVars)
      {
        //        assert(g->second.godleyId>=0);
        integVarMap[VariableValue::valueId(g.first)]=
          dynamic_cast<VariableDAG*>
          (makeDAG(VariableValue::valueId(g.first),
                   VariableValue::uqName(g.first), VariableValue::stock).get());
        VariableDAGPtr input(new IntegralInputVariableDAG);
        input->name=g.first;
        variables.push_back(input.get());
        // manage object's lifetime with expressionCache
        expressionCache.insertIntegralInput(g.first, input);
        input->rhs=expressionCache.insertAnonymous(NodePtr(new GodleyColumnDAG(g.second)));
      }

    for (auto& v: integVarMap)
      integrationVariables.push_back(v.second);

    // now start with the variables, and work our way back to how they
    // are defined
    for (VariableValues::value_type v: m.variableValues)
      if (v.second.isFlowVar())
        if (auto vv=dynamic_cast<VariableDAG*>
            (makeDAG(v.first, v.second.name, v.second.type()).get()))
          variables.push_back(vv);
          
    // sort variables into their order of definition
    sort(variables.begin(), variables.end(), 
         VariableDefOrder(expressionCache.size()));
  }

  NodePtr SystemOfEquations::makeDAG(const string& valueId, const string& name, VariableType::Type type)
  {
    if (expressionCache.exists(valueId))
      return expressionCache[valueId];

    assert(VariableValue::isValueId(valueId));
    assert(minsky.variableValues.count(valueId));
    VariableValue vv=minsky.variableValues[valueId];

    if (type==VariableType::constant)
      {
        NodePtr r(new ConstantDAG(vv.initValue(minsky.variableValues)));
        expressionCache.insert(valueId, r);
        return r;
      }

    // ensure name is unique
    string nm=name;
    for (unsigned i=0; varNames.count(nm); ++i)
      nm=name+"_"+to_string(i);
    varNames.insert(nm);
    
    shared_ptr<VariableDAG> r(new VariableDAG(valueId, nm, type));
    expressionCache.insert(valueId, r);
    r->init=vv.initValue(minsky.variableValues);
    if (vv.isFlowVar()) 
      {
        auto v=minsky.definingVar(valueId);
        if (v)
          r->rhs=getNodeFromWire(*v->ports[1]->wires[0]);
      }
    return r;
  }

  NodePtr SystemOfEquations::makeDAG(const OperationBase& op)
  {
    if (expressionCache.exists(op))
      return dynamic_pointer_cast<OperationDAGBase>(expressionCache[op]);

    if (op.type()==OperationType::differentiate)
      {
        assert(op.ports.size()==2);
        NodePtr expr;
        if (op.ports[1]->wires.size()==0 || !(expr=getNodeFromWire(*op.ports[1]->wires[0])))
          {
            minsky::minsky().displayErrorItem(op);          
            throw error("derivative not wired");
          }
        try
          {
            return expressionCache.insert
              (op, expr->derivative(*this));
          }
        catch (...)
          {
            minsky::minsky().displayErrorItem(op);          
            throw;
          }
      }
    else
      {
        shared_ptr<OperationDAGBase> r(OperationDAGBase::create(op.type()));
        expressionCache.insert(op, NodePtr(r));
        r->state=dynamic_pointer_cast<OperationBase>(minsky.model->findItem(op));
        assert(r->state);
        assert( r->state->type()!=OperationType::numOps);

        r->arguments.resize(op.numPorts()-1);
        for (size_t i=1; i<op.ports.size(); ++i)
          {
            auto& p=op.ports[i];
            for (auto w: p->wires)
              r->arguments[i-1].push_back(getNodeFromWire(*w));
          }
        return r;
      }
  }

  NodePtr SystemOfEquations::makeDAG(const SwitchIcon& sw)
  {
    // grab list of input wires
    vector<Wire*> wires;
    for (unsigned i=1; i<sw.ports.size(); ++i)
      {
        auto w=sw.ports[i]->wires;
        if (w.size()==0)
          {
            minsky.displayErrorItem(sw);
            throw error("input port not wired");
          }
        // shouldn't be more than 1 wire, ignore extraneous wires is present
        assert(w.size()==1);
        wires.push_back(w[0]);
      }

    assert(wires.size()==sw.numCases()+1);
    Expr input(expressionCache, getNodeFromWire(*wires[0]));

    auto r=make_shared<OperationDAG<OperationType::add>>();
    expressionCache.insert(sw, r);
    r->arguments[0].resize(sw.numCases());

    // implemented as a sum of step functions
    r->arguments[0][0]=getNodeFromWire(*wires[1])*(input<1);        
    for (unsigned i=1; i<sw.numCases()-1; ++i)
      r->arguments[0][i]=getNodeFromWire
        (*wires[i+1])*((input<i+1) - (input<i));
    r->arguments[0][sw.numCases()-1]=getNodeFromWire
      (*wires[sw.numCases()])*(1-(input<sw.numCases()-1));
    return r;
  };

  NodePtr SystemOfEquations::getNodeFromWire(const Wire& wire)
  {
    NodePtr r;
    if (auto p=wire.from())
      {
        auto& item=p->item;
        if (auto o=dynamic_cast<OperationBase*>(&item))
          {
            if (expressionCache.exists(*o))
              return expressionCache[*o];
            else
              // we're wired to an operation
              r=makeDAG(*o);
          }
        else if (auto s=dynamic_cast<SwitchIcon*>(&item))
          {
            if (expressionCache.exists(*s))
              return expressionCache[*s];
            else
              r=makeDAG(*s);
          }
        else if (auto v=dynamic_cast<VariableBase*>(&item))
          {
            if (expressionCache.exists(*v))
              return expressionCache[*v];
            else
              if (v && v->type()!=VariableBase::undefined) 
                // we're wired to a variable
                r=makeDAG(*v);
          }
      }
    return r;
  }     
  
    
  ostream& SystemOfEquations::latex(ostream& o) const
  {
    o << "\\begin{eqnarray*}\n";
    for (const VariableDAG* i: variables)
      {
        if (dynamic_cast<const IntegralInputVariableDAG*>(i) ||
            i->type==VariableType::constant) continue;
        o << i->latex() << "&=&";
        if (i->rhs) 
          i->rhs->latex(o);
        else
          o<<i->init;
        o << "\\\\\n";
      }

    for (const VariableDAG* i: integrationVariables)
      {
        o << mathrm(i->name)<<"(0)&=&"<<MathDAG::latex(i->init)<<"\\\\\n";
        o << "\\frac{ d " << mathrm(i->name) << 
          "}{dt} &=&";
        VariableDAGPtr input=expressionCache.getIntegralInput(i->valueId);
        if (input && input->rhs)
          input->rhs->latex(o);
        o << "\\\\\n";
      }
    return o << "\\end{eqnarray*}\n";
  }

  ostream& SystemOfEquations::latexWrapped(ostream& o) const
  {
    o << "\\begin{dgroup*}\n";
    for (const VariableDAG* i: variables)
      {
        if (dynamic_cast<const IntegralInputVariableDAG*>(i)) continue;
        if (i->type==VariableType::constant) continue;
        o<<"\\begin{dmath*}\n";
        o << i->latex() << "=";
        if (i->rhs) 
          i->rhs->latex(o);
        else
          o<<i->init;
        o << "\n\\end{dmath*}\n";
      }

    for (const VariableDAG* i: integrationVariables)
      {
        o<<"\\begin{dmath*}\n";
        o << mathrm(i->name)<<"(0)="<<MathDAG::latex(i->init)<<"\n\\end{dmath*}\n";
        o << "\\begin{dmath*}\n\\frac{ d " << mathrm(i->name) << 
          "}{dt} =";
        VariableDAGPtr input=expressionCache.getIntegralInput(i->valueId);
        if (input && input->rhs)
          input->rhs->latex(o);
        o << "\\end{dmath*}\n";
      }
    return o << "\\end{dgroup*}\n";
  }

  ostream& SystemOfEquations::matlab(ostream& o) const
  {
    o<<"function f=f(x,t)\n";
    // define names for the components of x for reference
    int j=1;
    for (const VariableDAG*  i: integrationVariables)
      o<<i->matlab()<<"=x("<<j++<<");\n";

    for (const VariableDAG* i: variables)
      {
        if (dynamic_cast<const IntegralInputVariableDAG*>(i) ||
            i->type==VariableType::constant) continue;
        o << i->matlab() << "=";
        if (i->rhs)
          o << i->rhs->matlab();
        else
          o << i->init;
        o<<";\n";
      }

    j=1;
    for (const VariableDAG* i: integrationVariables)
      {
        o << "f("<<j++<<")=";
        VariableDAGPtr input=expressionCache.getIntegralInput(i->valueId);
        if (input && input->rhs)
          input->rhs->matlab(o);
        else
          o<<0;
        o<<";\n";
      }
    o<<"endfunction;\n\n";

    // now write out the initial conditions
    j=1;
    for (const VariableDAG* i: integrationVariables)
      o << "x0("<<j++<<")="<<i->init<<";\n";
   
    return o;
  }

  void SystemOfEquations::populateEvalOpVector
  (EvalOpVector& equations, vector<Integral>& integrals)
  {
    equations.clear();
    integrals.clear();

    for (const VariableDAG* i: variables)
      {
        i->addEvalOps(equations);
        assert(minsky.variableValues.validEntries());
      }

    for (const VariableDAG* i: integrationVariables)
      {
        string vid=i->valueId;
        integrals.push_back(Integral());
        assert(VariableValue::isValueId(vid));
        assert(minsky.variableValues.count(vid));
        integrals.back().stock=minsky.variableValues[vid];
        integrals.back().operation=dynamic_cast<IntOp*>(i->intOp);
        VariableDAGPtr iInput=expressionCache.getIntegralInput(vid);
        if (iInput && iInput->rhs)
          integrals.back().input=iInput->rhs->addEvalOps(equations);
      }
    assert(minsky.variableValues.validEntries());

    // ensure all variables have their output port's variable value up to date
    minsky.model->recursiveDo
      (&Group::items,
       [&](Items&, Items::iterator i)
       {
         if (auto v=dynamic_cast<VariableBase*>(i->get()))
           {
             assert(minsky.variableValues.count(v->valueId()));
             v->ports[0]->setVariableValue(minsky.variableValues[v->valueId()]);
           }
         else if (auto pw=dynamic_cast<PlotWidget*>(i->get()))
           for (auto& port: pw->ports) 
             for (auto w: port->wires)
               // ensure plot inputs are evaluated
               w->from()->setVariableValue(getNodeFromWire(*w)->addEvalOps(equations));
         return false;
       });
  }

  void SystemOfEquations::processGodleyTable
  (map<string, GodleyColumnDAG>& godleyVariables, const GodleyIcon& gi)
  {
    auto& godley=gi.table;
    for (size_t c=1; c<godley.cols(); ++c)
      {
        string colName=stripActive(trimWS(godley.cell(0,c)));
        if (colName=="_" || VariableValue::uqName(colName).empty())
          throw error("unnamed Godley table column found");
        // resolve scope
        colName=VariableValue::valueId(gi.group.lock(), colName);
        if (processedColumns.count(colName)) continue; //skip shared columns
        processedColumns.insert(colName);
        GodleyColumnDAG& gd=godleyVariables[colName];
        gd.arguments.resize(2);
        vector<WeakNodePtr>& arguments=gd.arguments[0];
        for (size_t r=1; r<godley.rows(); ++r)
          {
            if (godley.initialConditionRow(r)) continue;
            FlowCoef fc(godley.cell(r,c));
            if (fc.name.empty()) continue;
            if (godley.signConventionReversed(c)) fc.coef*=-1;

            VariablePtr v(VariableType::flow, fc.name);
            v->group=gi.group;

            if (abs(fc.coef)==1)
              gd.arguments[fc.coef<0? 1: 0].push_back(WeakNodePtr(makeDAG(*v)));
            else
              {
                OperationDAG<OperationType::multiply>* term=
                  new OperationDAG<OperationType::multiply>;
                gd.arguments[fc.coef<0? 1: 0].push_back
                  (expressionCache.insertAnonymous(NodePtr(term)));

                term->arguments.resize(2);
                term->arguments[0].push_back
                  (expressionCache.insertAnonymous(NodePtr(new ConstantDAG(abs(fc.coef)))));
                term->arguments[1].push_back(WeakNodePtr(makeDAG(*v)));
              }
          }
      }
  }
}


