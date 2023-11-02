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
#include "minsky.h"
#include "variableValues.h"
#include "flowCoef.h"
#include "str.h"
#include "valueId.h"

#include "constMap.rcd"
#include "dimension.rcd"
#include "hypercube.rcd"
#include "hypercube.xcd"
#include "index.rcd"
#include "index.xcd"
#include "nobble.h"
#include "tensorInterface.rcd"
#include "tensorInterface.xcd"
#include "tensorVal.rcd"
#include "tensorVal.xcd"
#include "units.rcd"
#include "userFunction.h"
#include "variableValue.rcd"
#include "variableValues.rcd"
#include "variableValues.xcd"
#include "xvector.rcd"
#include "minsky_epilogue.h"

#include <iomanip>
#include <error.h>

using namespace ecolab;
using namespace std;

#ifdef WIN32
// std::quoted not supported (yet) on MXE
string quoted(const string& x)
{
  string r="\"";
  for (auto& i: x)
    if (i=='"')
      r+=R"(\")";
    else
      r+=i;
  return r+"\"";
}
#endif

namespace minsky
{
  std::vector<double> ValueVector::stockVars(1);
  std::vector<double> ValueVector::flowVars(1);

  bool VariableValue::idxInRange() const
  {return m_type==undefined || idx()+size()<=
      (isFlowVar()?ValueVector::flowVars.size(): ValueVector::stockVars.size());}
    

  
  double& VariableValue::operator[](size_t i)
  {
    assert(i<size() && idxInRange());
    return *(&valRef()+i);
  }

  VariableValue& VariableValue::operator=(minsky::TensorVal const& x)
  {
    index(x.index());
    hypercube(x.hypercube());
    assert(idxInRange());
    memcpy(&valRef(), x.begin(), x.size()*sizeof(x[0]));
    return *this;
  }

  VariableValue& VariableValue::operator=(const ITensor& x)
  {
    index(x.index());
    hypercube(x.hypercube());
    assert(idxInRange());
    for (size_t i=0; i<x.size(); ++i)
      (*this)[i]=x[i];
    return *this;
  }
 
  VariableValue& VariableValue::allocValue()                                        
  {    
    switch (m_type)
      {
      case undefined:
        m_idx=-1;
        break;
      case flow:
      case tempFlow:
      case constant:
      case parameter:
        m_idx=ValueVector::flowVars.size();
        ValueVector::flowVars.resize(ValueVector::flowVars.size()+size());
        //cout<<"allocating "<<name<<" @ "<<m_idx<<endl;
        break;
      case stock:
      case integral:
        m_idx=ValueVector::stockVars.size();
        ValueVector::stockVars.resize(ValueVector::stockVars.size()+size());
        break;
      default: break;
      }
    return *this;
  }

  const double& VariableValue::valRef() const                                     
  {
    switch (m_type)
      {
      case flow:
      case tempFlow:
      case constant:
      case parameter:
        //        assert(idxInRange());
         if (size_t(m_idx)<ValueVector::flowVars.size())
           return ValueVector::flowVars[m_idx];
         break;
      case stock:
      case integral:
        //        assert(idxInRange());
        if (size_t(m_idx)<ValueVector::stockVars.size())
          return ValueVector::stockVars[m_idx];
        break;
      default: break;
      }
    static double zero=0;
    return zero;
  }
  
  double& VariableValue::valRef()                                         
  {
    if (m_idx==-1)
      allocValue();
    switch (m_type)
      {
      case flow:
      case tempFlow:
      case constant:
      case parameter:
        assert(idxInRange());
        if (size_t(m_idx+size())<=ValueVector::flowVars.size())
          return ValueVector::flowVars[m_idx]; 
      case stock:
      case integral: 
        assert(idxInRange());
        if (size_t(m_idx+size())<=ValueVector::stockVars.size())
          return ValueVector::stockVars[m_idx]; 
        break;
      default: break;
      }
    throw error("invalid access of variable value reference: %s",name.c_str());
  }
  
