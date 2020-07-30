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
#ifndef VARIABLE_H
#define VARIABLE_H

#include "slider.h"
#include "str.h"
#include "CSVParser.h"

#include <ecolab.h>
#include <arrays.h>

#include <vector>
#include <map>
// override EcoLab's default CLASSDESC_ACCESS macro
#include "classdesc_access.h"

#include "polyBase.h"
#include <polyPackBase.h>
#include "variableType.h"
#include "item.h"
#include <accessor.h>
#include <cairo/cairo.h>

namespace minsky
{
  class VariablePtr;
  struct SchemaHelper;
  class GodleyIcon;

  /// exception-safe increment/decrement of a counter in a block
  struct IncrDecrCounter
  {
    int& ctr;
    IncrDecrCounter(int& ctr): ctr(ctr) {++ctr;}
    ~IncrDecrCounter() {--ctr;}
  };

  namespace VarAccessors
  {
    // constructors defined below once member functions available
    struct NameAccessor: ecolab::TCLAccessor<minsky::VariableBase,std::string,0> {NameAccessor();};
    struct InitAccessor: ecolab::TCLAccessor<minsky::VariableBase,std::string,1> {InitAccessor();};
    struct ValueAccessor: ecolab::TCLAccessor<minsky::VariableBase,double> {ValueAccessor();};
    struct SliderVisibleAccessor: ecolab::TCLAccessor<minsky::VariableBase,bool> {SliderVisibleAccessor();};
  }

  class VariableBase: virtual public classdesc::PolyPackBase,
                      public Item, public Slider, public VariableType,
                      public VarAccessors::NameAccessor,
                      public VarAccessors::InitAccessor,
                      public VarAccessors::ValueAccessor,
                      public VarAccessors::SliderVisibleAccessor
  {
  public:
    typedef VariableType::Type Type;
  protected:
 
    friend struct minsky::SchemaHelper;
    
  private:
    CLASSDESC_ACCESS(VariableBase);
    std::string m_name; 
    mutable int unitsCtr=0; ///< for detecting reentrancy in units()
    static int stockVarsPassed; ///< for detecting reentrancy in units()

  protected:
    void addPorts();
    
  public:
    static int varsPassed; ///< for caching units calculation
    ///factory method
    static VariableBase* create(Type type); 

    virtual size_t numPorts() const=0;
    virtual Type type() const=0;

    /// attempt to replace this variable with variable of \a type.
    /// @throw if not possible
    void retype(VariableType::Type type);

    
    /// reference to a controlling item - eg GodleyIcon, IntOp or a Group if an IOVar.
    classdesc::Exclude<std::weak_ptr<Item>> controller;
    bool visible() const override;

    const VariableBase* variableCast() const override {return this;}
    VariableBase* variableCast() override {return this;}

    float zoomFactor() const override;
    
    /// @{ variable displayed name
    virtual std::string name() const;
    virtual std::string name(const std::string& nm);
    /// @}

    /// accessor for the name member (may differ from name() with top
    /// level variables)
    const std::string& rawName() const {return m_name;}
    
    bool ioVar() const override;
    
    /// ensure an associated variableValue exists
    void ensureValueExists(VariableValue* vv, const std::string& name) const;

    /// string used to link to the VariableValue associated with this
    virtual std::string valueId() const;
    /// returns valueId for \a nm. If nm is not qualified, this
    /// variable's scope is used
    std::string valueIdInCurrentScope(const std::string& nm) const;
    /// variableValue associated with this. nullptr if not associated with a variableValue
    std::shared_ptr<VariableValue> vValue() const;
    std::vector<unsigned> dims() const {
      if (auto v=vValue()) return v->hypercube().dims();
      else return {};
    }
      

    /// @{ the initial value of this variable
    std::string init() const; /// < return initial value for this variable
    std::string init(const std::string&); /// < set the initial value for this variable
    /// @}
    
    /// @{ current value associated with this variable
    virtual double value(const double&);
    virtual double value() const override;  
    /// @}
    
    //    void setValue(const TensorVal&);

    
    /// sets variable value (or init value)
    void sliderSet(double x);
    /// initialise slider bounds when slider first opened
    void initSliderBounds() const;
    void adjustSliderBounds() const;
    bool sliderVisible() const {return Slider::sliderVisible;}
    bool sliderVisible(const bool& v) {
          if (v) {initSliderBounds(); adjustSliderBounds();}
          return Slider::sliderVisible=v;
    }

    /// sets/gets the units associated with this type
    Units units(bool check=false) const override;
    void setUnits(const std::string&);
    std::string unitsStr() const {return units().str();}
    
    bool handleArrows(int dir,bool) override;
    
    /// variable is on left hand side of flow calculation
    bool lhs() const {return type()==flow || type()==tempFlow;} 
    /// variable is temporary
    bool temp() const {return type()==tempFlow || type()==undefined;}
    virtual VariableBase* clone() const override=0;
    bool isStock() const {return type()==stock || type()==integral;}

    virtual ~VariableBase();

    /** draws the icon onto the given cairo context 
        @return cairo path of icon outline
    */
    void draw(cairo_t*) const override;  
    void resize(const LassoBox& b) override;
    ClickType::Type clickType(float x, float y) override;

    /// @return true if variable is defined (inputWired() || isStock() && controlled)
    bool defined() const {return inputWired() || (isStock() && controller.lock());}
    
    bool inputWired() const;
    /// return a list of existing variables a variable in this group
    /// could be connected to
    std::vector<std::string> accessibleVars() const;

    /// return formatted mantissa and exponent in engineering format
    EngNotation engExp() const
    {return minsky::engExp(value());}
    std::string mantissa(const EngNotation& e) const
    {return minsky::mantissa(value(),e);}

    /// export this variable as a CSV file
    void exportAsCSV(const std::string& filename) const;
    /// import CSV file, using \a spec
    void importFromCSV(std::string filename, const DataSpec& spec);

    void insertControlled(Selection& selection) override;
  };

