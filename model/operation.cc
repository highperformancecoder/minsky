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
//#include "variable.h"
//#include "portManager.h"
#include "minsky.h"
//#include "cairoItems.h"
#include "str.h"

#include <cairo_base.h>
#include <ecolab_epilogue.h>

#include <math.h>
#include <sstream>

using namespace ecolab;

namespace
{
  inline double sqr(double x) {return x*x;}

  struct DrawBinOp
  {
    cairo_t *cairo;
    DrawBinOp(cairo_t *cairo): cairo(cairo) {}

    void drawPlus() const
    {
      cairo_move_to(cairo,0,-5);
      cairo_line_to(cairo,0,5);
      cairo_move_to(cairo,-5,0);
      cairo_line_to(cairo,5,0);
      cairo_stroke(cairo);
    }

    void drawMinus() const
    {
      cairo_move_to(cairo,-5,0);
      cairo_line_to(cairo,5,0);
      cairo_stroke(cairo);
    }

    void drawMultiply() const
    {
      cairo_move_to(cairo,-5,-5);
      cairo_line_to(cairo,5,5);
      cairo_move_to(cairo,-5,5);
      cairo_line_to(cairo,5,-5);
      cairo_stroke(cairo);
    }

    void drawDivide() const
    {
      cairo_move_to(cairo,-5,0);
      cairo_line_to(cairo,5,0);
      cairo_new_sub_path(cairo);
      cairo_arc(cairo,0,3,1,0,2*M_PI);
      cairo_new_sub_path(cairo);
      cairo_arc(cairo,0,-3,1,0,2*M_PI);
      cairo_stroke(cairo);
    }

    // puts a small symbol to identify port
    // x, y = position of symbol
    void drawPort(void (DrawBinOp::*symbol)() const, float x, float y, float rotation)  const
    {
      cairo_save(cairo);
      
      double angle=rotation * M_PI / 180.0;
      double fm=std::fmod(rotation,360);
      if (!((fm>-90 && fm<90) || fm>270 || fm<-270))
        y=-y;
      cairo_rotate(cairo, angle);
      
      cairo_translate(cairo,0.7*x,0.6*y);
      cairo_scale(cairo,0.5,0.5);
      
      // and counter-rotate
      cairo_rotate(cairo, -angle);
      (this->*symbol)();
      cairo_restore(cairo);
    }
  };
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

  void OperationBase::addPorts()
  {
    ports.clear();
    if (numPorts()>0)
      ports.emplace_back(new Port(*this,Port::noFlags));
    for (size_t i=1; i<numPorts(); ++i)
      ports.emplace_back
        (new Port(*this, Port::inputPort | (multiWire()? Port::multiWire: Port::noFlags)));
  }

  const IntOp& IntOp::operator=(const IntOp& x)
  {
    Super::operator=(x); 
    intVar.reset(x.intVar->clone());
    return *this;
  }

  void IntOp::description(string desc)
  {

    // set a default name if none given
    if (desc.empty()) 
      desc=minsky().variableValues.newName("int");

    // unscoped descriptions treated as global
    if (desc.find(":")==string::npos)
      desc=":"+desc;

    desc=VariableValue::valueId(desc);

    if (intVar && intVar->valueId()==desc)
      return; // nothing to do

    vector<Wire> savedWires;
    if (numPorts()>0)
      {
        // save any attached wires for later use
        for (auto w: ports[0]->wires)
          savedWires.push_back(*w);
      }

    // if the variable name exists, and already has a connected
    // input, then it is not a candidate for being an integral
    // variable, so generate a new name that doesn't currently
    // exist

    if (minsky().variableValues.count(desc)) 
      try
        {
          minsky().convertVarType(desc, VariableType::integral);
        }
      catch (...)
        {
          desc=minsky().variableValues.newName(desc);
        }

    intVar.reset(new Variable<VariableType::integral>(desc));
    intVar->m_visible=false; // we're managing our own display
    intVar->group=group;
    // initialise in toggled state
    ports[0]=intVar->ports[0];

    // recreate any previously attached wires, initially in global group.
    for (auto& w: savedWires)
      minsky().model->addWire(new Wire(intVar->ports[0], w.to(), w.coords()));

    // this should also adjust the wire's group ownership appropriately
    if (auto g=group.lock())
      g->addItem(minsky().getNewId(), intVar);
  }

  bool OperationBase::selfWire(const shared_ptr<Port>& from, const shared_ptr<Port>& to) const
  {
    bool r=false;
    if (numPorts()>1 && from==ports[0])
      for (size_t i=1; !r && i<numPorts(); ++i) 
        r|=to==ports[i];
    return r;
  }

