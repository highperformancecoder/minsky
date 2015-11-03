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
#include "variableManager.h"
#include "portManager.h"
#include "godley.h"
#include "switchIcon.h"

#include "operation.h"
#include <cairo_base.h>
#include <classdesc.h>
#include <ostream>
#include <vector>
#include <map>

namespace minsky
{
  class Minsky;

  // An integral is an additional stock variable, that integrates its flow variable
  struct Integral
  {
    VariableValue stock;
    VariableValue input;
    IntOp* operation; //< reference to the interal operation object
    Integral(VariableValue input=VariableValue()):
      stock(VariableBase::integral), input(input), operation(NULL) {}
  };

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
  /// wraps in \mathrm if nm has more than one letter - and also takes
  /// into account LaTeX subscript/superscripts
  string mathrm(const string& nm);

  struct Node
  {
    /// algebraic heirarchy level, used for working out whether
    /// brackets are necessary.
    virtual int BODMASlevel() const=0; 
    /// writes LaTeX representation of this DAG to the stream
    virtual ostream& latex(ostream&) const=0; 
    /// writes a matlab representation of this DAG to the stream
    virtual ostream& matlab(ostream&) const=0; 
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
    virtual VariableValue addEvalOps
    (EvalOpVector&, std::map<int,VariableValue>& opValMap, 
     const VariableValue& result=VariableValue()) const=0;
    /// returns evaluation order in sequence of variable defintions
    /// @param maxOrder is used to limit the recursion depth
    virtual int order(unsigned maxOrder) const=0;
    /// used within io streaming
    LaTeXManip latex() const {return LaTeXManip(*this);}
    MatlabManip matlab() const {return MatlabManip(*this);}

    /// support for the derivative operator. 
    // Done as a virtual function to ensure that all Node types get a
    // definition, however, the actual work is delegated back to
    // SystemOfEquations via a templated method.
    virtual std::shared_ptr<Node> derivative(SystemOfEquations&) const=0;
    /// reference to where this node's value is stored
    mutable VariableValue result;

    Node(): result(VariableType::flow) {}
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
    WeakNodePtr operator=(const NodePtr& x)
    {payload=x.get(); return *this;}
    WeakNodePtr operator=(Node* x)
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
    double value;
    ConstantDAG(double value=0): value(value) {}
    int BODMASlevel() const  override {return 0;}
    int order(unsigned maxOrder) const  override {return 0;}
    ostream& latex(ostream& o) const  override {return o<<MathDAG::latex(value);}
    ostream& matlab(ostream& o) const  override {return o<<value;}
    void render(ecolab::cairo::Surface& surf) const override;
    VariableValue addEvalOps(EvalOpVector&, std::map<int,VariableValue>& opValMap,  
                             const VariableValue&) const override;
    NodePtr derivative(SystemOfEquations&) const override;
  };

  class VariableDAG: public Node, public VariableType
  {
  public:
    string valueId;
    int scope=-1; ///< namespace for this variable
    Type type=undefined;
    string name;
    double init=0;
    WeakNodePtr rhs;
    OperationPtr intOp; /// reference to operation if this is
                                 /// an integral variable
    VariableDAG() {}
    VariableDAG(const string& valueId, int scope, const string& name, Type type): 
      valueId(valueId), scope(scope), type(type), name(name) {}
    int BODMASlevel() const  override {return 0;}
    int order(unsigned maxOrder) const {
      if (rhs) {
        if (maxOrder==0) throw error("maximum order recursion reached");
        return rhs->order(maxOrder-1)+1;
      }
      else
        return 0;
    }
    ostream& latex(ostream&) const override;
    ostream& matlab(ostream&) const override;
    void render(ecolab::cairo::Surface& surf) const override;
    VariableValue addEvalOps
    (EvalOpVector&, std::map<int,VariableValue>& opValMap,  
     const VariableValue& v=VariableValue()) const override;
    using Node::latex;
    using Node::matlab;
    NodePtr derivative(SystemOfEquations&) const override; 
  };

  typedef shared_ptr<VariableDAG> VariableDAGPtr;

