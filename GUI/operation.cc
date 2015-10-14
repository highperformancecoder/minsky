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
#define OPNAMEDEF
#include "operation.h"
#include "variable.h"
#include "portManager.h"
#include "minsky.h"
#include "cairoItems.h"
#include "str.h"

#include <cairo_base.h>
#include <ecolab_epilogue.h>

#include <math.h>
#include <sstream>

using namespace ecolab;

namespace
{
  inline double sqr(double x) {return x*x;}
}

namespace minsky
{

  // necessary for Classdesc reflection!
  constexpr float IntOp::intVarOffset;
  constexpr float OperationBase::l;
  constexpr float OperationBase::h;
  constexpr float OperationBase::r;

  bool OperationBase::multiWire()
  {
    switch (type())
      {
      case add: case subtract: 
      case multiply: case divide:
      case min: case max:
      case and_: case or_:
        return true;
      default:
        return false;
      }
  }

  void OperationBase::addPorts(unsigned numPorts)
  {
    m_ports.clear();
    if (numPorts>0)
      m_ports.push_back(minsky().addOutputPort());
    for (size_t i=1; i<numPorts; ++i)
      m_ports.push_back(minsky().addPort(Port(0,0,true,multiWire())));
  }


  IntOp::IntOp(const vector<int>& ports): Super(ports), intVar(-1) 
  {
    if (ports.empty())
      {
        delPorts(); // to be sure, although there shouldn't be any ports
        addPorts();
      }
  }

  const IntOp& IntOp::operator=(const IntOp& x)
  {
    Super::operator=(x); 
    intVar=-1;  // cause a new integral variable to be created
    m_description=x.m_description; 
    addPorts();
    return *this;
  }

  void IntOp::addPorts()
  {
    m_ports.clear();
    setDescription();
    m_ports.push_back(minsky().addInputPort());
  }


  void OperationBase::delPorts()
  {
    for (int i: ports())
      minsky().delPort(i);
    m_ports.clear();
  }

  void IntOp::newName()
  {
    // if conversion unsuccessful, allocate a new variable name
    int i=1;
    string trialName;
    do
      trialName=m_description+str(i++);
    while (variableManager().values.count(VariableManager::valueId(group, trialName)));
    m_description=trialName;
    if (intVar>-1)
      variableManager()[intVar]->name(m_description);
  }

  void IntOp::setDescription()
  {
    // body of this method defined in a lambda to ensure
    // makeConsistent() is called regardless of the return path.
    // makeConsistent() potentially throws, soc cannot be called from
    // a destructor
    [&]()
      {
        vector<Wire> savedWires;
        // unscoped descriptions treated as global
        if (m_description.find(":")==string::npos)
          m_description=":"+m_description;

        if (numPorts()>0)
          {
            // save any attached wires for later use
            array<int> outWires=minsky().wiresAttachedToPort(ports()[0]);
            for (array<int>::iterator i=outWires.begin(); i!=outWires.end(); ++i)
              savedWires.push_back(minsky().wires[*i]);
          }


        if (intVar > -1)
          {
            const VariablePtr& v=variableManager()[intVar];
            if (!ports().empty() && ports()[0]!=v->outPort())
              minsky().delPort(ports()[0]);
            if (v->valueId()!=VariableManager::valueId(m_description))
              {
                minsky().variables.erase(intVar, true);
                intVar=-1;
              }
            else
              return; // nothing to be done
          }
        else if (!ports().empty())
          minsky().delPort(ports()[0]);

        // set a default name if none given
        if (m_description==":") 
          {
            m_description="int";
            newName();
          }
        // if the variable name exists, and already has a connected input,
        // then it is not a candidate for being an integral variable, so
        // generate a new name that doesn't currently exist

        if (variableManager().values.count(valueId())) 
          try
            {
              variableManager().convertVarType(valueId(), VariableType::integral);
            }
          catch (...)
            {
              newName();
            }

        VariablePtr iv(VariableType::integral, m_description);
        iv->visible=false; // we're managing our own display
        iv->group=group;
        intVar=variableManager().addVariable(iv);

        // make the intVar outport the integral operator's outport
        if (ports().size()<1) m_ports.resize(1);

        m_ports[0]=iv->outPort();

        // restore any previously attached wire
        for (size_t i=0; i<savedWires.size(); ++i)
          {
            Wire& w=savedWires[i];
            w.from=ports()[0];
            if (variableManager().addWire(w.to, w.from))
              minsky().addWire(w);
          }
      }();
    minsky().variables.makeConsistent();
  }

  bool OperationBase::selfWire(int from, int to) const
  {
    bool r=false;
    if (numPorts()>1 && from==ports()[0])
      for (size_t i=1; !r && i<numPorts(); ++i) 
        r|=to==ports()[i];
    return r;
  }

  namespace
  {
    OperationFactory<OperationBase, Operation> operationFactory;
  }

