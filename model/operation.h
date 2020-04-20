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
#ifndef OPERATION_H
#define OPERATION_H

#include <ecolab.h>
#include <xml_pack_base.h>
#include <xml_unpack_base.h>

// override EcoLab's default CLASSDESC_ACCESS macro
#include "classdesc_access.h"

#include "item.h"
#include "variable.h"
#include "slider.h"

#include <vector>
#include <cairo/cairo.h>

#include <arrays.h>

#include "polyBase.h"
#include "polyPackBase.h"
#include <pack_base.h>
#include "operationType.h"

namespace minsky
{
  using namespace ecolab;
  using namespace classdesc;
  using namespace std;
  using classdesc::shared_ptr;
  class OperationPtr;

  class OperationBase: virtual public classdesc::PolyPackBase,
                       public Item, public OperationType
  {
    CLASSDESC_ACCESS(OperationBase);
  public:
    static constexpr float l=-8, h=12, r=12;
    typedef OperationType::Type Type;

    virtual size_t numPorts() const=0;
    ///factory method. \a ports is used for recreating an object read
    ///from a schema
    static OperationBase* create(Type type); 
    virtual Type type() const=0;

    const OperationBase* operationCast() const override {return this;}
    OperationBase* operationCast() override {return this;}
    
    virtual ~OperationBase() {}

    /// visual representation of operation on the canvas
    virtual void iconDraw(cairo_t *) const=0;

    /// returns a list of values the ports currently have
    string portValues() const;

    // returns true if multiple input wires are allowed.
    bool multiWire();

    // manage the port structures associated with this operation
    virtual void addPorts();

    void draw(cairo_t*) const override;
    void resize(const LassoBox& b) override;
    ClickType::Type clickType(float x, float y) override;        

    /// current value of output port
    double value() const override;

    /// operation argument. For example, the offset used in a
    /// difference operator, or binsize in a binning op
    double arg=1;

    /// axis selector in tensor operations
    string axis;

    /// return dimension names of tensor object attached to input
    /// if binary op, then the union of dimension names is returned
    std::vector<string> dimensions() const;
    Units units(bool check=false) const override;

  protected:

    friend struct EvalOpBase;
    friend class SchemaHelper;
  };

  template <minsky::OperationType::Type T>
  class Operation: public ItemT<Operation<T>, OperationBase>,
                   public PolyPack<Operation<T> >
  {
    typedef OperationBase Super;
  public:
    typedef OperationType::Type Type;
    Type type() const override {return T;}
    void iconDraw(cairo_t *) const override;
    size_t numPorts() const override 
    {return OperationTypeInfo::numArguments<T>()+1;}
    Operation() {
      this->addPorts();
      // custom arg defaults
      switch (T)  {
        case OperationType::runningSum: case OperationType::runningProduct:
          this->arg=-1;
          break;
        default:
          break;
        }
    }
    Operation(const Operation& x): Super(x) {this->addPorts();}
    Operation& operator=(const Operation& x) {
      Super::operator=(x);
      this->addPorts();
      return *this;
    }
    std::string classType() const override {return "Operation:"+OperationType::typeName(T);}
  };

  class Time: public Operation<OperationType::time>
  {
  public:
    Units units(bool) const override;
  };
  
  class Derivative: public Operation<OperationType::differentiate>
  {
  public:
    Units units(bool) const override;
  };

  class Copy: public Operation<OperationType::copy>
  {
  public:
    Units units(bool check) const override {return ports[1]->units(check);}
  };

  class IntOp;
  struct IntOpAccessor: public ecolab::TCLAccessor<minsky::IntOp, std::string>
  {IntOpAccessor();};
  
