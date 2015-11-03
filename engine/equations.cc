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

    struct NoArgument
    {
      OperationPtr state;
      unsigned argNum1, argNum2;
      NoArgument(const OperationPtr& s, unsigned a1, unsigned a2): 
        state(s), argNum1(a1), argNum2(a2) {}
      const char* what() const {
        if (state)
          minsky::minsky().displayErrorItem(state->x(),state->y());
        return ("missing argument "+str(argNum1)+","+str(argNum2)+
                " on operation "+(state? state->name():string(""))).c_str();
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
  (EvalOpVector& ev, std::map<int,VariableValue>& opValMap, const VariableValue& r) const 
  {
    if (result.idx()<0)
      {
        if (r.idx()>=0 && r.isFlowVar())
          result=r;
        else
          result.allocValue();
        result=value;
        // set the initial value of the actual variableValue (if it exists)
        VariableValues& values=minsky::minsky().variables.values;
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
    if (scope<0)
      return o<<mathrm(name);
    else
      return o<<"{"<<mathrm(name)<<"}_{"<<scope<<"}";
  }

  ostream& VariableDAG::matlab(ostream& o) const
  {
    if (type==constant)
      return o<<init;
    if (scope<0)
      return o<<validMatlabIdentifier(name);
    else
      // TODO: could potentially clash with another identifier
      return o<<validMatlabIdentifier(name)<<"__"<<scope;
  }

  VariableValue VariableDAG::addEvalOps
  (EvalOpVector& ev, std::map<int,VariableValue>& portValMap, const VariableValue& r) const
  {
    if (result.idx()<0)
      {
        result=minsky::minsky().variables.getVariableValue(valueId);
        if (result.type()==VariableType::undefined) 
          {
            result=VariableValue(VariableType::tempFlow);
            result.allocValue();
          }
        if (rhs)
          rhs->addEvalOps(ev,portValMap,result);
      }
    if (r.isFlowVar() && r.idx()>=0 && (r.idx()!=result.idx() || !result.isFlowVar()))
      ev.push_back(EvalOpPtr(OperationType::copy, r, result));
    return result;
  }

  VariableValue IntegralInputVariableDAG::addEvalOps
  (EvalOpVector& ev, std::map<int,VariableValue>& portValMap, const VariableValue& r) const
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
          rhs->addEvalOps(ev,portValMap,result);
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
  (EvalOpVector& ev, std::map<int,VariableValue>& portValMap, const VariableValue& r) const
  {
    if (result.idx()<0)
      {
        if (IntOp* i=dynamic_cast<IntOp*>(state.get()))
          {
            if (VariablePtr iv=i->getIntVar())
              {
                result=minsky::minsky().variables.getVariableValue(iv->valueId());
                // integral copies need to be done now, in case of cycles
                if (r.isFlowVar() && r.idx()>=0)
                  ev.push_back(EvalOpPtr(OperationType::copy, r, result));
                if (state && state->numPorts()>0)
                  portValMap[state->ports()[0]]=result;
                return result; // integration handled as part of RK arlgorithm
              }
            else
              throw error("no integration variable for %s",i->description().c_str());

          }
        else if (r.isFlowVar() && r.idx()>=0)
          result=r;
        else 
          result.allocValue();

        if (state && state->numPorts()>0)
          portValMap[state->ports()[0]]=result;


        // prepare argument expressions
        vector<vector<VariableValue> > argIdx(arguments.size());
        for (size_t i=0; type()!=integrate && i<arguments.size(); ++i)
          for (size_t j=0; j<arguments[i].size(); ++j)
            if (arguments[i][j])
              argIdx[i].push_back(arguments[i][j]->addEvalOps(ev, portValMap));
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
          case constant:
            ev.push_back(EvalOpPtr(type(), result));
            dynamic_cast<ConstantEvalOp&>(*ev.back()).value=
              dynamic_cast<Constant&>(*state).value;
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
    if (arguments.empty()) return o;
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
    if (arguments.empty()) return o;
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
    if (arguments.empty()) return o;
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
    if (arguments.empty())
      return o<<"1";
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
      if (arguments.size()>1 && !arguments[0].empty() && arguments[0][0])
        o<<"min(0,"<<arguments[0][0]->matlab()<<")";
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
        o<<"max(0,"<<arguments[0][0]->matlab()<<")";
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
    if (IntOp* i=dynamic_cast<IntOp*>(state.get()))
      o << validMatlabIdentifier(i->description());
    return o;
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
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\Theta\\left("<<arguments[1][0]->latex()<<"-" <<
          arguments[0][0]->latex()<<"\\right)";
      else
        o<<"\\Theta\\left(-"<<arguments[0][0]->latex()<<"\\right)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\Theta\\left("<<arguments[1][0]->latex()<<"\\right)";
      else
        o<<"0";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::le>::latex(ostream& o) const
  {
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\Theta\\left("<<arguments[1][0]->latex()<<"-" <<
          arguments[0][0]->latex()<<"\\right)+\\delta\\left("<<
          arguments[1][0]->latex()<<"-" <<
          arguments[0][0]->latex()<<"\\right)";
      else
        o<<"\\Theta\\left(-"<<arguments[0][0]->latex()<<"\\right)+\\delta\\left("<<
          arguments[0][0]->latex()<<"\\right)";
    else
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\Theta\\left("<<arguments[1][0]->latex()<<"\\right)+\\delta\\left("<<
          arguments[1][0]->latex()<<"\\right)";
      else
        o<<"1";
    return o;
  }

  template <>
  ostream& OperationDAG<OperationType::eq>::latex(ostream& o) const
  {
    if (arguments.size()>0 && !arguments[0].empty() && arguments[0][0])
      if (arguments.size()>1 && !arguments[1].empty() && arguments[1][0])
        o<<"\\delta\\left("<<arguments[0][0]->latex()<<"-" <<
          arguments[1][0]->latex()<<"\\right)";
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
    if (IntOp* i=dynamic_cast<IntOp*>(state.get()))
      o << mathrm(i->description());
    return o;
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

  SystemOfEquations::SystemOfEquations(const Minsky& m): minsky(m)
  {
    expressionCache.insertAnonymous(zero);
    expressionCache.insertAnonymous(one);
    zero->result=m.variables.values["constant:zero"];
    one->result=m.variables.values["constant:one"];

    // firstly, we need to create a map of ports belonging to operations
    for (const OperationPtr& o: minsky.operations)
      for (size_t i=0; i<o->numPorts(); ++i)
        portToOperation[o->ports()[i]]=o.id();

    // then a map of ports belonging to switches
    for (auto& s: minsky.switchItems)
      for (int p: s->ports())
        portToSwitch[p]=s.id();

    // store stock & integral variables for later reordering
    map<string, VariableDAG*> integVarMap;

    // search through operations looking for integrals
    for (const OperationPtr& o: minsky.operations)
      if (IntOp* i=dynamic_cast<IntOp*>(o.get()))
        {
          if (VariablePtr iv=i->getIntVar())
            {
              // .get() OK here because object lifetime controlled by
              // expressionCache
              VariableDAG* v=integVarMap[iv->valueId()]=
                dynamic_cast<VariableDAG*>(makeDAG
                                           (iv->valueId(), iv->scope(), iv->name(), iv->type()).get());
              v->intOp=o;
              if (minsky.wiresAttachedToPort(i->ports()[1]).size()>0)
                {
                  // with integrals, we need to create a distinct variable to
                  // prevent infinite recursion of order() in the case of graph cycles
                  VariableDAGPtr input(new IntegralInputVariableDAG);
                  input->name=iv->name();
                  variables.push_back(input.get());
                  // manage object's lifetime with expressionCache
                  expressionCache.insertIntegralInput(iv->valueId(), input);
                  input->rhs=getNodeFromWire(minsky.wiresAttachedToPort(i->ports()[1])[0]);
                }
            }
        }
      else if (const Constant* c=dynamic_cast<const Constant*>(o.get()))
        {
          VariablePtr v(VariableType::parameter, c->description);
          // note makeDAG caches a reference to the object, and manages lifetime
          variables.push_back(makeDAG(*v).get());
          variables.back()->rhs=expressionCache.insertAnonymous(NodePtr(new ConstantDAG(c->value)));
        }

    // process the Godley tables
    map<string, GodleyColumnDAG> godleyVars;
    for (Minsky::GodleyItems::const_iterator g=m.godleyItems.begin(); 
         g!=m.godleyItems.end(); ++g)
      processGodleyTable(godleyVars, g->table, g->id());

    for (map<string, GodleyColumnDAG>::iterator g=godleyVars.begin(); 
         g!=godleyVars.end(); ++g)
      {
        assert(g->second.godleyId>=0);
        VariableDAG* v=integVarMap[VariableManager::valueId(g->first)]=
          makeDAG(VariableManager::valueId(g->first), VariableManager::scope(g->first), 
                  VariableManager::uqName(g->first), VariableType::stock).get();
        VariableDAGPtr input(new IntegralInputVariableDAG);
        input->name=g->first;
        variables.push_back(input.get());
        // manage object's lifetime with expressionCache
        expressionCache.insertIntegralInput(g->first, input);
        input->rhs=expressionCache.insertAnonymous(NodePtr(new GodleyColumnDAG(g->second)));
      }

    // reorder integration variables according to sVars
    const vector<string>& sVars=minsky.variables.stockVars();
    for (vector<string>::const_iterator v=sVars.begin(); 
         v!=sVars.end(); ++v)
      {
        string vid=VariableManager::valueId(*v);
        if (!integVarMap.count(vid))
          throw error("no definition provided for stock variable %s",vid.c_str());
        integrationVariables.push_back(integVarMap[vid]);
      }

    // now start with the variables, and work our way back to how they
    // are defined
    for (VariableValues::value_type v: m.variables.values)
      if (v.second.isFlowVar())
        variables.push_back
          (makeDAG(v.first, 
                   v.second.type()==VariableType::constant? -1: VariableManager::scope(v.second.name),
                   VariableManager::uqName(v.second.name), v.second.type()).get());
          
    // sort variables into their order of definition
    sort(variables.begin(), variables.end(), 
         VariableDefOrder(expressionCache.size()));
    assert(integrationVariables.size()==m.variables.stockVars().size());
  }

  shared_ptr<VariableDAG> SystemOfEquations::makeDAG(const string& valueId, int scope, const string& name, VariableType::Type type)
  {
    if (expressionCache.exists(valueId))
      return dynamic_pointer_cast<VariableDAG>(expressionCache[valueId]);

    shared_ptr<VariableDAG> r(new VariableDAG(valueId, scope, VariableManager::uqName(name), type));
    expressionCache.insert(valueId, r);
    VariableValue vv=minsky.variables.getVariableValue(valueId);
    r->init=vv.initValue(minsky.variables.values);
    if (vv.isFlowVar()) 
      {
        r->rhs=getNodeFromWire(minsky.variables.wireToVariable(valueId));
//        if (!r->rhs) // add initial condition
//          {
//            anonymousNodes.push_back(NodePtr(new ConstantDAG(r->init)));
//            r->rhs=anonymousNodes.back();
//          }
      }
    return r;
  }

  NodePtr SystemOfEquations::makeDAG(const OperationPtr& op)
  {
    if (expressionCache.exists(*op))
      return dynamic_pointer_cast<OperationDAGBase>(expressionCache[*op]);

    if (op->type()==OperationType::differentiate)
      {
        assert(op->numPorts()==2);
        ecolab::array<int> wires=minsky.wiresAttachedToPort(op->ports()[1]);
        assert(wires.size()<=1);
        if (wires.size()==0)
          {
            minsky.displayErrorItem(*op);          
            throw error("derivative not wired");
          }

        return expressionCache.insert
          (*op, getNodeFromWire(wires[0])->derivative(*this));
      }
    else
      {
        shared_ptr<OperationDAGBase> r(OperationDAGBase::create(op->type()));
        expressionCache.insert(*op, NodePtr(r));
        r->state=op;
        assert(r->state);
        assert( r->state->type()!=OperationType::numOps);
        if (const Constant* c=dynamic_cast<const Constant*>(op.get()))
          {
            r->name=c->description;
            r->init=c->value;
          }
        else if (const IntOp* i=dynamic_cast<const IntOp*>(op.get()))
          r->name=i->description();

        r->arguments.resize(op->numPorts()-1);
        for (size_t i=1; i<op->numPorts(); ++i)
          {
            PortManager::Ports::const_iterator p=minsky.ports.find(op->ports()[i]);
            if (p != minsky.ports.end() && p->input())
              {
                ecolab::array<int> wires=minsky.wiresAttachedToPort(op->ports()[i]);
                for (size_t w=0; w<wires.size(); ++w)
                  r->arguments[i-1].push_back(getNodeFromWire(wires[w]));
              }
          }
        return r;
      }
  }

  NodePtr SystemOfEquations::makeDAG(const SwitchIcon& sw)
  {
    // grab list of input wires
    vector<int> wires;
    for (unsigned i=1; i<sw.ports().size(); ++i)
      {
        auto w=minsky.wiresAttachedToPort(sw.ports()[i]);
        if (w.size()==0)
          {
            minsky.displayErrorItem(sw);
            throw error("input port not wired");
          }
        // shouldn't be more than 1 wire, ignore exraneous wires is present
        assert(w.size()==1);
        wires.push_back(w[0]);
      }

    assert(wires.size()==sw.numCases()+1);
    Expr input(expressionCache, getNodeFromWire(wires[0]));

    auto r=make_shared<OperationDAG<OperationType::add>>();
    expressionCache.insert(sw, r);
    r->arguments[0].resize(sw.numCases());

    // implemented as a sum of step functions
    r->arguments[0][0]=getNodeFromWire(wires[1])*(input<1);        
    for (unsigned i=1; i<sw.numCases()-1; ++i)
      r->arguments[0][i]=getNodeFromWire
        (wires[i+1])*((input<i+1) - (input<i));
    r->arguments[0][sw.numCases()-1]=getNodeFromWire
      (wires[sw.numCases()])*(1-(input<sw.numCases()-1));
    return r;
  };

  NodePtr SystemOfEquations::getNodeFromWire(int wire)
  {
    auto wi=minsky.wires.find(wire);
    NodePtr r;
    if (wi != minsky.wires.end())
      {
        const Wire& w=*wi;
        if (portToOperation.count(w.from))
          {
            const OperationPtr& o=
              *minsky.operations.find(portToOperation[w.from]);
            if (expressionCache.exists(*o))
              return expressionCache[*o];
            else
              // we're wired to an operation
              r=makeDAG(o);
          }
        else if (portToSwitch.count(w.from))
          {
            const SwitchIcon& s=*minsky.switchItems[portToSwitch[w.from]];
            if (expressionCache.exists(s))
              return expressionCache[s];
            else
              r=makeDAG(s);
          }
        else
          {
            VariablePtr v(minsky.variables.getVariableFromPort(w.from));
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
        VariableDAGPtr input=expressionCache.getIntegralInput
          (VariableManager::valueId(i->scope,i->name));
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
        VariableDAGPtr input=expressionCache.getIntegralInput
          (VariableManager::valueId(i->scope,i->name));
        if (input && input->rhs)
          input->rhs->latex(o);
        o << "\\end{dmath*}\n";
      }
    return o << "\\end{dgroup*}\n";
  }

  ostream& SystemOfEquations::matlab(ostream& o) const
  {
    assert(integrationVariables.size()==minsky.variables.stockVars().size());
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
        VariableDAGPtr input=expressionCache.getIntegralInput
          (VariableManager::valueId(i->scope,i->name));
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
  (EvalOpVector& equations, vector<Integral>& integrals, 
     std::map<int,VariableValue>& portValMap)
  {
    assert(integrationVariables.size()==minsky.variables.stockVars().size());
    equations.clear();
    integrals.clear();
    portValMap.clear();

    for (const VariableDAG* i: variables)
      i->addEvalOps(equations, portValMap);

    for (const VariableDAG* i: integrationVariables)
      {
        string vid=VariableManager::valueId(i->scope,i->name);
        integrals.push_back(Integral());
        integrals.back().stock=
          minsky.variables.getVariableValue(vid);
        integrals.back().operation=dynamic_cast<IntOp*>(i->intOp.get());
        VariableDAGPtr iInput=expressionCache.getIntegralInput(vid);
        if (iInput && iInput->rhs)
          integrals.back().input=iInput->rhs->addEvalOps(equations, portValMap);
      }

    // loop over plots and ensure that each connected input port
    // has its expression evaluated 
    for (const PlotWidget& p: minsky.plots)
      for (int port: p.ports())
        {
          auto wires=minsky.wiresAttachedToPort(port);
          for (int w: wires)
            {
              NodePtr n=getNodeFromWire(w);
              n->addEvalOps(equations, portValMap);
              // ensure portValMap is updated with results of
              // anonymous operations (eg of differentiate)
              auto wi=minsky.wires.find(w);
              if (wi != minsky.wires.end())
                portValMap[wi->from]=n->result;
            }
        }

    // load up variable output port associations
    for (const VariablePtr& v: minsky.variables)
      if (minsky.wiresAttachedToPort(v->outPort()).size()>0)
        portValMap[v->outPort()]=
          minsky.variables.getVariableValueFromPort(v->outPort());
  }

  void SystemOfEquations::processGodleyTable
  (map<string, GodleyColumnDAG>& godleyVariables, const GodleyTable& godley, int godleyId)
  {
    for (size_t c=1; c<godley.cols(); ++c)
      {
        string colName=stripActive(trimWS(godley.cell(0,c)));
        if (VariableManager::uqName(colName).empty())
          throw error("unnamed Godley table column found");
        // if local, append scope
        if (colName.find(':')==string::npos)
          colName=VariableManager::valueId(-1, colName);
        if (processedColumns.count(colName)) continue; //skip shared columns
        processedColumns.insert(colName);
        GodleyColumnDAG& gd=godleyVariables[colName];
        gd.godleyId=godleyId;
        gd.arguments.resize(2);
        vector<WeakNodePtr>& arguments=gd.arguments[0];
        for (size_t r=1; r<godley.rows(); ++r)
          {
            if (godley.initialConditionRow(r)) continue;
            FlowCoef fc(godley.cell(r,c));
            if (fc.name.empty()) continue;
            if (godley.signConventionReversed(c)) fc.coef*=-1;

            VariablePtr v(VariableType::flow, fc.name);
            // if local variable, set scope
            if (fc.name.find(':')==string::npos)
              v->setScope(-1);
  
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