  OperationBase* OperationBase::create(OperationType::Type type,
                                       const vector<int>& ports)
  {
    switch (type)
      {
      case integrate: return new IntOp(ports);
      case data: return new DataOp(ports);
      case constant: return new Constant(ports);
      default:
        auto r=operationFactory.create(type);
        if (!ports.empty())
          r->addPorts(ports);
        return r;
      }
  }

  array<int> Operations::visibleOperations() const
  {
    array<int> ret;
    for (const_iterator i=begin(); i!=end(); ++i)
      if ((*i)->visible)
        ret<<=i->id();
    return ret;
  }


  bool IntOp::toggleCoupled()
  {
    // body of this method defined in a lambda to ensure
    // makeConsistent() is called regardless of the return path.
    // makeConsistent() potentially throws, soc cannot be called from
    // a destructor
    bool r=[&]()
      {
        if (type()!=integrate) return false;
        if (intVar==-1) setDescription();

        VariablePtr v=getIntVar();
        v->toggleInPort();

        assert(m_ports.size()==2);
        if (coupled()) 
          {
            // we are coupled, decouple variable
            assert(v->inPort()>=0);
            m_ports[0]=minsky().addOutputPort();
            minsky().addWire(Wire(m_ports[0],v->inPort()));
            v->visible=true;
            v->rotation=rotation;
            float angle=rotation*M_PI/180;
            float xoffs=OperationBase::r+intVarOffset+RenderVariable(*v).width();
            v->moveTo(x()+xoffs*::cos(angle), y()+xoffs*::sin(angle));
          }
        else
          {
            assert(v->inPort()==-1);
            minsky().delPort(ports()[0]);
            m_ports[0]=v->outPort();
            v->visible=false;
          }
        return coupled();
      }();
    minsky().variables.makeConsistent();
    return r;
  }

  string OperationBase::portValues() const
  {
    string r="equations not yet constructed, please reset";
    // search the equation list for this operation
    for (EvalOpVector::const_iterator ei=minsky().equations.begin();
         ei!=minsky().equations.end(); ++ei)
      if (this==(*ei)->state.get())
        {
          const EvalOpBase& e=**ei;
          r="[out]="+str(ValueVector::flowVars[e.out]);
          if (e.numArgs()>0)
            r+=" [in1]="+ str(e.flow1? ValueVector::flowVars[e.in1]: 
                               ValueVector::stockVars[e.in1]);
          if (e.numArgs()>1)
            r+=" [in2]="+ str(e.flow2? ValueVector::flowVars[e.in2]: 
                               ValueVector::stockVars[e.in2]);
        }
    return r;
  }

  void Constant::adjustSliderBounds()
  {
    if (sliderMax<value) sliderMax=value;
    if (sliderMin>value) sliderMin=value;
  }

  void Constant::initOpSliderBounds()
  {
    if (!sliderBoundsSet) 
      {
        if (value==0)
          {
            sliderMin=-1;
            sliderMax=1;
            sliderStep=0.1;
          }
        else
          {
            sliderMin=-value*10;
            sliderMax=value*10;
            sliderStep=std::abs(0.1*value);
          }
        sliderStepRel=false;
        sliderBoundsSet=true;
      }
  }

  void DataOp::readData(const string& fileName)
  {
    ifstream f(fileName.c_str());
    data.clear();
    // for now, we just read pairs of numbers, separated by
    // whitespace. Later, we need to add in the smarts to handle a
    // variety of CSV formats
    double x, y;
    while (f>>x>>y)
      data[x]=y; // TODO: throw if more than one equal value of x provided?

    // trim any leading directory
    size_t p=fileName.rfind('/');
    // '/' is guaranteed not to be in fileName, so we can use that as
    // a delimiter
    description = "\\verb/"+
      ((p!=string::npos)? fileName.substr(p+1): fileName) + "/";
  }

  double DataOp::interpolate(double x) const
  {
    // not terribly sensible, but need to return something
    if (data.empty()) return 0;

    map<double, double>::const_iterator v=data.lower_bound(x);
    if (v==data.end())
      return data.rbegin()->second;
    else if (v==data.begin())
      return v->second;
    else if (v->first > x)
      {
        map<double, double>::const_iterator v0=v;
        --v0;
        return (x-v0->first)*(v->second-v0->second)/
          (v->first-v0->first)+v0->second;
      }
    else
      {
        assert(v->first==x);
        return v->second;
      }
  }

  double DataOp::deriv(double x) const
  {
    map<double, double>::const_iterator v=data.lower_bound(x);
    if (v==data.end() || v==data.begin())
      return 0;
    map<double, double>::const_iterator v1=v, v2=v; 
    --v1;
    if (v->first==x)
      {
        ++v2;
        if (v2==data.end()) v2=v;
        return (v2->second-v1->second)/(v2->first-v1->first);
      }
    else 
      return (v->second-v1->second)/(v->first-v1->first);
  }

}
