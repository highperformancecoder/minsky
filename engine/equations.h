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
/*
  Structures for building a mathematical representation of the Minsky
  model as a directed acyclic graph (DAG)
*/

#ifndef EQUATIONS_H
#define EQUATIONS_H

#include "evalOp.h"
#include "godleyIcon.h"
#include "switchIcon.h"
#include "lock.h"

#include "operation.h"
#include <cairo_base.h>
#include <classdesc.h>
#include <ostream>
#include <vector>
#include <map>
#include <string>
#include <cstddef>
#include "integral.h"

namespace minsky
{
  class Minsky;
  /// checks if any GUI events are waiting, and proces an event if so
  void doOneEvent(bool idleTasksOnly);
}

namespace MathDAG
{
  using namespace std;
  using classdesc::shared_ptr;
  using namespace minsky;
  class SystemOfEquations;

  struct Node;
  /// a manipulator to make iostream expressions easy 
  struct LaTeXManip
  {
    const Node& node;
    LaTeXManip(const Node& node): node(node) {}
  };

  struct MatlabManip
  {
    const Node& node;
    MatlabManip(const Node& node): node(node) {}
  };

  /// convert double to a LaTeX string representing that value
  string latex(double);
  //  string latex(const TensorVal&);
  /// convert an initialisation string into a matlab expression
  string latexInit(const string&);
  /// wraps in \mathrm if nm has more than one letter - and also takes
  /// into account LaTeX subscript/superscripts
  string mathrm(const string& nm);

  //  string matlab(const TensorVal&);
  /// convert an initialisation string into a matlab expression
  string matlabInit(const string&);
  
  struct Node
  {
    virtual ~Node() {}
    
    /// algebraic heirarchy level, used for working out whether
    /// brackets are necessary.
    virtual int BODMASlevel() const=0; 
    /// writes LaTeX representation of this DAG to the stream
    virtual ostream& latex(ostream&) const=0;
    std::string latexStr() const {ostringstream o; latex(o); return o.str();}
    /// writes a matlab representation of this DAG to the stream
    virtual ostream& matlab(ostream&) const=0;
    std::string matlabStr() const {ostringstream o; matlab(o); return o.str();}
    /// renders a visual representation of this node to \a surf
    /// graphic extends right from the current pen position (which
    /// needs to be defined), and pen is moved to the right edge of
    /// the graphic. The determine bounding box, render into a
    /// recording surface, and use width()/height()
    virtual void render(ecolab::cairo::Surface& surf) const=0;
    
    /// adds EvalOps to an EvalOpVector representing this node.
    /// @return a variable where the result is stored.  If a
    /// flowVariable has been provided in \a result, that may be used
    /// directly, otherwise a copy operation is added to ensure it
    /// receives the result.
    virtual VariableValuePtr addEvalOps(EvalOpVector&, const VariableValuePtr& result={})=0;
    /// returns evaluation order in sequence of variable defintions
    /// @param maxOrder is used to limit the recursion depth
    virtual int order(unsigned maxOrder) const=0;
    /// returns true if the evaluation of this involves tensor processing
    /// @param visited set to break graph cycles
    virtual bool tensorEval(std::set<const Node*>& visited) const=0;
    /// returns true if the evaluation of this involves tensor processing
    bool tensorEval() {std::set<const Node*> visited; return tensorEval(visited);}
    mutable int cachedOrder=-1;
    /// used within io streaming
    LaTeXManip latex() const {return LaTeXManip(*this);}
    MatlabManip matlab() const {return MatlabManip(*this);}

    /// support for the derivative operator. 
    // Done as a virtual function to ensure that all Node types get a
    // definition, however, the actual work is delegated back to
    // SystemOfEquations via a templated method.
    virtual std::shared_ptr<Node> derivative(SystemOfEquations&) const=0;
    /// reference to where this node's value is stored
    VariableValuePtr result;
  };

