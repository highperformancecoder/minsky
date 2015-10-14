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

#include "variableManager.h"
#include "slider.h"
#include "clickType.h"

#include <vector>
#include <cairo/cairo.h>

#include <arrays.h>

#include "polyPackBase.h"
#include "operationType.h"
#include "opVarBaseAttributes.h"

namespace minsky
{
  using namespace ecolab;
  using namespace classdesc;
  using namespace std;
  using classdesc::shared_ptr;

  class OperationBase: public classdesc::PolyBase<minsky::OperationType::Type>,
                       virtual public classdesc::PolyPackBase,
                       public OpVarBaseAttributes, public OperationType
  {
    CLASSDESC_ACCESS(OperationBase);
  public:
    static constexpr float l=-8, h=12, r=12;
    typedef OperationType::Type Type;

    size_t numPorts() const  {return ports().size();}
    ///factory method. \a ports is used for recreating an object read
    ///from a schema
    static OperationBase* create(Type type, 
                                 const vector<int>& ports = vector<int>()); 
    virtual OperationBase* clone() const=0;

    OperationBase() {}
    OperationBase(const OpVarBaseAttributes& attr): OpVarBaseAttributes(attr) {}
    OperationBase(const vector<int>& ports) {m_ports=ports;}
    virtual ~OperationBase() {}

    /// visual representation of operation on the canvas
    virtual void iconDraw(cairo_t *) const=0;

    /// return the symbolic name of this operation's type
    string name() const {return typeName(type());}
    /// return the symbolic name of \a type
    static string opName(int i) {return typeName(i);}

    /// returns true if from matches the out port, and to matches one of
    /// the in ports
    bool selfWire(int from, int to) const;

    /// returns a list of values the ports currently have
    string portValues() const;

    /// draws the icon onto the given cairo context 
    void draw(cairo_t*) const;

    /// returns the clicktype given a mouse click at \a x, \a y.
    ClickType::Type clickType(float x, float y) const override {
      return minsky::clickType(*this,x,y);
    }

    // returns true if multiple input wires are allowed.
    bool multiWire();
  protected:
    // manage the port structures associated with this operation
    virtual void addPorts()=0;
    void addPorts(unsigned numPorts);
    void addPorts(const vector<int>& p) {
      if (!p.empty())
        // TODO - possible consistency check possible here
        m_ports=p; 
      else
        addPorts(); 
    }
    void delPorts();

    friend struct EvalOpBase;
    friend class SchemaHelper;
  };

  template <minsky::OperationType::Type T>
  class Operation: public Poly<Operation<T>, OperationBase>,
                   public PolyPack<Operation<T> >
  {
    typedef OperationBase Super;
  public:
    typedef OperationType::Type Type;
    Type type() const {return T;}
    virtual void iconDraw(cairo_t *) const;

    // ensure copies create new ports
    Operation(const Operation& x): Super(x) {this->addPorts();}
    const Operation& operator=(const Operation& x)
    {Super::operator=(x); this->addPorts(); return *this;}

    Operation() {this->addPorts();}
    Operation(const vector<int>& ports) {this->addPorts(ports);}
    ~Operation() {this->delPorts();}
  protected:
    using OperationBase::addPorts;
    void addPorts(unsigned n) {OperationBase::addPorts(n);} // delegate to work around classdesc
    void addPorts() override {addPorts(OperationTypeInfo::numArguments<T>()+1);}
  };

  struct NamedOp
  {
    string description;
  };

  class Constant: public NamedOp, public Slider, 
                  public Operation<minsky::OperationType::constant>
  {
    typedef Operation<OperationType::constant> Super;
  public:
    double value; ///< constant value
    Constant(const vector<int>& ports=vector<int>()):  
      Super(ports), value(0) {}

    // clone has to be overridden, as default impl return object of
    // type Operation<T>
    Constant* clone() const  override {return new Constant(*this);}
    /// ensure slider does not override value
    void adjustSliderBounds();
    /// initialises sliderbounds based on current value, if not set otherwise
    void initOpSliderBounds();

    void pack(pack_t& x, const string& d) const
    {::pack(x,d,*this);}
      
    void unpack(unpack_t& x, const string& d)
    {::unpack(x,d,*this);}
  };