  namespace
  {
    OperationFactory<OperationBase, Operation> operationFactory;
  }

  OperationBase* OperationBase::create(OperationType::Type type)
  {
    switch (type)
      {
      case integrate: return new IntOp;
      case data: return new DataOp;
      case constant: return new Constant;
      default: return operationFactory.create(type);
      }
  }

  ecolab::array<int> Operations::visibleOperations() const
  {
    ecolab::array<int> ret;
    for (const_iterator i=begin(); i!=end(); ++i)
      if ((*i)->visible())
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

        assert(intVar);
        //        intVar->toggleInPort();

        assert(ports.size()==2);
        if (coupled()) 
          {
            intVar->ports.resize(2);
            intVar->ports[0].reset(new Port(*intVar,Port::noFlags));
            intVar->ports[1].reset(new Port(*intVar,Port::inputPort));
            Wire* newWire=new Wire(ports[0], intVar->ports[1]);
            if (auto g=group.lock())
              g->addWire(minsky().getNewId(), newWire);
            else
              minsky().model->addWire(minsky().getNewId(), newWire);
            intVar->m_visible=true;
            intVar->rotation=rotation;
            float angle=rotation*M_PI/180;
            //TODO       float xoffs=OperationBase::r+intVarOffset+RenderVariable(*intVar).width();
            //TODO intVar->moveTo(x()+xoffs*::cos(angle), y()+xoffs*::sin(angle));
          }
        else
          {
            intVar->ports.resize(1); // deletes wire also
            ports[0]=intVar->ports[0];
            intVar->m_visible=false;
          }
        return coupled();
      }();
    //TODO minsky().variables.makeConsistent();
    return r;
  }

  string OperationBase::portValues() const
  {
    string r="equations not yet constructed, please reset";
    // search the equation list for this operation
//    for (EvalOpVector::const_iterator ei=minsky().equations.begin();
//         ei!=minsky().equations.end(); ++ei)
//      if (this==(*ei)->state.get())
//        {
//          const EvalOpBase& e=**ei;
//          r="[out]="+str(ValueVector::flowVars[e.out]);
//          if (e.numArgs()>0)
//            r+=" [in1]="+ str(e.flow1? ValueVector::flowVars[e.in1]: 
//                               ValueVector::stockVars[e.in1]);
//          if (e.numArgs()>1)
//            r+=" [in2]="+ str(e.flow2? ValueVector::flowVars[e.in2]: 
//                               ValueVector::stockVars[e.in2]);
//        }
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

  // virtual draw methods for operations - defined here rather than
  // operations.cc because it is more related to the functionality in
  // this file.

  template <> void Operation<OperationType::constant>::iconDraw(cairo_t* cairo) const
  {
    assert(false); //shouldn't be here
  }

  template <> void Operation<OperationType::data>::iconDraw(cairo_t* cairo) const
  {
    cairo_translate(cairo,-1,0);
    cairo_scale(cairo,1.5,0.75);
    cairo_arc(cairo,0,-3,3,0,2*M_PI);
    cairo_arc(cairo,0,3,3,0,M_PI);
    cairo_move_to(cairo,-3,3);
    cairo_line_to(cairo,-3,-3);
    cairo_move_to(cairo,3,3);
    cairo_line_to(cairo,3,-3);
    cairo_identity_matrix(cairo);
    cairo_set_line_width(cairo,1);
    cairo_stroke(cairo);
  }

  template <> void Operation<OperationType::time>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"t");
  }

  template <> void Operation<OperationType::copy>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"=");
  }

  template <> void Operation<OperationType::integrate>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-7,4.5);
    cairo_show_text(cairo,"\xE2\x88\xAB");
    cairo_show_text(cairo,"dt");
  }

  template <> void Operation<OperationType::differentiate>::iconDraw(cairo_t* cairo) const
  {
    cairo_save(cairo);
    cairo_move_to(cairo,-7,-1);
    cairo_set_font_size(cairo,8);
    cairo_show_text(cairo,"d");
    cairo_move_to(cairo,-7,0);cairo_line_to(cairo,2,0);
    cairo_set_line_width(cairo,0.5);cairo_stroke(cairo);
    cairo_move_to(cairo,-7,7);
    cairo_show_text(cairo,"dt");
    cairo_restore(cairo);
  }

  template <> void Operation<OperationType::sqrt>::iconDraw(cairo_t* cairo) const
  {
    cairo_save(cairo);
    cairo_set_font_size(cairo,10);   
    cairo_move_to(cairo,-7,6);
    cairo_show_text(cairo,"\xE2\x88\x9a");
    cairo_set_line_width(cairo,0.5);
    cairo_rel_move_to(cairo,0,-9);
    cairo_rel_line_to(cairo,5,0);
    cairo_set_source_rgb(cairo,0,0,0);
    cairo_stroke(cairo);
    cairo_restore(cairo);
    //    cairo_show_text(cairo,"sqrt");
  }

  template <> void Operation<OperationType::exp>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-7,3);
    cairo_show_text(cairo,"e");
    cairo_rel_move_to(cairo,0,-4);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"x");
  }

  template <> void Operation<OperationType::pow>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"x");
    cairo_rel_move_to(cairo,0,-4);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"y");
    cairo_set_font_size(cairo,5);
    cairo_move_to(cairo, l+1, -h+6);