  typedef std::shared_ptr<Node> NodePtr;
  /// weak reference into subexpression cache
  struct WeakNodePtr
  {
    Node* payload;
    Node& operator*() {return *payload;}
    const Node& operator*() const {return *payload;}
    Node* operator->() {return payload;}
    const Node* operator->() const {return payload;}
    WeakNodePtr& operator=(const NodePtr& x)
    {payload=x.get(); return *this;}
    WeakNodePtr& operator=(Node* x)
    {payload=x; return *this;}
    WeakNodePtr(): payload(nullptr) {}
    template <class T>
    WeakNodePtr(const std::shared_ptr<T>& x): payload(x.get()) {}
    operator bool() const {return payload!=nullptr;}
  };

  inline ostream& operator<<(ostream& o, LaTeXManip m)
  {return m.node.latex(o);}
  inline ostream& operator<<(ostream& o, MatlabManip m)
  {return m.node.matlab(o);}

  // the presence of the result member in the base class means a ConstantDAG cannot be declared static
  struct ConstantDAG: public Node
  {
    string value;
    ConstantDAG(const string& value="0"): value(value.length()? value: "0") {}
    ConstantDAG(double value): value(str(value)) {}
    int BODMASlevel() const  override {return 0;}
    int order(unsigned maxOrder) const  override {return 0;}
    bool tensorEval(std::set<const Node*>&) const override {return false;}
    ostream& latex(ostream& o) const  override {return o<<value;}
    ostream& matlab(ostream& o) const  override {return o<<value;}
    void render(ecolab::cairo::Surface& surf) const override;
    VariableValuePtr addEvalOps(EvalOpVector&, const VariableValuePtr&) override;
    NodePtr derivative(SystemOfEquations&) const override;
  };

  class VariableDAG: public Node, public VariableType
  {
  public:
    string valueId;
    Type type=undefined;
    string name;
    string init="0";
    WeakNodePtr rhs;
    IntOp* intOp=0; /// reference to operation if this is
                                 /// an integral variable
    VariableDAG() {}
    VariableDAG(const string& valueId, const string& name, Type type): 
      valueId(valueId), type(type), name(name) {}
    int BODMASlevel() const  override {return 0;}
    int order(unsigned maxOrder) const override {
      if (rhs) {
        if (cachedOrder>=0) return cachedOrder;
        if (maxOrder==0) throw error("maximum order recursion reached");
        return cachedOrder=rhs->order(maxOrder-1)+1;
      }
      else
        return 0;
    }
    bool tensorEval(std::set<const Node*>&) const override;
    using Node::tensorEval;
    using Node::latex;
    using Node::matlab;
    using Node::addEvalOps;
    ostream& latex(ostream&) const override;
    ostream& matlab(ostream&) const override;
    VariableValuePtr addEvalOps(EvalOpVector&, const VariableValuePtr& v={}) override;
    void render(ecolab::cairo::Surface& surf) const override;
    NodePtr derivative(SystemOfEquations&) const override; 
    /// adds a TensorEvalOp, returns true if successful
    bool addTensorOp(EvalOpVector& ev);
  };

  typedef shared_ptr<VariableDAG> VariableDAGPtr;

  /// represents the input of an integration operation - differs from
  /// Variable DAG in that it doesn't refer to the VariableValue
  struct IntegralInputVariableDAG: public VariableDAG
  {
    VariableValuePtr addEvalOps(EvalOpVector&,const VariableValuePtr&) override;
  };

  struct OperationDAGBase: public Node, public OperationType  
  {
    vector<vector<WeakNodePtr> > arguments;
    string name;
    string init="0";
    ItemPtr state;
    OperationDAGBase(const string& name=""): 
      name(name) {}
    virtual Type type() const=0;
    /// factory method 
    static OperationDAGBase* create(Type type, const string& name="");
    int order(unsigned maxOrder) const override;
    bool tensorEval(std::set<const Node*>&) const override;
    using Node::tensorEval;
    VariableValuePtr addEvalOps(EvalOpVector&, const VariableValuePtr&) override;
    void checkArg(unsigned i, unsigned j) const;
  };