  class IntOp: public Operation<minsky::OperationType::integrate>
  {
    typedef Operation<OperationType::integrate> Super;
    // integrals have named integration variables
    ///integration variable associated with this op. -1 if not used
    int intVar; 
    /// name of integration variable
    string m_description; 
    CLASSDESC_ACCESS(IntOp);
    void addPorts(); // override. Also allocates new integral var if intVar==-1
    friend struct SchemaHelper;
  public:
    // offset for coupled integration variable, tr
    static constexpr float intVarOffset=10;

    IntOp(): intVar(-1) {}
    IntOp(const vector<int>& ports);
    ~IntOp() {if (!ecolab::interpExiting) variableManager().erase(intVarID(), true);}

    // ensure that copies create a new integral variable
    IntOp(const IntOp& x): 
      OperationBase(x), Super(x), intVar(-1), m_description(x.m_description)  {addPorts();}
    const IntOp& operator=(const IntOp& x); 

    // clone has to be overridden, as default impl return object of
    // type Operation<T>
    IntOp* clone() const  override {return new IntOp(*this);}

    /// set integration variable name
    void setDescription();
    /// @{ name of the associated integral variable
    void description(const string& desc) {
      m_description=desc;
      setDescription();
    }
    const string& description() const {return m_description;}
    /// @}
    /// generate a new name not otherwise in the system
    void newName(); 

    string valueId() const 
    {return VariableManager::valueId(group, m_description);}

    /// return ID of integration variable
    int intVarID() const {return intVar;}

    /// return reference to integration variable
    VariablePtr getIntVar() const {
      VariableManager::const_iterator i;
      if (intVar>-1 && (i=variableManager().find(intVar))!=variableManager().end())
        return *i;
      else
        return VariablePtr();
    }

    /// toggles coupled state of integration variable. Only valid for integrate
    /// @return coupled state
    bool toggleCoupled();
    bool coupled() const {
      return intVar>-1 && ports().size()>0 && ports()[0]==getIntVar()->outPort();
    }

    void setZoomOnAttachedVariable() {
      if (auto v=getIntVar())
        v->setZoom(zoomFactor);
    }

    void pack(pack_t& x, const string& d) const
    {::pack(x,d,*this);}
      
    void unpack(unpack_t& x, const string& d)
    {::unpack(x,d,*this);}
  };

  class DataOp: public NamedOp, public Operation<minsky::OperationType::data>
  {
    CLASSDESC_ACCESS(DataOp);
  public:
    std::map<double, double> data;
    DataOp(const vector<int>& ports=vector<int>()):
      Operation<OperationType::data>(ports) {}
    void readData(const string& fileName);
    // interpolates y data between x values bounding the argument
    double interpolate(double) const;
    // derivative of the interpolate function. At the data points, the
    // derivative is defined as the weighted average of the left & right
    // derivatives, weighted by the respective intervals
    double deriv(double) const;
    DataOp* clone() const override {return new DataOp(*this);}

    void pack(pack_t& x, const string& d) const
    {::pack(x,d,*this);}
      
    void unpack(unpack_t& x, const string& d)
    {::unpack(x,d,*this);}
  };

  /// shared_ptr class for polymorphic operation objects. Note, you
  /// may assume that this pointer is always valid, although currently
  /// the implementation doesn't guarantee it (eg reset() is exposed).
  class OperationPtr: public classdesc::shared_ptr<OperationBase>
  {
  public:
    OperationPtr(OperationType::Type type=OperationType::numOps,
                 const vector<int>& ports=vector<int>()): 
      shared_ptr<OperationBase>(OperationBase::create(type, ports)) {}
    // reset pointer to a newly created operation
    OperationPtr(OperationBase* op): shared_ptr<OperationBase>(op) 
    {assert(op);}
    OperationPtr clone() const {return OperationPtr(get()->clone());}
    virtual int id() const {return -1;}
    size_t use_count() const {return  classdesc::shared_ptr<OperationBase>::use_count();}
  };



  struct Operations: public TrackedIntrusiveMap<int, OperationPtr>
  {
    array<int> visibleOperations() const;
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
#endif
