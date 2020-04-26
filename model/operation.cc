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
#include "geometry.h"
#define OPNAMEDEF
#include "operation.h"
#include "ravelWrap.h"
#include "minsky.h"
#include "str.h"
#include "cairoItems.h"

#include <cairo_base.h>
#include <pango.h>
#include "minsky_epilogue.h"

#include <math.h>
#include <sstream>

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

using namespace ecolab;

namespace
{
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
  
  ClickType::Type OperationBase::clickType(float xx, float yy)
  {
    double fm=std::fmod(rotation(),360);
    bool notflipped=(fm>-90 && fm<90) || fm>270 || fm<-270;
    Rotate r(rotation()+(notflipped? 0: 180),0,0); // rotate into variable's frame of reference
    double z=zoomFactor();
    double dx=xx-x(), dy=yy-y();
    if (fabs(fabs(dx)-iWidth()) < portRadius*z &&
        fabs(fabs(dy)-iHeight()) < portRadius*z &&
        fabs(hypot(dx,dy)-hypot(iWidth(),iHeight())) < portRadius*z)
      return ClickType::onResize;
    return Item::clickType(xx,yy);
  }
  
  float OperationBase::scaleFactor() const
  {
	float z=zoomFactor();
    float l=OperationBase::l*z, r=OperationBase::r*z, 
      h=OperationBase::h*z;	  
    return std::max(1.0,std::min(iWidth()*z/(std::max(l,r)),iHeight()*z/h));  
  }  