  template <OperationType::Type T>
  struct OperationDAG: public OperationDAGBase
  {
    Type type() const override {return T;}
    OperationDAG(const string& name=""): OperationDAGBase(name) 
    {arguments.resize(OperationTypeInfo::numArguments<T>());}
    int BODMASlevel() const override {
      switch (type())
        {
        case OperationType::multiply:
        case OperationType::divide:
        case OperationType::and_:
          return 1;
        case OperationType::subtract:
        case OperationType::add:
        case OperationType::or_:
          return 2;
        case OperationType::constant: // varies, depending on what's in it
          if (name.find_first_of("+-")!=string::npos)
            return 2;
          else
            return 1;
        default:
          return 0;
        }
    }
    ostream& latex(ostream&) const override; 
    ostream& matlab(ostream& o) const override;
    void render(ecolab::cairo::Surface& surf) const override;
    using Node::latex;
    using Node::matlab;
    NodePtr derivative(SystemOfEquations&) const override; 
  };

  /// represents a Godley column
  struct GodleyColumnDAG: public OperationDAG<OperationType::subtract>
  {
    std::string name; //unqualified name of stock variable
    int order(unsigned maxOrder) const override {return 0;} // Godley columns define integration vars
  };

  struct LockDAG: public Node
  {
    const Lock& lock;
    WeakNodePtr rhs;
    LockDAG(const Lock& lock): lock(lock) {}
    int BODMASlevel() const override {return 0;} 
    ostream& latex(ostream& o) const override  {return o<<"locked";} 
    ostream& matlab(ostream& o) const override  {return o<<"";} 
    void render(ecolab::cairo::Surface& surf) const override;
    VariableValuePtr addEvalOps(EvalOpVector&, const VariableValuePtr& result={}) override;
    int order(unsigned maxOrder) const override {return rhs? rhs->order(maxOrder-1)+1:0;}
    bool tensorEval(std::set<const Node*>&) const override {return true;}
    std::shared_ptr<Node> derivative(SystemOfEquations&) const override
    {lock.throw_error("derivative not defined for locked objects");}
  };
  
  class SubexpressionCache
  {
    std::map<std::string, NodePtr > cache;
    std::map<std::string, VariableDAGPtr> integrationInputs;
    std::map<const Node*, NodePtr> reverseLookupCache;
  public:
    std::string key(const OperationBase& x) const {
      return "op:"+std::to_string(std::size_t(x.ports(0).lock().get()));
    }
    std::string key(const VariableBase& x) const {
      return "var:"+x.valueId();
    }
    std::string key(const SwitchIcon& x) const {
      return "switch:"+std::to_string(std::size_t(x.ports(0).lock().get()));
    }
    std::string key(const Lock& x) const {
      return "lock:"+std::to_string(std::size_t(x.ports(0).lock().get()));
    }
    /// strings refer to variable names
    std::string key(const string& x) const {
      return "var:"+x;
    }
    template <class T>
    bool exists(const T& x) {return cache.count(key(x));}
    template <class T>
    NodePtr operator[](const T& x) const {
      auto r=cache.find(key(x));
      if (r!=cache.end())
        return r->second;
      else
        return NodePtr();
    }
    template <class T>
    const NodePtr& insert(const T& x, const NodePtr& n) {
      reverseLookupCache[n.get()]=n;
      return cache.insert(make_pair(key(x),n)).first->second;
    }
    void insertIntegralInput(const string& name, const VariableDAGPtr& n) {
      integrationInputs.insert(make_pair("input:"+name,n));
      reverseLookupCache[n.get()]=n;
    }
    VariableDAGPtr getIntegralInput(const string& name) const {
      auto r=integrationInputs.find("input:"+name);
      if (r!=integrationInputs.end())
        return r->second;
      else
        return VariableDAGPtr();
    }
    std::size_t size() const {return cache.size()+integrationInputs.size();}
    /// returns NodePtr corresponding to object \x, if it exists in cache, nullptr otherwise
    NodePtr reverseLookup(const Node& x) const {
      auto it=reverseLookupCache.find(&x);
      if (it==reverseLookupCache.end())
        return NodePtr();
      else
        return it->second;
    }
    // inserts x anonymously
    //NodePtr insertAnonymous(const NodePtr& x) {
    NodePtr insertAnonymous(NodePtr x) {
      assert(x);
      return reverseLookupCache.insert(make_pair(x.get(), x)).first->second;
    }
  };


