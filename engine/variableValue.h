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
#include "str.h"
#include "CSVDialog.h"
#include "latexMarkup.h"
#include "valueId.h"
#include "variableSummary.h"
#include <regex> 
#include <utility>
#include <boost/locale.hpp>
using namespace boost::locale::conv;

namespace minsky
{
  class VariableValue;
  struct VariableValues;
  class Group;
  typedef std::shared_ptr<Group> GroupPtr;
  using namespace civita;

  struct VariableValueData: public civita::ITensorVal
  {
    using ITensorVal::operator=;
    VariableValueData& operator=(const VariableValueData&)=default;
    
    /// the initial value of this variable
    std::string init;
    /// when init is a tensor of values, this overrides the init string
    TensorVal tensorInit;
    /// when the RHS is attached to a tensor expression, this is a reference to it
    TensorPtr rhs;
    
    /// dimension units of this value
    Units units;
    bool unitsCached=false; // optimisation to prevent evaluating this units value more than once
    void setUnits(const std::string& x) {units=Units(x);}

    bool sliderVisible=false; // determined at reset time
    bool godleyOverridden=false;
    std::string name; // name of this variable
    classdesc::Exclude<std::weak_ptr<Group>> m_scope;

    /// for importing CSV files
    CSVDialog csvDialog;
    
  };
  
  class VariableValue: public VariableType, public VariableValueData
  {
    CLASSDESC_ACCESS(VariableValue);
  private:
    Type m_type;
    int m_idx=-1; /// index into value vector
    double& valRef(); 
    const double& valRef() const;
    
    friend class VariableManager;
    friend struct SchemaHelper;

    // values are always live
    ITensor::Timestamp timestamp() const override {return Timestamp::clock::now();}
    
  public:
    /// variable has an input port
    bool lhs() const {
      return m_type==flow || m_type==tempFlow;}
    /// variable is a temporary
    bool temp() const {
      return type()==tempFlow || type()==undefined;}
    /// returns true if variable's data is allocated on the flowVariables vector
    bool isFlowVar() const {
      return m_type!=stock && m_type!=integral && m_type!=undefined;
    }
    bool isZero() const {
      return m_type==constant && (init.empty() || init=="0");
    }

    VariableType::Type type() const {return m_type;}

    ///< value at the \a ith location of the vector/tensor. Default,
    ///(i=0) is right for scalar quantities
    // name valueAt to allow TCL exposure
    double valueAt(size_t i) const {
      if (i>=size())
        throw std::runtime_error("Index out of range");
      return operator[](i);
    }
    double value() const {return valueAt(0);}
    ///< set the value at the \a ith location
    double setValue(size_t i, double x) {
      if (i>=size())
        throw std::runtime_error("Index out of range");
      return operator[](i)=x;
    }
    double setValue(double x) {return setValue(0,x);}
    int idx() const {return m_idx;}
    void reset_idx() {m_idx=-1;}    

    double operator[](std::size_t i) const override {return *(&valRef()+i);}
    double& operator[](std::size_t i) override;

    const Index& index(Index&& i) override {
      std::size_t prevNumElems = size();
      m_index=i;
      if (idx()==-1 || !idxInRange() || (prevNumElems<size()))    
        allocValue();
      assert(idxInRange());
      return m_index;
    }
    using ITensorVal::index;
    
    template <class T>                                            
    void hypercube_(T x) {    
      assert(idx()==-1||idxInRange());
      std::size_t prevNumElems = size();
      ITensor::hypercube(x);    
      if (idx()==-1 || (prevNumElems<size()))    
        allocValue();
      assert(idxInRange());
    }

    bool idxInRange() const;

    const Hypercube& hypercube() const override {
      if (rhs) return rhs->hypercube();
      return m_hypercube;
    }
    
    const Hypercube& hypercube(const Hypercube& hc) override
    {hypercube_(hc); return m_hypercube;}
    const Hypercube& hypercube(Hypercube&& hc) override
    {hypercube_(hc); return m_hypercube;}
    using ITensorVal::hypercube;
                                                                           
    VariableValue(VariableType::Type type=VariableType::undefined, const std::string& name="", const std::string& init="", const GroupPtr& group=GroupPtr()): 
      m_type(type)
    {
      this->init=init;
      this->name=utf_to_utf<char>(name);
      m_scope=scope(group,name);
    }

    VariableValue(VariableType::Type type, const VariableValue& vv):  VariableValueData(vv) {
      m_type=type;
    }

    VariableValue(const VariableValue&)=delete;
    void operator=(const VariableValue&)=delete;
    
    using ITensorVal::operator=;
    VariableValue& operator=(TensorVal const&);
    VariableValue& operator=(const ITensor& x) override;

    /// allocate space in the variable vector. @returns reference to this
    VariableValue& allocValue();

    std::string valueId() const {return valueIdFromScope(m_scope.lock(),canonicalName(name));}

    void exportAsCSV(const std::string& filename, const std::string& comment="") const;

    Summary summary() const;
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