  void OperationBase::draw(cairo_t* cairo) const
  {
    // if rotation is in 1st or 3rd quadrant, rotate as
    // normal, otherwise flip the text so it reads L->R
    double angle=rotation() * M_PI / 180.0;
    double fm=std::fmod(rotation(),360);
    bool textFlipped=!((fm>-90 && fm<90) || fm>270 || fm<-270);
    double coupledIntTranslation=0;
    float z=zoomFactor();

    auto t=type();
    // call the iconDraw method if data description is empty
    if (t==OperationType::data && dynamic_cast<const DataOp&>(*this).description().empty())
      t=OperationType::numOps;

    switch (t)
      {
        // at the moment its too tricky to get all the information
        // together for rendering constants
      case OperationType::data:
        {
        
          auto& c=dynamic_cast<const DataOp&>(*this);
          
          Pango pango(cairo);
          pango.setFontSize(10*scaleFactor()*z);
          pango.setMarkup(latexToPango(c.description()));
          pango.angle=angle + (textFlipped? M_PI: 0);
          Rotate r(rotation()+ (textFlipped? 180: 0),0,0);

          // parameters of icon in userspace (unscaled) coordinates
          float w, h, hoffs;
          w=0.5*pango.width()+2*z; 
          h=0.5*pango.height()+4*z;        
          hoffs=pango.top()/z;
    
          {
            cairo::CairoSave cs(cairo);
            cairo_move_to(cairo,r.x(-w+1,-h-hoffs+2*z), r.y(-w+1,-h-hoffs+2*z));
            pango.show();
          }

          cairo_rotate(cairo, angle);
               
          cairo_set_source_rgb(cairo,0,0,1);
          cairo_move_to(cairo,-w,-h);
          cairo_line_to(cairo,-w,h);
          cairo_line_to(cairo,w,h);

          cairo_line_to(cairo,w+2*z,0);
          cairo_line_to(cairo,w,-h);
          cairo_close_path(cairo);
          cairo::Path clipPath(cairo);
          cairo_stroke(cairo);
          
          cairo_rotate(cairo,-angle); // undo rotation

          // set the output ports coordinates
          // compute port coordinates relative to the icon's
          // point of reference
          Rotate rr(rotation(),0,0);

          ports[0]->moveTo(x()+rr.x(w+2,0), y()+rr.y(w+2,0));
          if (numPorts()>1)
            ports[1]->moveTo(x()+rr.x(-w,0), y()+rr.y(-w,0));
          if (mouseFocus)
            {
              drawPorts(cairo);
              displayTooltip(cairo,tooltip);
              if (onResizeHandles) drawResizeHandles(cairo);             
            }
          clipPath.appendToCurrent(cairo);
          cairo_clip(cairo);
          if (selected) drawSelected(cairo);
          return;
        }
      case OperationType::integrate:
        if (const IntOp* i=dynamic_cast<const IntOp*>(this))
          if (i->coupled())
            {
              auto& iv=*i->intVar;
              //            iv.zoomFactor=zoomFactor;
              RenderVariable rv(iv,cairo);
              // we need to add some translation if the variable is bound
              cairo_rotate(cairo,rotation()*M_PI/180.0);
              coupledIntTranslation=-0.5*(i->intVarOffset+2*rv.width()+2+r)*z;
              if (rv.width()<iv.iWidth()) coupledIntTranslation=-0.5*(i->intVarOffset+2*iv.iWidth()+2+r)*z;
              //            cairo_translate(cairo, coupledIntTranslation, 0);
              cairo_rotate(cairo,-rotation()*M_PI/180.0);
            }
        cairo_save(cairo);
        cairo_scale(cairo,z,z);
        iconDraw(cairo);
        cairo_restore(cairo);
        break;
      default:
        cairo_save(cairo);
        cairo_scale(cairo,z,z);
        iconDraw(cairo);
        cairo_restore(cairo);
        break;
      }

    float l=OperationBase::l*z, r=OperationBase::r*z, 
      h=OperationBase::h*z;
      
    if (fabs(l)<iWidth()*z) l=-iWidth()*z;        
    if (r<iWidth()*z) r=iWidth()*z;    
    if (h<iHeight()*z) h=iHeight()*z;
      
    int intVarWidth=0;
    cairo_save(cairo);
    cairo_rotate(cairo, angle);
    // Operation icons now have the same shape as constants/parameters. Coupled integrate operation retains previous shape. For ticket 362.
    if (const IntOp* i=dynamic_cast<const IntOp*>(this)) {
      if (i->coupled()) {		  
         cairo_move_to(cairo,l,h);
         cairo_line_to(cairo,l,-h);
         cairo_line_to(cairo,r,0);    
	   }      
	 } else {
         cairo_move_to(cairo,-r,-h);
         cairo_line_to(cairo,-r,h);
         cairo_line_to(cairo,r,h);
         cairo_line_to(cairo,r+2*z,0);
         cairo_line_to(cairo,r,-h);        
	 }
     cairo_close_path(cairo);		  	 

    //  cairo_save(cairo);
    cairo_set_source_rgb(cairo,0,0,1);
    cairo_stroke_preserve(cairo);
    //  cairo_restore(cairo);

    if (const IntOp* i=dynamic_cast<const IntOp*>(this))
      if (i->coupled())
        {
          float ivo=i->intVarOffset*z;
          cairo_new_path(cairo);
          cairo_move_to(cairo,r,0);
          cairo_line_to(cairo,r+ivo,0);
          cairo_set_source_rgb(cairo,0,0,0);
          cairo_stroke(cairo);
        
          VariablePtr intVar=i->intVar;
          // display an integration variable next to it
          RenderVariable rv(*intVar, cairo);
          // save the render width for later use in setting the clip
          intVarWidth=rv.width()*z;
          if (rv.width()<intVar->iWidth()) intVarWidth=intVar->iWidth()*z;
          // set the port location...
          intVar->moveTo(i->x()+r+ivo+intVarWidth, i->y());
            
          cairo_save(cairo);
          cairo_translate(cairo,r+ivo+intVarWidth,0);
          // to get text to render correctly, we need to set
          // the var's rotation, then antirotate it
          i->intVar->rotation(i->rotation());
          cairo_rotate(cairo, -M_PI*i->rotation()/180.0);
          rv.draw();
          //i->getIntVar()->draw(cairo);
          cairo_restore(cairo);

          // build clip path the hard way grr...
          cairo_move_to(cairo,l,h);
          cairo_line_to(cairo,l,-h);
          cairo_line_to(cairo,r,0);
          cairo_line_to(cairo,r+ivo,0);
          float rvw=rv.width()*z, rvh=rv.height()*z;
          if (rv.width()<intVar->iWidth()) rvw=intVar->iWidth()*z;
          if (rv.height()<intVar->iHeight()) rvh=intVar->iHeight()*z;
          cairo_line_to(cairo,r+ivo,-rvh);
          cairo_line_to(cairo,r+ivo+2*rvw,-rvh);
          cairo_line_to(cairo,r+ivo+2*rvw+2*z,0);
          cairo_line_to(cairo,r+ivo+2*rvw,rvh);
          cairo_line_to(cairo,r+ivo,rvh);
          cairo_line_to(cairo,r+ivo,0);
          cairo_line_to(cairo,r,0);
          cairo_close_path(cairo);
        }

    cairo::Path clipPath(cairo);

    // compute port coordinates relative to the icon's
    // point of reference. Move out port 2 pixels right for ticket For ticket 362.
    double x0=r, y0=0, x1=l, y1=numPorts() > 2? -h+3: 0, 
      x2=l, y2=numPorts() > 2? h-3: 0;
                  
    if (textFlipped) swap(y1,y2);

    // adjust for integration variable
    if (const IntOp* i=dynamic_cast<const IntOp*>(this))
      if (i->coupled())
        x0+=i->intVarOffset+2*intVarWidth+2;

    cairo_save(cairo);
    cairo_identity_matrix(cairo);
    cairo_translate(cairo, x(), y());
    //cairo_scale(cairo,zoomFactor,zoomFactor);
    cairo_rotate(cairo, angle);
    cairo_user_to_device(cairo, &x0, &y0);
    cairo_user_to_device(cairo, &x1, &y1);
    cairo_user_to_device(cairo, &x2, &y2);
    cairo_restore(cairo);

    if (numPorts()>0) 
      ports[0]->moveTo(x0, y0);
    if (numPorts()>1) 
      {
#ifdef DISPLAY_POW_UPSIDE_DOWN
        if (type()==OperationType::pow)
          ports[1]->moveTo(x2, y2);
        else
#endif
          ports[1]->moveTo(x1, y1);
      }

    if (numPorts()>2)
      {
#ifdef DISPLAY_POW_UPSIDE_DOWN
        if (type()==OperationType::pow)
          ports[2]->moveTo(x1, y1);
        else
#endif
          ports[2]->moveTo(x2, y2);
      }

    cairo_translate(cairo,-coupledIntTranslation,0);

    cairo_restore(cairo); // undo rotation
    if (mouseFocus)
      {
        drawPorts(cairo);
        displayTooltip(cairo,tooltip);
        if (onResizeHandles) drawResizeHandles(cairo);
      }

    cairo_new_path(cairo);
    clipPath.appendToCurrent(cairo);
    cairo_clip(cairo);
    if (selected) drawSelected(cairo);
  }
  
