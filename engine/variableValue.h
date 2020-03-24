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
#ifndef VARIABLE_VALUE
#define VARIABLE_VALUE
#include "variableType.h"
#include "tensorInterface.h"
#include "tensorVal.h"
#include "ecolab.h"
#include "classdesc_access.h"
#include "constMap.h"
#include "str.h"
#include <boost/regex.hpp>

namespace minsky
{
  class VariableValue;
  struct VariableValues;
  class Group;
  typedef std::shared_ptr<Group> GroupPtr;
  using namespace civita;
  
  class VariableValue: public VariableType, public civita::ITensorVal
  {
    CLASSDESC_ACCESS(VariableValue);
  private:
    Type m_type;
    int m_idx; /// index into value vector
    double& valRef(); 
    const double& valRef() const;
    std::vector<unsigned> m_dims;
    std::vector<size_t> m_index;     
    
    friend class VariableManager;
    friend struct SchemaHelper;
  public:
    using ITensor::hypercube;
    /// variable has an input port
    bool lhs() const {
      return m_type==flow || m_type==tempFlow;}
    /// variable is a temporary
    bool temp() const {
      return type()==tempFlow || type()==undefined;}
    /// returns true if variable's data is allocated on the flowVariables vector
    bool isFlowVar() const {
      return m_type!=stock && m_type!=integral;
    }
    bool isZero() const {
      return m_type==constant && (init.empty() || init=="0");
    }

    Type type() const {return m_type;}

    /// the initial value of this variable
    std::string init;
    /// when init is a tensor of values, this overrides the init string
    TensorVal tensorInit;

    /// dimension units of this value
    Units units;
    bool unitsCached=false; // optimisation to prevent evaluating this units value more than once
    void setUnits(const std::string& x) {units=Units(x);}
    
    bool godleyOverridden;
    std::string name; // name of this variable
    classdesc::Exclude<std::weak_ptr<Group>> m_scope;

    ///< value at the \a ith location of the vector/tensor. Default,
    ///(i=0) is right for scalar quantities
    double value(size_t i=0) const {return *(begin()+i);}
    int idx() const {return m_idx;}
    void reset_idx() {m_idx=-1;}    

    // values are always live
    Timestamp timestamp() const override {return Timestamp::clock::now();}
    
    double operator[](size_t i) const override {return *(&valRef()+i);}
    double& operator[](size_t i) override {return *(&valRef()+i);}


    std::vector<size_t> index() const override {return m_index;}
    void index(const std::vector<size_t>& i) override {
      size_t prevNumElems = size();
      m_index=i;
      if (idx()==-1 || (prevNumElems<size()))    
        allocValue();    
    }

    
    size_t numDenseElements() const {return hypercube().numElements();}
  
    size_t size() const override
    {return m_index.size() ? m_index.size(): numDenseElements();}    
    
    template <class T>                                            
    void hypercube_(T x) {    
      size_t prevNumElems = size();
      ITensor::hypercube(x);    
      if (idx()==-1 || (prevNumElems<size()))    
        allocValue();    
    }
    
    const Hypercube& hypercube(const Hypercube& hc) override
    {hypercube_(hc); return m_hypercube;}
    const Hypercube& hypercube(Hypercube&& hc) override
    {hypercube_(hc); return m_hypercube;}
                                                                              
    void makeXConformant(const ITensor& x) {
      m_hypercube.makeConformant(x.hypercube());
    }
    
    VariableValue(Type type=VariableType::undefined, const std::string& name="", const std::string& init="", const GroupPtr& group=GroupPtr()): 
      m_type(type), m_idx(-1), init(init), godleyOverridden(0), name(name), m_scope(scope(group,name)) {}

//    const VariableValue& operator=(double x) {valRef()=x; return *this;}
//    const VariableValue& operator+=(double x) {valRef()+=x; return *this;}
//    const VariableValue& operator-=(double x) {valRef()-=x; return *this;}
    const VariableValue& operator=(TensorVal const&);
    const VariableValue& operator=(const ITensor& x) override;
//    const VariableValue& operator+=(const TensorVal& x);
//    const VariableValue& operator-=(const TensorVal& x);