  TensorVal VariableValues::initValue
  (const VariableValue& v, set<string>& visited) const
  {
    if (v.tensorInit.rank()>0)
      return v.tensorInit;
    
    FlowCoef fc(v.init);
    if (trimWS(fc.name).empty())
      return fc.coef;

    // special generator functions handled here
    auto p=fc.name.find('(');
    if (p!=string::npos)
      {
        string fn=fc.name.substr(0,p);
        // unpack args
        const char* x=fc.name.c_str()+p+1;
        char* e;
        vector<unsigned> dims;
        for (;;)
          {
            auto tmp=strtol(x,&e,10);
            if (tmp>0 && e>x && *e)
              {
                x=e+1;
                dims.push_back(tmp);
              }
            else
              break;
          }
        TensorVal r(dims);
        r.allocVal();

        if (fn=="iota")
          for (size_t i=0; i<r.size(); ++i)
            r[i]=i;
        else if (fn=="one")
          for (size_t i=0; i<r.size(); ++i)
            r[i]=1;
        else if (fn=="zero" || fn=="eye")
          {
            for (size_t i=0; i<r.size(); ++i)
              r[i]=0;
            if (fn=="eye")
              {
                // diagonal elements set to 1
                // find minimum dimension, and stride of diagonal elements
                size_t mind=r.size(), stride=1;
                for (auto i: dims)
                  mind=min(mind, size_t(i));
                for (size_t i=0; i<dims.size()-1; ++i)
                  stride*=(dims[i]+1);
                for (size_t i=0; i<mind; ++i)
                  r[stride*i]=1;
              }
          }
        else if (fn=="rand")
          {
            for (size_t i=0; i<r.size(); ++i)
              r[i]=double(rand())/RAND_MAX;
          }
        return r;
      }
        
    // resolve name
    auto valueId=minsky::valueId(v.m_scope.lock(), fc.name);
    if (visited.count(valueId))
      throw error("circular definition of initial value for %s",
                  fc.name.c_str());
    VariableValues::const_iterator vv=find(valueId);
    if (vv==end())
      throw error("Unknown variable %s in initialisation of %s",fc.name.c_str(), v.name.c_str());

    visited.insert(valueId);
    return fc.coef*initValue(*vv->second, visited);
  }

  void VariableValues::resetValue(VariableValue& v) const
  {
    if (v.idx()<0) v.allocValue();
      // initialise variable only if its variable is not defined or it is a stock
      if (!v.isFlowVar() || !cminsky().definingVar(v.valueId()))
        {
          if (v.tensorInit.size())
            {
              // ensure dimensions are correct
              auto hc=v.tensorInit.hypercube();
              for (auto& xv: hc.xvectors)
                {
                  auto dim=cminsky().dimensions.find(xv.name);
                  if (dim!=cminsky().dimensions.end())
                    xv.dimension=dim->second;
                }
              v.tensorInit.hypercube(hc);
            }
          if (v.tensorInit.rank()>0)
            v=v.tensorInit;
          else
            v=initValue(v);
        }
      assert(v.idxInRange());
  }


  string VariableValues::newName(const string& name) const
  {
    int i=1;
    string trialName;
    do
      trialName=utf_to_utf<char>(name)+to_string(i++);
    while (count(valueId(trialName)));
    return trialName;
  }

  void VariableValues::reset()
  {
    // reallocate all variables
    ValueVector::stockVars.clear();
    ValueVector::flowVars.clear();
    for (auto& v: *this) {
      v.second->reset_idx();  // Set idx of all flowvars and stockvars to -1 on reset. For ticket 1049		
      resetValue(v.second->allocValue());
      assert(v.second->idxInRange());
    }
}

  bool VariableValues::validEntries() const
  {
    return all_of(begin(), end(), [](const value_type& v){return isValueId(v.first);});
  }