  void OperationBase::resize(const LassoBox& b)
  {
    float invZ=1/zoomFactor();  
    moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));
    iWidth(std::abs(b.x1-b.x0)*invZ);
    iHeight(std::abs(b.y1-b.y0)*invZ);
  }
  

  double OperationBase::value() const
  {
    try
      {
        unique_ptr<ScalarEvalOp> e(ScalarEvalOp::create(type()));
        if (e)
          switch (e->numArgs())
            {
            case 0: return e->evaluate(0,0);
            case 1: return e->evaluate(ports[1]->value());
            case 2: return e->evaluate(ports[1]->value(),ports[2]->value());
            }
      }
    catch (...)
      {/* absorb exception here - some operators cannot be evaluated this way*/}
    return nan("");
  }


  vector<string> OperationBase::dimensions() const
  {
    set<string> names;
    for (size_t i=1; i<ports.size(); ++i)
      {
        auto& vv=ports[i]->getVariableValue();
        for (auto& i: vv.hypercube().xvectors)
          names.insert(i.name);
      }
    return {names.begin(), names.end()};
  }

  namespace {
    // return fractional part of x
    inline double fracPart(double x) {
      double dummy;
      return modf(x,&dummy);
    }

    // extract units from inputs, checking they're all consistent
    struct CheckConsistent: public Units
    {
      CheckConsistent(const Item& item)
      {
        bool inputFound=false;
        for (size_t i=1; i<item.ports.size(); ++i)
          for (auto w: item.ports[i]->wires())
            if (inputFound)
              {
                auto tmp=w->units(true);
                if (tmp!=*this)
                  item.throw_error("incompatible units: "+tmp.str()+"≠"+str());
              }
            else
              {
                inputFound=true;
                Units::operator=(w->units(true));
              }
      }
    };
  }

  Units OperationBase::units(bool check) const
  {
    // default operations are dimensionless, but check that inputs are also
    switch (classify(type()))
      {
      case function: case reduction: case scan: case tensor:
        if (check && !ports[1]->units(check).empty())
          throw_error("function input not dimensionless");
        return {};
      case constop:
        return {};        
      case binop:
        switch (type())
          {
            // these binops need to have dimensionless units
          case log: case and_: case or_:

            if (check && !ports[1]->units(check).empty())
              throw_error("function inputs not dimensionless");
            return {};
          case pow:
            {
              auto r=ports[1]->units(check);

              if (!r.empty())
                {
                  if (!ports[2]->wires().empty())
                    if (auto v=dynamic_cast<VarConstant*>(&ports[2]->wires()[0]->from()->item()))
                      if (fracPart(v->value())==0)
                        {
                          for (auto& i: r) i.second*=v->value();
                          r.normalise();
                          return r;
                        }
                  if (check)
                    throw_error("dimensioned pow only possible if exponent is a constant integer");
                }
              return r;
            }
            // these binops must have compatible units
          case le: case lt: case eq:
            {
              if (check)
                CheckConsistent(*this);
              return {};
            }
          case add: case subtract: case max: case min:
            {
              if (check)
                return CheckConsistent(*this);
              if (!ports[1]->wires().empty())
                return ports[1]->wires()[0]->units(check);
              if (!ports[2]->wires().empty())
                return ports[2]->wires()[0]->units(check);
              return {};
            }
            // multiply and divide are especially computed
          case multiply: case divide:
            {
              Units units;
              for (auto w: ports[1]->wires())
                {
                  auto tmp=w->units(check);
                  for (auto& i: tmp)
                    units[i.first]+=i.second;
                }
              int f=(type()==multiply)? 1: -1; //indices are negated for division
              for (auto w: ports[2]->wires())
                {
                  auto tmp=w->units(check);
                  for (auto& i: tmp)
                    units[i.first]+=f*i.second;
                }
              units.normalise();
              return units;
            }
          default:
            if (check)
              throw_error("Operation<"+OperationType::typeName(type())+">::units() should be overridden");
            return {};
          }
      default:
        if (check)
          throw_error("Operation<"+OperationType::typeName(type())+">::units() should be overridden");
        return {};
      }
  }

  Units Time::units(bool) const {return cminsky().timeUnit;}
  Units Derivative::units(bool check) const {
    Units r=ports[1]->units(check);
    if (!cminsky().timeUnit.empty())
      r[cminsky().timeUnit]--;
    r.normalise();
    return r;
  }

  IntOpAccessor::IntOpAccessor(): ecolab::TCLAccessor<IntOp, std::string>
    ("description",(Getter)&IntOp::description,(Setter)&IntOp::description) {}
  
  Units IntOp::units(bool check) const {
    Units r=ports[1]->units(check);
    if (!cminsky().timeUnit.empty())
      r[cminsky().timeUnit]++;
    r.normalise();
    return r;
  }

  void IntOp::insertControlled(Selection& selection)
  {
    selection.ensureItemInserted(intVar);
  }
  
  const IntOp& IntOp::operator=(const IntOp& x)
  {
    Super::operator=(x); 
    intVar.reset(x.intVar->clone());
    return *this;
  }

  void IntOp::removeControlledItems() const
  {
    if (intVar)
      if (auto g=group.lock())
        g->removeItem(*intVar);
  }

  
  string IntOp::description(const string& a_desc)
  {
    auto desc=a_desc;
    
    // set a default name if none given
    if (desc.empty())
      desc=minsky().variableValues.newName
        (VariableValue::valueId(group.lock(),"int"));

    // disallow global integration variables
    if (desc[0]==':') desc=desc.substr(1);
    
    if (intVar && intVar->group.lock() == group.lock() && intVar->name()==desc)
      return description(); // nothing to do

    vector<Wire> savedWires;
    if (numPorts()>0)
      {
        // save any attached wires for later use
        for (auto w: ports[0]->wires())
          savedWires.push_back(*w);
      }

    // if the variable name exists, and already has a connected
    // input, then it is not a candidate for being an integral
    // variable, so generate a new name that doesn't currently
    // exist

    string vid=VariableValue::valueId(group.lock(),desc);
    auto i=minsky().variableValues.find(vid);      
    if (i!=minsky().variableValues.end()) 
      {
        if (i->second.type()!=VariableType::integral) 
          try
            {
              minsky().convertVarType(vid, VariableType::integral);
            }
          catch (...)
            {
              desc=minsky().variableValues.newName(vid);
            }
        else
          if (minsky().definingVar(vid))               // Also check that integral has input. for ticket 1068.
            desc=minsky().variableValues.newName(vid);
          else 
            desc=vid; 
        if (desc[0]==':') desc=desc.substr(1);// disallow global integration variables
      }
    
    
    ItemPtr oldIvar;
    if (intVar)
      oldIvar=minsky().model->removeItem(*intVar);

    intVar.reset(new Variable<VariableType::integral>(desc));
    if (auto g=group.lock())
      intVar->controller=g->findItem(*this); // we're managing our own display
    // initialise in toggled state
    ports[0]=intVar->ports[0];

    // recreate any previously attached wires, initially in global group.
    for (auto& w: savedWires)
      minsky().model->addWire(new Wire(intVar->ports[0], w.to(), w.coords()));

    bb.update(*this); // adjust icon bounding box - see ticket #704
    
    // this should also adjust the wire's group ownership appropriately
    if (auto g=group.lock())
      g->addItem(intVar);
    return description();
  }

  namespace
  {
    OperationFactory<OperationBase, Operation> operationFactory;
  }

  OperationBase* OperationBase::create(OperationType::Type type)
  {
    switch (type)
      {
      case time: return new Time;
      case copy: return new Copy;
      case integrate: return new IntOp;
      case differentiate: return new Derivative;
      case data: return new DataOp;
      case ravel: return new Ravel;
      case constant: throw error("Constant deprecated");
      default: return operationFactory.create(type);
      }
  }

  bool IntOp::toggleCoupled()
  {
    if (type()!=integrate) return false;

    assert(intVar);
    //        intVar->toggleInPort();

    assert(ports.size()==3);
    if (coupled()) 
      {
        intVar->ports[1].reset(new Port(*intVar,Port::inputPort));
        ports[0].reset(new Port(*this,Port::noFlags));
        WirePtr newWire(new Wire(ports[0], intVar->ports[1]));
        if (auto g=group.lock())
          g->addWire(newWire);
        else
          minsky().model->addWire(newWire);
        intVar->controller.reset();
        intVar->rotation(rotation());
     }
    else
      {
        // need to explicitly remove wire, as deleting the port is
        // not sufficient - wires hold a reference to the ports
        // they connect
        if (auto g=group.lock())
          {
            for (auto w: intVar->ports[1]->wires())
              g->removeWire(*w);
            intVar->controller=g->findItem(*this);
          }
        ports[0]=intVar->ports[0];
        intVar->mouseFocus=false; // prevent drawing of variable ports when coupled
      }
    bb.update(*this); // adjust bounding box for coupled integral operation - see ticket #1055  
    return coupled();
  }

  string OperationBase::portValues() const
  {
    string r="equations not yet constructed, please reset";
    if (ports.size()>0)
      r="[out]="+to_string(ports[0]->value());
    if (ports.size()>1)
      r+=" [in1]="+ to_string(ports[1]->value());
    if (ports.size()>2)
      r+=" [in2]="+ to_string(ports[2]->value());
    return r;
  }
  
  string DataOp::description() const
  {
	return m_description;  
  }
   
  string DataOp::description(const std::string& x)
  {
    m_description=x;
    bb.update(*this); // adjust icon bounding box - see ticket #1121
    return m_description;
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
    description("\\verb/"+
      ((p!=string::npos)? fileName.substr(p+1): fileName) + "/");
    //initXVector();
  }

  void DataOp::initRandom(double xmin, double xmax, unsigned numSamples)
  {
    data.clear();
    double dx=(xmax-xmin)/numSamples;
    for (double x=xmin; x<xmax; x+=dx) //NOLINT
      data[x]=double(rand())/RAND_MAX;
    //initXVector();
  }

