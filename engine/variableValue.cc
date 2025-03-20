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
#include "slider.rcd"
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
  std::vector<double,CIVITA_ALLOCATOR<double>> ValueVector::stockVars(1);
  std::vector<double,CIVITA_ALLOCATOR<double>> ValueVector::flowVars(1);

  namespace {
    // special scalar constants
    struct SpecialConst: public VariableValue
    {
      using ITensorVal::operator=;
      SpecialConst(const string& name, const string& init):
        VariableValue(VariableType::constant,name) {m_init=init;}
    };
  }
  
  VariableValuePtr& VariableValues::zero() {
    static VariableValuePtr s_zero(make_shared<SpecialConst>("constant:zero","0"));
    return s_zero;
  }
  VariableValuePtr& VariableValues::one() {
    static VariableValuePtr s_one(make_shared<SpecialConst>("constant:one","1"));
    return s_one;
  }
  
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
    if (x.size())
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
        assert(size());
        // return a more meaningful error message to the user than what STL does
        if (ValueVector::flowVars.max_size()-ValueVector::flowVars.size()<size())
          throw runtime_error("Maximum processing data exceeded.");
        ValueVector::flowVars.resize(ValueVector::flowVars.size()+size());
        break;
      case stock:
      case integral:
        m_idx=ValueVector::stockVars.size();
        assert(size());
        // return a more meaningful error message to the user than what STL does
        if (ValueVector::stockVars.max_size()-ValueVector::stockVars.size()<size())
          throw runtime_error("Maximum processing data exceeded.");
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
        //assert(idxInRange()); // assertions fail after cancelled reset()
        if (size_t(m_idx)<ValueVector::flowVars.size())
          return ValueVector::flowVars[m_idx];
        break;
      case stock:
      case integral:
        //assert(idxInRange()); // assertions fail after cancelled reset()
        if (size_t(m_idx)<ValueVector::stockVars.size())
          return ValueVector::stockVars[m_idx];
        break;
      default: break;
      }
    static const double zero=0;
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
  
  const std::string& VariableValue::init(const std::string& x)  {
    m_init=x;
    // don't reallocate if initialisation will be ignored (eg wired flow)
    if (x.empty() || (m_type!=parameter && isFlowVar() && cminsky().definingVar(valueId())))
      return m_init;
    auto tensorInit=cminsky().variableValues.initValue(*this);
    if (tensorInit.size()>size()) m_idx=-1; // force reallocation
    index(tensorInit.index());
    hypercube(tensorInit.hypercube());
    return m_init;
  }

  const Hypercube& VariableValue::hypercube() const
  {
    if (rhs) return rhs->hypercube();
    // return the initialisation hypercube if not a defined flowVar
    if (tensorInit.rank()>0 && (!isFlowVar() || m_type==parameter || !cminsky().definingVar(valueId())))
      return tensorInit.hypercube();
    return m_hypercube;
  }

  void VariableValue::sliderSet(double x)
  {
    if (!isfinite(x)) return;
    if (x<sliderMin) x=sliderMin;
    if (x>sliderMax) x=sliderMax;
    sliderStep=maxSliderSteps();    
    init(to_string(x));
    setValue(x);
  }

  void VariableValue::incrSlider(double step)
  {
    sliderSet(value()+step*(sliderStepRel? value(): 1)*sliderStep);
  }
  
  void VariableValue::adjustSliderBounds()
  {
    if (size()==1 && !isnan(value()))
      {
        if (!isfinite(sliderMax) ||sliderMax<value())
          sliderMax=value()? abs(10*value()):1;
        if (!isfinite(sliderMin) || sliderMin>=value())
          sliderMin=value()? -abs(10*value()):-1;
        assert(sliderMin<sliderMax);
        sliderStep=maxSliderSteps(); 
      }
  }

  
  TensorVal VariableValues::initValue
  (const VariableValue& v, set<string>& visited) const
  {
    if (v.tensorInit.rank()>0)
      return v.tensorInit;
    
    const FlowCoef fc(v.init());
    if (trimWS(fc.name).empty())
      return fc.coef;

    // special generator functions handled here
    auto p=fc.name.find('(');
    if (p!=string::npos)
      {
        //        const string fn=fc.name.substr(0,p);
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

        if (fc.name.starts_with("iota"))
          for (size_t i=0; i<r.size(); ++i)
            r[i]=i;
        else if (fc.name.starts_with("one"))
          for (size_t i=0; i<r.size(); ++i)
            r[i]=1;
        else if (fc.name.starts_with("zero") || fc.name.starts_with("eye"))
          {
            for (size_t i=0; i<r.size(); ++i)
              r[i]=0;
            if (fc.name.starts_with("eye"))
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
        else if (fc.name.starts_with("rand"))
          {
            for (size_t i=0; i<r.size(); ++i)
              r[i]=double(rand())/RAND_MAX; // NOLINT
          }
        return r;
      }
        
    // resolve name
    auto valueId=minsky::valueId(v.m_scope.lock(), fc.name);
    if (visited.contains(valueId))
      throw error("circular definition of initial value for %s",
                  fc.name.c_str());
    const VariableValues::const_iterator vv=find(valueId);
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
        if (v.tensorInit.rank())
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
        if (v.tensorInit.rank())
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


  void VariableValue::exportAsCSV(const string& filename, const string& comment, bool tabular) const
  {
    ofstream of(filename);
    if (!of)
      throw runtime_error("Unable to open "+filename+" for writing.");
    if (!comment.empty())
      of<<R"(""")"<<comment<<R"(""")"<<endl;
               
    // calculate longest dimension
    auto dims=hypercube().dims();
    auto longestDim=max_element(dims.begin(),dims.end())-dims.begin();

    const auto& xv=hypercube().xvectors;
    
    ostringstream os;
    for (const auto& i: xv)
      {
        if (&i>xv.data()) os<<",";
        os<<json(static_cast<const NamedDimension&>(i));
      }
    of<<quoted("RavelHypercube=["+os.str()+"]")<<endl;
    if (tabular && rank()>0)
      of<<"HorizontalDimension="<<quoted(xv[longestDim].name)<<endl;
    
    for (size_t i=0; i<xv.size(); ++i)
      if (!tabular || i!=longestDim)
        of<<CSVQuote(xv[i].name,',')<<",";

    if (tabular && rank()>0)
      for (size_t k=0; k<dims[longestDim]; ++k)
        {
          if (k>0) of<<",";
          of<<CSVQuote(str(xv[longestDim][k],xv[longestDim].dimension.units),',');
        }
    else
      of<<"value$";
    of<<"\n";

    auto idxv=index();

    if (tabular && rank()>0)
      {
        size_t stride=1;
        for (size_t i=0; i<longestDim; ++i)
          stride*=dims[i];
        for (size_t i=0; i<hypercube().numElements(); i+=stride*dims[longestDim])
          for (size_t j=0; j<stride; ++j)
            {
              // collect elements in a buffer, which can be discarded if no data on line
              bool isData=false;
              vector<double> data;
              data.reserve(dims[longestDim]);
              for (size_t k=0; k<stride*dims[longestDim]; k+=stride)
                {
                  data.push_back(this->atHCIndex(i+j+k));
                  if (isfinite(data.back())) isData=true;
                }
              if (isData)
                {
                  auto idx=i+j;
                  for (size_t k=0; k<rank(); ++k)
                    {
                      auto div=std::div(idx, ssize_t(dims[k]));
                      if (k!=longestDim)
                        {
                          if (k>1 || (k>0 && longestDim>0)) of<<",";
                          of << "\""<<str(xv[k][div.rem], xv[k].dimension.units) << "\"";
                        }
                      idx=div.quot;
                    }
                  for (auto d: data)
                    {
                      of<<",";
                      if (isfinite(d)) of<<d;
                    }
                  of<<"\n";
                }
            }
      }
    else
      {
        size_t i=0;
        for (auto d=begin(); d!=end(); ++i, ++d)
          if (isfinite(*d))
            {
              ssize_t idx=idxv.empty()? i: idxv[i];
              for (size_t j=0; j<rank(); ++j)
                {
                  auto div=std::div(idx, ssize_t(dims[j]));
                  of << "\""<<str(xv[j][div.rem], xv[j].dimension.units) << "\",";
                  idx=div.quot;
                }
              of << *d << "\n";
            }
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
    string udfDefinition;
    try
      {
        udfDefinition=varNode && varNode->rhs? varNode->rhs->matlabStr():"";
      }
    catch (const std::exception& ex)
      {
        udfDefinition=ex.what(); // if matlabStr fails, insert error message
      }
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
      init(),
      sliderStep, sliderMin, sliderMax,
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
