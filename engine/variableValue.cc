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
#include "variableValue.h"
#include "flowCoef.h"
#include "str.h"
#include "minsky.h"
#include <ecolab_epilogue.h>
#include <error.h>

using namespace ecolab;
using namespace std;
namespace minsky
{
  std::vector<double> ValueVector::stockVars(1);
  std::vector<double> ValueVector::flowVars(1);

  const VariableValue& VariableValue::operator=(minsky::TensorVal const& x)
  {
    bool realloc=numElements()!=x.data.size();
    if (dims()!=x.dims) dims(x.dims);
    if (realloc) allocValue();
    memcpy(&valRef(), &x.data[0], x.data.size()*sizeof(x.data[0]));
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
        ValueVector::flowVars.resize
          (ValueVector::flowVars.size()+numElements());
        //      *this=init;
        break;
      case stock:
      case integral:
        m_idx=ValueVector::stockVars.size();
        ValueVector::stockVars.resize(ValueVector::stockVars.size()+numElements());
        //     *this=init;
        break;
      default: break;
      }
    return *this;
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
        if (size_t(m_idx+numElements())<=ValueVector::flowVars.size())
          return ValueVector::flowVars[m_idx];
      case stock:
      case integral:
        if (size_t(m_idx+numElements())<=ValueVector::stockVars.size())
          return ValueVector::stockVars[m_idx];
      default: break;
      }
    throw error("invalid access of variable value reference: %s",name.c_str());
  }

  TensorVal VariableValue::initValue
  (const VariableValues& v, set<string>& visited) const
  {
    if (!tensorInit.data.empty())
      return tensorInit;
    
    FlowCoef fc(init);
    if (trimWS(fc.name).empty())
      return fc.coef;
    else
      {
        // special generator functions handled here
        auto p=fc.name.find('(');
        if (p!=string::npos)
          {
            string fn=fc.name.substr(0,p);
            // unpack args
            const char* x=fc.name.c_str()+p+1;
            TensorVal r;
            char* e;
            for (;;)
              {
                auto tmp=strtol(x,&e,10);
                if (tmp>0 && e>x && *e)
                  {
                    x=e+1;
                    r.dims.push_back(tmp);
                  }
                else
                  break;
              }

            size_t n=1;
            for (auto i: r.dims) n*=i;
            r.data.resize(n);

            if (fn=="iota")
              for (size_t i=0; i<n; ++i)
                r.data[i]=i;
            else if (fn=="one")
              for (size_t i=0; i<n; ++i)
                r.data[i]=1;
            else if (fn=="zero" || fn=="eye")
              {
                for (size_t i=0; i<n; ++i)
                  r.data[i]=0;
                if (fn=="eye")
                  {
                    // diagonal elements set to 1
                    // find minimum dimension, and stride of diagonal elements
                    size_t mind=n, stride=1;
                    for (auto i: r.dims)
                      mind=min(mind, size_t(i));
                    for (size_t i=0; i<r.dims.size()-1; ++i)
                      stride*=(r.dims[i]+1);
                    for (size_t i=0; i<mind; ++i)
                      r.data[stride*i]=1;
                  }
              }
            else if (fn=="rand")
              {
                srand(time(nullptr));
                for (size_t i=0; i<n; ++i)
                  r.data[i]=double(rand())/RAND_MAX;
              }
            return r;
          }
        
        // resolve name
        auto valueId=VariableValue::valueId(m_scope.lock(), fc.name);
        if (visited.count(valueId))
          throw error("circular definition of initial value for %s",
                      fc.name.c_str());
        VariableValues::const_iterator vv=v.end();
        vv=v.find(valueId);
        if (vv==v.end())
          throw error("Unknown variable %s in initialisation of %s",fc.name.c_str(), name.c_str());
        else
          {
            visited.insert(valueId);
            return fc.coef*vv->second.initValue(v, visited);
          }
      }
  }

  void VariableValue::reset(const VariableValues& v)
  {
    if (m_idx<0) allocValue(); 
    operator=(initValue(v));
  }


  int VariableValue::scope(const std::string& name) 
  {
    boost::smatch m;
    if (boost::regex_search(name, m, boost::regex(R"((\d*)]?:.*)")))
      if (m.size()>1 && m[1].matched && !m[1].str().empty())
        {
          int r;
          sscanf(m[1].str().c_str(),"%d",&r);
          return r;
        }
      else
        return -1;
    else
      // no scope information is present
      throw error("scope requested for local variable");
  }

  GroupPtr VariableValue::scope(GroupPtr scope, const std::string& a_name)
  {
    auto name=stripActive(a_name);
    if (name[0]==':' && scope)
      {
        // find maximum enclosing scope that has this same-named variable
        for (auto g=scope->group.lock(); g; g=g->group.lock())
          for (auto i: g->items)
            if (auto v=dynamic_cast<VariableBase*>(i.get()))
              {
                auto n=stripActive(v->name());
                if (n==name.substr(1)) // without ':' qualifier
                  {
                    scope=g;
                    goto break_outerloop;
                  }
              }
        scope.reset(); // global var
      break_outerloop: ;
      }
    return scope;
  }
  
  
  string VariableValue::valueIdFromScope(const GroupPtr& scope, const std::string& name)
  {
    if (!scope || !scope->group.lock())
      return VariableValue::valueId(-1,name); // retain previous global var id
    else
      return VariableValue::valueId(size_t(scope.get()), name);
}
  
  std::string VariableValue::uqName(const std::string& name)
  {
    string::size_type p=name.rfind(':');
    if (p==string::npos)
      return name;
    else
    return name.substr(p+1);
  }
 
  string VariableValues::newName(const string& name) const
  {
    int i=1;
    string trialName;
    do
      trialName=name+to_string(i++);
    while (count(VariableValue::valueId(trialName)));
    return trialName;
  }

  void VariableValues::reset()
  {
    // reallocate all variables
    ValueVector::stockVars.clear();
    ValueVector::flowVars.clear();
    for (auto& v: *this)
      v.second.allocValue().reset(*this);
}

  bool VariableValues::validEntries() const
  {
    for (auto& v: *this)
      if (!v.second.isValueId(v.first))
        return false;
    return true;
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

  string mantissa(double value, const EngNotation& e)
  {
    const char* conv;
    switch (e.sciExp-e.engExp)
      {
      case -3: conv="%7.4f"; break;
      case -2: conv="%6.3f"; break;
      case 0: case -1: conv="%5.2f"; break;
      case 1: conv="%5.1f"; break;
      case 2: case 3: conv="%5.0f"; break;
      default: return ""; // shouldn't be here...
      }
    char val[10];
    sprintf(val,conv,value*pow(10,-e.engExp));
    return val;
  }
  
  string expMultiplier(int exp)
  {return exp!=0? "×10<sup>"+std::to_string(exp)+"</sup>": "";}

//  namespace
//  {
//    // wrapper storing just the string part of an XVector element
//    struct XName: public string
//    {
//      XName() {}
//      XName(const XVector::value_type& x):
//        string(x.second) {}
//    };
//  }
  
  void VariableValue::makeXConformant(const VariableValue& a)
  {
    auto xv=xVector;
    for (auto& i: a.xVector)
      {
        set<string> alabels;
        for (auto& j: i)
          alabels.insert(str(j));
        size_t j=0;
        for (j=0; j<xv.size(); ++j)
          if (xv[j].name==i.name)
            {
              XVector newLabels;
              for (auto i: xVector[j])
                if (alabels.count(str(i)))
                  newLabels.push_back(i);
              xv[j].swap(newLabels);
              break;
            }
        if (j==xVector.size()) // axis not present on LHS, so increase rank
          xv.push_back(i);
      }
    setXVector(move(xv));
    if (numElements()==0)
      throw error("tensors nonconformant");
  }

  void VariableValue::computeStrideAndSize(const string& dim, size_t& stride, size_t& size) const
  {
    stride=1;
    if (dim.empty())
      // default to first axis if empty
      size=xVector.empty()? 1:xVector[0].size();
    else
      {
        for (auto& i: xVector)
          {
            size=i.size();
            if (i.name==dim) return;
            stride*=i.size();
          }
        throw runtime_error("axis "+dim+" not found");
      }
  }


  void VariableValue::exportAsCSV(const string& filename, const string& comment) const
  {
    ofstream of(filename);
    if (!comment.empty())
      of<<"\""<<comment<<"\"\n";
    size_t i=0;
    for (auto& i: xVector)
      of<<"\""<<i.name<<"\",";
    of<<"value$\n";
    for (auto d=begin(); d!=end(); ++i, ++d)
      if (isfinite(*d))
        {
          size_t stride=1;
          for (size_t j=0; j<xVector.size(); ++j)
            {
              of << "\""<<str(xVector[j][(i/stride) % xVector[j].size()]) << "\",";
              stride*=xVector[j].size();
            }
          of << *d << endl;
        }
  }
}