    /// allocate space in the variable vector. @returns reference to this
    VariableValue& allocValue();

    std::string valueId() const {return valueIdFromScope(m_scope.lock(),name);}
    
    /// evaluates the initial value, based on the set of variables
    /// contained in \a VariableManager. \a visited is used to check
    /// for circular definitions
    TensorVal initValue
    (const VariableValues&, std::set<std::string>& visited) const;
    TensorVal initValue(const VariableValues& v) const {
      std::set<std::string> visited;
      return initValue(v, visited);
    }
    void reset(const VariableValues&); 

    /// check that name is a valid valueId (useful for assertions)
    static bool isValueId(const std::string& name) {
      return name.length()>1 && name.substr(name.length()-2)!=":_" &&
        boost::regex_match(name, boost::regex(R"((constant)?\d*:[^:\s]+)"));
    }

    /// construct a valueId
    static std::string valueId(int scope, std::string name) {
      if (scope<0) return ":"+stripActive(uqName(name));
      else return std::to_string(scope)+":"+stripActive(uqName(name));
    }
    static std::string valueId(std::string name) {
      return valueId(scope(name), name);
    }
    /// starting from reference group ref, applying scoping rules to determine the actual scope of \a name
    /// If name prefixed by :, then search up group heirarchy for locally scoped var, otherwise return ref
    static GroupPtr scope(GroupPtr ref, const std::string& name);
    static std::string valueId(const GroupPtr& ref, const std::string& name) 
    {return valueIdFromScope(scope(ref,name), name);}
    static std::string valueIdFromScope(const GroupPtr& scope, const std::string& name);
    
    /// extract scope from a qualified variable name
    /// @throw if name is unqualified
    static int scope(const std::string& name);
    /// extract unqualified portion of name
    static std::string uqName(const std::string& name);

    void exportAsCSV(const std::string& filename, const std::string& comment="") const;
  };

  struct ValueVector
  {
    /// vector of variables that are integrated via Runge-Kutta. These
    /// variables label the columns of the Godley table
    static std::vector<double> stockVars;
    /// variables defined as a simple function of the stock variables,
    /// also known as lhs variables. These variables appear in the body
    /// of the Godley table
    static std::vector<double> flowVars;
  };

  struct VariableValues: public ConstMap<std::string, VariableValue>
  {
    VariableValues() {clear();}
    void clear() {
      ConstMap<std::string, VariableValue>::clear();
      // add special values for zero and one, used for the derivative
      // operator in SystemOfEquations
      insert
        (value_type("constant:zero",
                    VariableValue(VariableType::constant,"constant:zero","0")));
      insert
        (value_type("constant:one",
                    VariableValue(VariableType::constant,"constant:one","1")));
    }
    /// generate a new valueId not otherwise in the system
    std::string newName(const std::string& name) const;
    void reset();
    /// checks that all entry names are valid
    bool validEntries() const;
    void resetUnitsCache() {
      for (auto& i: *this)
        i.second.unitsCached=false;
    }
  };
  
  struct EngNotation {int sciExp, engExp;};
  /// return formatted mantissa and exponent in engineering format
  EngNotation engExp(double value);
  std::string mantissa(double value, const EngNotation&);
  std::string expMultiplier(int exp);


}

#include "variableValue.cd"
#include "variableValue.xcd"

#ifdef _CLASSDESC
#pragma omit pack minsky::VariableValue
#pragma omit unpack minsky::VariableValue
#endif

namespace classdesc_access
{
  // nobble these as we're not using them
  template <>
  struct access_pack<minsky::VariableValue>:
    public classdesc::NullDescriptor<classdesc::pack_t> {};
  template <>
  struct access_unpack<minsky::VariableValue>:
    public classdesc::NullDescriptor<classdesc::unpack_t> {};
}

#endif