  /// represents the input of an integration operation - differs from
  /// Variable DAG in that it doesn't refer to the VariableValue
  struct IntegralInputVariableDAG: public VariableDAG
  {
    VariableValue addEvalOps
    (EvalOpVector&, std::map<int,VariableValue>& opValMap,  
     const VariableValue& v=VariableValue()) const override;
  };

  struct OperationDAGBase: public Node, public OperationType  
  {
    vector<vector<WeakNodePtr> > arguments;
    string name;
    double init;
    OperationPtr state;
    OperationDAGBase(const string& name=""): 
      name(name) {}
    virtual Type type() const=0;
    /// factory method 
    static OperationDAGBase* create(Type type, const string& name="");
    int order(unsigned maxOrder) const override;
    VariableValue addEvalOps(EvalOpVector&, std::map<int,VariableValue>& opValMap,  
                             const VariableValue&) const override;
    void checkArg(unsigned i, unsigned j) const;
  };

  template <OperationType::Type T>
  struct OperationDAG: public OperationDAGBase
  {
    Type type() const {return T;}
    OperationDAG(const string& name=""): OperationDAGBase(name) 
    {arguments.resize(OperationTypeInfo::numArguments<T>());}
    int BODMASlevel() const {
      switch (type())
        {
        case OperationType::multiply:
        case OperationType::divide:
          return 1;
        case OperationType::subtract:
        case OperationType::add:
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
    int godleyId;
    GodleyColumnDAG(): godleyId(-1) {}
    int order(unsigned maxOrder) const override {return 0;} // Godley columns define integration vars
  };

  class SubexpressionCache
  {
    std::map<std::string, NodePtr > cache;
    std::map<std::string, VariableDAGPtr> integrationInputs;
    std::map<const Node*, NodePtr> reverseLookupCache;
  public:
    std::string key(const OperationBase& x) const {
      return "op:"+str(x.ports()[0]);
    }
    std::string key(const VariableBase& x) const {
      return "var:"+x.fqName();
    }
    std::string key(const SwitchIcon& x) const {
      return "switch:"+str(x.ports()[0]);
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
    size_t size() const {return cache.size()+integrationInputs.size();}
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

    const Minsky& minsky;
    map<int, int> portToOperation;
    map<int, int> portToSwitch;

    /// create a variable DAG. returns cached value if previously called
    shared_ptr<VariableDAG> makeDAG(const string& valueId, int scope, const string& name, VariableType::Type type);
    shared_ptr<VariableDAG> makeDAG(VariableBase& v)
    {return makeDAG(v.valueId(),v.scope(),v.name(),v.type());}
    /// create an operation DAG. returns cached value if previously called
    NodePtr makeDAG(const OperationPtr& op);
    NodePtr makeDAG(const SwitchIcon& op);

    /// returns cached subexpression node representing what feeds the
    /// wire, creating a new one if necessary
    NodePtr getNodeFromWire(int wire);

    void processGodleyTable
    (map<string, GodleyColumnDAG>& godleyVariables, const GodleyTable& godley, int godleyId);

    /// applies the chain rule to expression x
    template <class Expr> NodePtr chainRule(const Expr& x, const Expr& deriv);

  public:
    /// construct the system of equations 
    SystemOfEquations(const Minsky&);
    ostream& latex(ostream&) const; ///< render as a LaTeX eqnarray
    /// Use LaTeX brqn environment to wrap long lines
    ostream& latexWrapped(ostream&) const; 
    ostream& matlab(ostream&) const; ///< render as MatLab code
    /// create equations suitable for Runge-Kutta solver
    /// @param vector of equations to be constructed
    /// @param vector of integrals to be constructed
    /// @param portValMap - map of flowVar ids assigned with an output port
    void populateEvalOpVector
    (EvalOpVector& equations, std::vector<Integral>& integrals, 
     std::map<int,VariableValue>& portValMap);

    /// symbolically differentiate \a expr
    template <class Expr> NodePtr derivative(const Expr& expr);

    /// useful constants to share
    NodePtr zero{new ConstantDAG(0)}, one{new ConstantDAG(1)};

    /// render equations into a cairo context
    void renderEquations(ecolab::cairo::Surface&) const;
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