  EngNotation engExp(double v) 
  {
    EngNotation r;
    r.sciExp=(v!=0)? floor(log(fabs(v))/log(10)): 0;
    if (r.sciExp==3) // special case for dates
      r.engExp=0;
    else
      r.engExp=r.sciExp>=0? 3*(r.sciExp/3): 3*((r.sciExp+1)/3-1);
    return r;
  }

  string mantissa(double value, const EngNotation& e, int digits)
  {
    int width, decimal_places;
    digits=std::max(digits, 3);
    switch (e.sciExp-e.engExp)
      {
      case -3: width=digits+4; decimal_places=digits+1; break;
      case -2: width=digits+3; decimal_places=digits; break;
      case 0: case -1: width=digits+2; decimal_places=digits-1; break;
      case 1: width=digits+2; decimal_places=digits-2; break;
      case 2: case 3: width=digits+2; decimal_places=digits-3; break;
      default: return ""; // shouldn't be here...
      }
    char val[80];
    const char conv[]="%*.*f";
    snprintf(val,sizeof(val),conv,width,decimal_places,value*pow(10,-e.engExp));
    return val;
  }
  
  string expMultiplier(int exp)
  {return exp!=0? "×10<sup>"+std::to_string(exp)+"</sup>": "";}


  void VariableValue::exportAsCSV(const string& filename, const string& comment) const
  {
    ofstream of(filename);
    if (!comment.empty())
      of<<R"(""")"<<comment<<R"(""")"<<endl;
               
    const auto& xv=hypercube().xvectors;
    ostringstream os;
    for (const auto& i: xv)
      {
        if (&i>xv.data()) os<<",";
        os<<json(static_cast<const NamedDimension&>(i));
      }
    of<<quoted("RavelHypercube=["+os.str()+"]")<<endl;
    for (const auto& i: hypercube().xvectors)
      of<<"\""<<i.name<<"\",";
    of<<"value$\n";

    auto idxv=index();
    size_t i=0;
    for (auto d=begin(); d!=end(); ++i, ++d)
      if (isfinite(*d))
        {
          ssize_t idx=idxv.empty()? i: idxv[i];
          for (size_t j=0; j<rank(); ++j)
            {
              auto div=std::div(idx, ssize_t(hypercube().xvectors[j].size()));
              of << "\""<<str(hypercube().xvectors[j][div.rem], hypercube().xvectors[j].dimension.units) << "\",";
              idx=div.quot;
            }
          of << *d << endl;
        }
  }

  Summary VariableValue::summary() const
  {
    MathDAG::SystemOfEquations system(cminsky());
    MathDAG::VariableDAGPtr varNode;
    switch (type())
      {
      case integral:
      case stock:
        varNode=system.getNodeFromIntVar(valueId());
        break;
      default:
        varNode=system.getNodeFromValueId(valueId());
        break;
      }

    string scopeName=":";
    if (auto scope=m_scope.lock())
      if (scope!=cminsky().model)
        scopeName=scope->title.empty()? scope->id(): scope->title;

    string godleyName;
    string definition=varNode && varNode->rhs? varNode->rhs->latexStr(): "";
    string udfDefinition=varNode && varNode->rhs? varNode->rhs->matlabStr():"";
    if (auto var=cminsky().definingVar(valueId()))
      {
        if (auto controller=dynamic_pointer_cast<GodleyIcon>(var->controller.lock()))
          godleyName=controller->table.title.empty()? controller->id(): controller->table.title;
        if (auto p=var->ports(1).lock())
          if (!p->wires().empty())
            if (auto udf=dynamic_cast<UserFunction*>(&p->wires().front()->from()->item()))
              {
                definition="\\text{"+udf->expression+"}";
                udfDefinition=udf->expression;
              }
      }

    
    return Summary{
      valueId(),
      name,
      type(),
      definition,
      udfDefinition,
      init,
      value(),
      scopeName,
      godleyName,
      hypercube().dims(),
      units.latexStr()
    };
    
  }

}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Units);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::ValueVector);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariableValue);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariableValues);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariableValuePtr);