  class SystemOfEquations
  {
    SubexpressionCache expressionCache;
    // these are weak references
    vector<VariableDAG*> variables;
    vector<VariableDAG*> integrationVariables;
    set<string> processedColumns; // to avoid double counting shared columns
    vector<pair<VariableDAGPtr,string>> derivInputs; //handle recursively defined stock vars and derivatives
    
    const Minsky& minsky;

    /// create a variable DAG. returns cached value if previously called
    NodePtr makeDAG(const string& valueId, const string& name, VariableType::Type type);
    NodePtr makeDAG(VariableBase& v)
    {v.ensureValueExists(v.vValue().get(),v.name()); return makeDAG(v.valueId(),uqName(v.name()),v.type());}
    /// create an operation DAG. returns cached value if previously called
    NodePtr makeDAG(const OperationBase& op);
    NodePtr makeDAG(const SwitchIcon& op);
    NodePtr makeDAG(const Lock& op);

    /// returns cached subexpression node representing what feeds the
    /// wire, creating a new one if necessary
    NodePtr getNodeFromWire(const Wire& wire);

    void processGodleyTable
    (map<string, GodleyColumnDAG>& godleyVariables, const GodleyIcon&);

    /// applies the chain rule to expression x
    template <class Expr> NodePtr chainRule(const Expr& x, const Expr& deriv);

    /// used to rename ambiguous variables in different scopes
    std::set<std::string> varNames;
    /// keep track of derivatives of variables, to trap definition loops
    std::set<std::string> processingDerivative;
    /// table of user defined functions and their definitions
    std::map<std::string, std::string> userDefinedFunctions;
  public:
    /// construct the system of equations 
    SystemOfEquations(const Minsky&, const Group&g);
    SystemOfEquations(const Minsky& m);
    ostream& latex(ostream&) const; ///< render as a LaTeX eqnarray
    /// Use LaTeX brqn environment to wrap long lines
    ostream& latexWrapped(ostream&) const; 
    ostream& matlab(ostream&) const; ///< render as MatLab code
    /// create equations suitable for Runge-Kutta solver
    /// @param vector of equations to be constructed
    /// @param vector of integrals to be constructed
    /// @param portValMap - map of flowVar ids assigned with an output port
    void populateEvalOpVector
    (EvalOpVector& equations, std::vector<Integral>& integrals);

    // ensure all variables have their output port's variable value up
    // to date and add evalOps for plots and sheets
    void updatePortVariableValue(EvalOpVector& equations);

    
    /// symbolically differentiate \a expr
    template <class Expr> NodePtr derivative(const Expr& expr);

    /// useful constants to share
    NodePtr zero{new ConstantDAG("0")}, one{new ConstantDAG("1")};
    
    VariableDAGPtr getNodeFromVar(const VariableBase& v);
    VariableDAGPtr getNodeFromValueId(const std::string& v)
    {return dynamic_pointer_cast<VariableDAG>(expressionCache[v]);}
    VariableDAGPtr getNodeFromIntVar(const std::string& valueId)
    {return expressionCache.getIntegralInput(valueId);}
    ostringstream getDefFromIntVar(const VariableBase& v);

    /// render equations into a cairo context
    void renderEquations(ecolab::cairo::Surface&, double height) const;
  };

  /// creates a new name to represent the derivative of a variable
  std::string differentiateName(const std::string& x);

  template <> NodePtr SystemOfEquations::derivative(const ConstantDAG&);
  template <> NodePtr SystemOfEquations::derivative(const VariableDAG&);

  template <OperationType::Type T>
  NodePtr OperationDAG<T>::derivative(SystemOfEquations& se) const
  {return se.derivative(*this);}

}


#endif