  template <minsky::VariableType::Type T>
  class Variable: public ItemT<Variable<T>, VariableBase>,
                  public classdesc::PolyPack<Variable<T> >
  {
  public:
    typedef VariableBase::Type Type;
    Type type() const override {return T;}
    size_t numPorts() const override;

    Variable(const Variable& x): VariableBase(x) {this->addPorts();}
    Variable& operator=(const Variable& x) {
      VariableBase::operator=(x);
      this->addPorts();
      return *this;
    }
    Variable(const std::string& name="") {this->name(name); this->addPorts();}
    std::string classType() const override 
    {return "Variable:"+VariableType::typeName(type());}
    Variable* clone() const override {
      auto v=ItemT<Variable<T>, VariableBase>::clone();
      v->controller.reset(); // cloned variables are not controlled by this's controller
      // v is a Variable*, but C++ covariant return types rule prevent it being declared as such
      assert(dynamic_cast<Variable*>(v));
      return static_cast<Variable*>(v); 
    }
  };

  struct VarConstant: public Variable<VariableType::constant>
  {
    int id;
    static int nextId;
    VarConstant(): id(nextId++) {ensureValueExists(nullptr,"");}
    std::string valueId() const override {return "constant:"+str(id);}
    std::string name() const override {return init();}
    std::string name(const std::string& nm) override {ensureValueExists(nullptr,""); return name();}
    using VariableBase::value;
    double value(const double& x) override {init(str(x)); return x;}
    VarConstant* clone() const override {auto r=new VarConstant(*this); r->group.reset(); return r;}
    std::string classType() const override {return "VarConstant";}
    void TCL_obj(classdesc::TCL_obj_t& t, const classdesc::string& d) override 
    {::TCL_obj(t,d,*this);}
  };

  class VariablePtr: 
    public classdesc::shared_ptr<VariableBase>
  {
    typedef classdesc::shared_ptr<VariableBase> PtrBase;
  public:
    virtual int id() const {return -1;}
    VariablePtr(VariableBase::Type type=VariableBase::undefined, 
                const std::string& name=""): 
      PtrBase(VariableBase::create(type)) {get()->name(name);}
    virtual ~VariablePtr() {}
    template <class P>
    VariablePtr(P* var): PtrBase(dynamic_cast<VariableBase*>(var)) 
    {
      // check for incorrect type assignment
      assert(!var || *this);
    }
    VariablePtr(const PtrBase& x): PtrBase(x) {}
    VariablePtr(const VariableBase& x): PtrBase(x.clone()) {}
    /// changes type of variable to \a type
    void retype(VariableBase::Type type);
    VariablePtr(const ItemPtr& x): 
      PtrBase(std::dynamic_pointer_cast<VariableBase>(x)) {}
    /// make variable's type consistent with the type of the valueId
    void makeConsistentWithValue();
  };

}
#include "variable.cd"
#include "variable.xcd"
#endif