  class IntOp: public ItemT<IntOp, Operation<minsky::OperationType::integrate>>,
               public IntOpAccessor
  {
    typedef Operation<OperationType::integrate> Super;
    // integrals have named integration variables
    ///integration variable associated with this op.
    CLASSDESC_ACCESS(IntOp);
    friend struct SchemaHelper;
  public:
    // offset for coupled integration variable, tr
    static constexpr float intVarOffset=10;

    IntOp() {description("");}
    // ensure that copies create a new integral variable
    IntOp(const IntOp& x): 
      OperationBase(x), Super(x) {intVar.reset(); description(x.description());}
    ~IntOp() {removeControlledItems();}
    
    const IntOp& operator=(const IntOp& x); 

    /// @{ name of the associated integral variable
    std::string description(const std::string& desc);
    std::string description() const {return intVar? intVar->name(): "";}
    /// @}

    string valueId() const 
    {return intVar->valueId();}

    void removeControlledItems() const override;

   /// return reference to integration variable
    VariablePtr intVar; 

    bool handleArrows(int dir,bool) override {return intVar->handleArrows(dir,false);}

    /// toggles coupled state of integration variable. Only valid for integrate
    /// @return coupled state
    bool toggleCoupled();
    bool coupled() const {
      assert(intVar);
      return ports.size()>0 && intVar->ports.size()>0 && ports[0]==intVar->ports[0];
    }
    Units units(bool) const override;

    void pack(pack_t& x, const string& d) const override;
    void unpack(unpack_t& x, const string& d) override;

    void insertControlled(Selection& selection) override;
  };

  class DataOp: public ItemT<DataOp, Operation<minsky::OperationType::data>>,
                public ecolab::TCLAccessor<DataOp,std::string>
  {
    CLASSDESC_ACCESS(DataOp);
    friend struct SchemaHelper;
    string m_description;
  public:
    DataOp(): ecolab::TCLAccessor<DataOp,std::string>
      ("description",(ecolab::TCLAccessor<DataOp,std::string>::Getter)&DataOp::description,
       (ecolab::TCLAccessor<DataOp,std::string>::Setter)&DataOp::description)
    {}
    ~DataOp() {}
    
    const DataOp& operator=(const DataOp& x); 

    /// @{ name of the associated data operation
    std::string description() const;  
    std::string description(const std::string&);    
    /// @}
    std::map<double, double> data;
    void readData(const string& fileName);
    /// initialise with uniform random numbers 
    void initRandom(double xmin, double xmax, unsigned numSamples);
    /// interpolates y data between x values bounding the argument
    double interpolate(double) const;
    /// derivative of the interpolate function. At the data points, the
    /// derivative is defined as the weighted average of the left & right
    /// derivatives, weighted by the respective intervals
    double deriv(double) const;
    Units units(bool check) const override {return ports[1]->units(check);}

    /// called to initialise a variable value when no input wire is connected
    //    void initOutputVariableValue(VariableValue&) const;
    
    void pack(pack_t& x, const string& d) const override;
    void unpack(unpack_t& x, const string& d) override;
  };

  /// shared_ptr class for polymorphic operation objects. Note, you
  /// may assume that this pointer is always valid, although currently
  /// the implementation doesn't guarantee it (eg reset() is exposed).
  class OperationPtr: public classdesc::shared_ptr<OperationBase>
  {
  public:
    typedef classdesc::shared_ptr<OperationBase> PtrBase;
    OperationPtr(OperationType::Type type=OperationType::numOps): 
      PtrBase(OperationBase::create(type)) {}
    // reset pointer to a newly created operation
    OperationPtr(OperationBase* op): PtrBase(op) {assert(op);}
    OperationPtr clone() const {return OperationPtr(ItemPtr(get()->clone()));}
    size_t use_count() const {return  classdesc::shared_ptr<OperationBase>::use_count();}
    OperationPtr(const PtrBase& x): PtrBase(x) {}
    OperationPtr& operator=(const PtrBase& x) {PtrBase::operator=(x); return *this;}
    OperationPtr(const ItemPtr& x): 
      PtrBase(std::dynamic_pointer_cast<OperationBase>(x)) {}
  };


}

  // for TCL interfacing
inline std::ostream& operator<<(std::ostream& x, const std::vector<int>& y)
{
  for (size_t i=0; i<y.size(); ++i)
    x<<(i==0?"":" ")<<y[i];
  return x;
}


inline void pack(classdesc::pack_t&,const classdesc::string&,classdesc::ref<ecolab::urand>&) {}
inline void unpack(classdesc::pack_t&,const classdesc::string&,classdesc::ref<ecolab::urand>&) {}
inline void xml_pack(classdesc::xml_pack_t&,const classdesc::string&,classdesc::ref<ecolab::urand>&) {}
inline void xml_unpack(classdesc::xml_unpack_t&,const classdesc::string&,classdesc::ref<ecolab::urand>&) {}



#include "operation.cd"
#include "operation.xcd"
#endif