//  void DataOp::initXVector()
//  {
//    xVector.clear();
//    xVector.emplace_back("x");
//    for (auto& i: data)
//      xVector[0].emplace_back(i.first,to_string(i.first));
//  }
  
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

//  void DataOp::initOutputVariableValue(VariableValue& v) const
//  {
//    v.xVector=xVector;
//    auto iy=v.begin();
//    for (auto& j: xVector[0])
//      *iy++=interpolate(j.first);
//  }

  // virtual draw methods for operations - defined here rather than
  // operations.cc because it is more related to the functionality in
  // this file.

  template <> void Operation<OperationType::constant>::iconDraw(cairo_t* cairo) const
  {
    assert(false); //shouldn't be here
  }

  template <> void Operation<OperationType::ravel>::iconDraw(cairo_t* cairo) const
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
  
  template <> void Operation<OperationType::euler>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"e");
  }
   
    template <> void Operation<OperationType::pi>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"π");
  }    

  template <> void Operation<OperationType::copy>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,-5);
    Pango pango(cairo);
    pango.setFontSize(7*scaleFactor()*zoomFactor());
    pango.setMarkup("→");
    pango.show();
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
    cairo_set_font_size(cairo,8*scaleFactor());
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
    cairo_set_font_size(cairo,10*scaleFactor());   
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
    cairo_set_font_size(cairo,7*scaleFactor());
    cairo_show_text(cairo,"x");
  }

  template <> void Operation<OperationType::pow>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"x");
    cairo_rel_move_to(cairo,0,-4);
    cairo_set_font_size(cairo,7*scaleFactor());
    cairo_show_text(cairo,"y");
    cairo_set_font_size(cairo,5*scaleFactor());
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
    cairo_stroke(cairo);
    cairo_restore(cairo);
  }

  template <> void Operation<OperationType::or_>::iconDraw(cairo_t* cairo) const
  {
    cairo_save(cairo);
    cairo_set_source_rgb(cairo,0,0,0);
    cairo_move_to(cairo,-4,-3);
    cairo_line_to(cairo,-1,3);
    cairo_line_to(cairo,2,-3);
    cairo_stroke(cairo);
    cairo_restore(cairo);
  }

  template <> void Operation<OperationType::not_>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"¬");
  }

  template <> void Operation<OperationType::ln>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo," ln");
  }

  template <> void Operation<OperationType::log>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"log");
    cairo_rel_move_to(cairo,0,3);
    cairo_set_font_size(cairo,7*scaleFactor());
    cairo_show_text(cairo,"b");
    cairo_set_font_size(cairo,5*scaleFactor());
    cairo_move_to(cairo, l+1, -h+6);
    cairo_show_text(cairo,"x");
    cairo_move_to(cairo, l+1, h-3);
    cairo_show_text(cairo,"b");
  
  }

  template <> void Operation<OperationType::sin>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sin");
  }

  template <> void Operation<OperationType::cos>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cos");
  }

  template <> void Operation<OperationType::tan>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tan");
  }

  template <> void Operation<OperationType::asin>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,9*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sin");
    cairo_rel_move_to(cairo,0,-3);
    cairo_set_font_size(cairo,7*scaleFactor());
    cairo_show_text(cairo,"-1");
    cairo_rel_move_to(cairo,0,-2);
  }

  template <> void Operation<OperationType::acos>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,9*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cos");
    cairo_rel_move_to(cairo,0,-3);
    cairo_set_font_size(cairo,7*scaleFactor());
    cairo_show_text(cairo,"-1");
    cairo_rel_move_to(cairo,0,-2);
  }

  template <> void Operation<OperationType::atan>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,9*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tan");
    cairo_rel_move_to(cairo,0,-3);
    cairo_set_font_size(cairo,7*scaleFactor());
    cairo_show_text(cairo,"-1");
    cairo_rel_move_to(cairo,0,-2);
  }

  template <> void Operation<OperationType::sinh>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sinh");
  }

  template <> void Operation<OperationType::cosh>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cosh");
  }

  template <> void Operation<OperationType::tanh>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tanh");
  }

  template <> void Operation<OperationType::abs>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,9*scaleFactor());
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"|x|");
  }
  template <> void Operation<OperationType::floor>::iconDraw(cairo_t* cairo) const
  {
//    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-7,-7);
    Pango pango(cairo);
    pango.setFontSize(7*scaleFactor()*zoomFactor());
    pango.setMarkup("⌊x⌋");
    pango.show();
  }
  template <> void Operation<OperationType::frac>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"frac");
  }
  template <> void Operation<OperationType::add>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawPlus();
    d.drawPort(&DrawBinOp::drawPlus, l, h, rotation());
    d.drawPort(&DrawBinOp::drawPlus, l, -h, rotation());
  }

  template <> void Operation<OperationType::subtract>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawMinus();
    d.drawPort(&DrawBinOp::drawPlus, l, -h, rotation());
    d.drawPort(&DrawBinOp::drawMinus, l, h, rotation());
  }

  template <> void Operation<OperationType::multiply>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawMultiply();
    d.drawPort(&DrawBinOp::drawMultiply, l, h, rotation());
    d.drawPort(&DrawBinOp::drawMultiply, l, -h, rotation());
  }

  template <> void Operation<OperationType::divide>::iconDraw(cairo_t* cairo) const
  {
    DrawBinOp d(cairo);
    d.drawDivide();
    d.drawPort(&DrawBinOp::drawMultiply, l, -h, rotation());
    d.drawPort(&DrawBinOp::drawDivide, l, h, rotation());
  }

  template <> void Operation<OperationType::sum>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,-7);
    Pango pango(cairo);
    pango.setFontSize(7*scaleFactor()*zoomFactor());
    pango.setMarkup("∑");
    pango.show();
  }

  template <> void Operation<OperationType::product>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,-7);
    Pango pango(cairo);
    pango.setFontSize(7*scaleFactor()*zoomFactor());
    pango.setMarkup("∏");
    pango.show();
  }

  template <> void Operation<OperationType::infimum>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"inf");
  }

 template <> void Operation<OperationType::supremum>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sup");
  }
  
  template <> void Operation<OperationType::infIndex>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"infi");
  }

 template <> void Operation<OperationType::supIndex>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"supi");
  }

  template <> void Operation<OperationType::any>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"any");
  }

  template <> void Operation<OperationType::all>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"all");
  }

 template <> void Operation<OperationType::runningSum>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-7,-7);
    Pango pango(cairo);
    pango.setFontSize(7*scaleFactor()*zoomFactor());
    pango.setMarkup("∑+");
    pango.show();
  }

 template <> void Operation<OperationType::runningProduct>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-6,-7);
    Pango pango(cairo);
    pango.setFontSize(7*scaleFactor()*zoomFactor());
    pango.setMarkup("∏×");
    pango.show();
  }

 template <> void Operation<OperationType::difference>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,-7);
    Pango pango(cairo);
    pango.setFontSize(7*scaleFactor()*zoomFactor());
    pango.setMarkup("Δ");
    pango.show();
  }

  template <> void Operation<OperationType::innerProduct>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,-10);
    Pango pango(cairo);
    pango.setFontSize(14*scaleFactor()*zoomFactor());
    pango.setMarkup("·");
    pango.show();
  }

  template <> void Operation<OperationType::outerProduct>::iconDraw(cairo_t* cairo) const
  {
    cairo_move_to(cairo,-4,-10);
    Pango pango(cairo);
    pango.setFontSize(10*scaleFactor()*zoomFactor());
    pango.setMarkup("⊗");
    pango.show();
  }

  template <> void Operation<OperationType::index>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,10*scaleFactor());
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"idx");
  }

  template <> void Operation<OperationType::gather>::iconDraw(cairo_t* cairo) const
  {
    cairo_set_font_size(cairo,8*scaleFactor());
    cairo_move_to(cairo,-7,3);
    cairo_show_text(cairo,"x[i]");
    cairo_set_font_size(cairo,5*scaleFactor());
    cairo_move_to(cairo, l+1, -h+6);
    cairo_show_text(cairo,"x");
    cairo_move_to(cairo, l+1, h-3);
    cairo_show_text(cairo,"i");
  }

 

  template <> void Operation<OperationType::numOps>::iconDraw(cairo_t* cairo) const
  {/* needs to be here, and is actually called */}

  void IntOp::pack(pack_t& x, const string& d) const
  {::pack(x,d,*this);}
      
  void IntOp::unpack(unpack_t& x, const string& d)
  {::unpack(x,d,*this);}

  void DataOp::pack(pack_t& x, const string& d) const
  {::pack(x,d,*this);}
      
  void DataOp::unpack(unpack_t& x, const string& d)
  {::unpack(x,d,*this);}
}