#ifdef DISPLAY_POW_UPSIDE_DOWN
    cairo_show_text(cairo,"y");
#else
    cairo_show_text(cairo,"x");
#endif
    cairo_move_to(cairo, l+1, h-3);
#ifdef DISPLAY_POW_UPSIDE_DOWN
    cairo_show_text(cairo,"x");
#else
    cairo_show_text(cairo,"y");
#endif
  }

  template <> void Operation<OperationType::le>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"≤");
  }

  template <> void Operation<OperationType::lt>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"<");
  }

  template <> void Operation<OperationType::eq>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"=");
  }

  template <> void Operation<OperationType::min>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"min");
  }

  template <> void Operation<OperationType::max>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"max");
  }

  template <> void Operation<OperationType::and_>::iconDraw(cairo_t* cairo) const
  {
    cairo_save(cairo);
    cairo_set_source_rgb(cairo,0,0,0);
    cairo_move_to(cairo,-4,3);
    cairo_line_to(cairo,-1,-3);
    cairo_line_to(cairo,2,3);
    cairo_restore(cairo);
  }

  template <> void Operation<OperationType::or_>::iconDraw(cairo_t* cairo) const
  {
    cairo_save(cairo);
    cairo_set_source_rgb(cairo,0,0,0);
    cairo_move_to(cairo,-4,-3);
    cairo_line_to(cairo,-1,3);
    cairo_line_to(cairo,2,-3);
    cairo_restore(cairo);
  }

  template <> void Operation<OperationType::not_>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"¬");
  }

  template <> void Operation<OperationType::ln>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo," ln");
  }

  template <> void Operation<OperationType::log>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"log");
    cairo_rel_move_to(cairo,0,3);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"b");
    cairo_set_font_size(cairo,5);
    cairo_move_to(cairo, l+1, -h+6);
    cairo_show_text(cairo,"x");
    cairo_move_to(cairo, l+1, h-3);
    cairo_show_text(cairo,"b");
  
  }

  template <> void Operation<OperationType::sin>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sin");
  }

  template <> void Operation<OperationType::cos>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cos");
  }

  template <> void Operation<OperationType::tan>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tan");
  }

  template <> void Operation<OperationType::asin>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,9);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sin");
    cairo_rel_move_to(cairo,0,-3);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"-1");
    cairo_rel_move_to(cairo,0,-2);
  }

  template <> void Operation<OperationType::acos>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,9);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cos");
    cairo_rel_move_to(cairo,0,-3);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"-1");
    cairo_rel_move_to(cairo,0,-2);
  }

  template <> void Operation<OperationType::atan>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,9);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tan");
    cairo_rel_move_to(cairo,0,-3);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"-1");
    cairo_rel_move_to(cairo,0,-2);
  }

  template <> void Operation<OperationType::sinh>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sinh");
  }

  template <> void Operation<OperationType::cosh>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cosh");
  }

  template <> void Operation<OperationType::tanh>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tanh");
  }

  template <> void Operation<OperationType::abs>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"|x|");
  }
  template <> void Operation<OperationType::add>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawPlus();
    d.drawPort(&DrawBinOp::drawPlus, l, h, rotation);
    d.drawPort(&DrawBinOp::drawPlus, l, -h, rotation);
  }

  template <> void Operation<OperationType::subtract>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawMinus();
    d.drawPort(&DrawBinOp::drawPlus, l, -h, rotation);
    d.drawPort(&DrawBinOp::drawMinus, l, h, rotation);
  }

  template <> void Operation<OperationType::multiply>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawMultiply();
    d.drawPort(&DrawBinOp::drawMultiply, l, h, rotation);
    d.drawPort(&DrawBinOp::drawMultiply, l, -h, rotation);
  }

  template <> void Operation<OperationType::divide>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawDivide();
    d.drawPort(&DrawBinOp::drawMultiply, l, -h, rotation);
    d.drawPort(&DrawBinOp::drawDivide, l, h, rotation);
  }

  template <> void Operation<OperationType::numOps>::iconDraw(cairo_t* cairo) const
  {/* needs to be here, and is actually called */}



}
