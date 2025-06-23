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
#include "minsky.h"
#include "geometry.h"
#include "operation.h"
#include "dataOp.h"
#include "userFunction.h"
#include "ravelWrap.h"
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
using ecolab::cairo::CairoSave;

namespace minsky
{
  namespace
  {
    struct SumInputPort: public MultiWireInputPort
    {
      void combineInput(double& x, double y) const override {x+=y;}
      SumInputPort(Item& item): MultiWireInputPort(item) {}    
    };

    struct MulInputPort: public MultiWireInputPort
    {
      void combineInput(double& x, double y) const override {x*=y;}
      MulInputPort(Item& item): MultiWireInputPort(item) {}    
      double identity() const override {return 1;}
    };

    struct MaxInputPort: public MultiWireInputPort
    {
      void combineInput(double& x, double y) const override {if (y>x) x=y;}
      MaxInputPort(Item& item): MultiWireInputPort(item) {}
      double identity() const override {return -std::numeric_limits<double>::max();}
    };

    struct MinInputPort: public MultiWireInputPort
    {
      void combineInput(double& x, double y) const override {if (y<x) x=y;}
      MinInputPort(Item& item): MultiWireInputPort(item) {}    
      double identity() const override {return std::numeric_limits<double>::min();}
    };
    struct AndInputPort: public MultiWireInputPort
    {
      void combineInput(double& x, double y) const override {x=(x>0.5)&&(y>0.5);}
      AndInputPort(Item& item): MultiWireInputPort(item) {}    
      double identity() const override {return 1;}
    };
    struct OrInputPort: public MultiWireInputPort
    {
      void combineInput(double& x, double y) const override {x=(x>0.5)||(y>0.5);}
      OrInputPort(Item& item): MultiWireInputPort(item) {}    
    };
  }
  
  bool OperationBase::multiWire() const
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
    m_ports.clear();
    if (numPorts()>0)
      m_ports.emplace_back(make_shared<Port>(*this));
    for (size_t i=1; i<numPorts(); ++i)
      switch (type())
        {
        case add: case subtract: 
          m_ports.emplace_back(make_shared<SumInputPort>(*this));
          break;
        case multiply: case divide:
          m_ports.emplace_back(make_shared<MulInputPort>(*this));
          break;
        case min:
          m_ports.emplace_back(make_shared<MinInputPort>(*this));
          break;
        case max:
          m_ports.emplace_back(make_shared<MaxInputPort>(*this));
          break;
        case and_:
          m_ports.emplace_back(make_shared<AndInputPort>(*this));
          break;
        case or_:
          m_ports.emplace_back(make_shared<OrInputPort>(*this));
          break;
        case meld:
        case merge:
          m_ports.emplace_back(make_shared<MultiWireInputPort>(*this));
          break;
        default:
          m_ports.emplace_back(make_shared<InputPort>(*this));
          break;
        }
  }
  
  float OperationBase::scaleFactor() const
  {
    const float z=zoomFactor();
    const float l=OperationBase::l*z, r=OperationBase::r*z, 
      h=OperationBase::h*z;
    return std::max(1.0f,std::min(0.5f*iWidth()*z/std::max(l,r),0.5f*iHeight()*z/h));  
  }  

  void OperationBase::drawUserFunction(cairo_t* cairo) const
  {
    // if rotation is in 1st or 3rd quadrant, rotate as
    // normal, otherwise flip the text so it reads L->R
    const double angle=rotation() * M_PI / 180.0;
    const bool textFlipped=flipped(rotation());
    const float z=zoomFactor();

    auto& c=dynamic_cast<const NamedOp&>(*this);
          
    Pango pango(cairo);
    pango.setFontSize(10.0*scaleFactor()*z);
    pango.setMarkup(latexToPango(c.description()));
    pango.angle=angle + (textFlipped? M_PI: 0);
    const Rotate r(rotation()+ (textFlipped? 180: 0),0,0);

    // parameters of icon in userspace (unscaled) coordinates
    float w, h, hoffs;
    w=0.5*pango.width()+2*z; 
    h=0.5*pango.height()+4*z;        
    hoffs=pango.top()/z;
    
    {
      const cairo::CairoSave cs(cairo);
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
    const Rotate rr(rotation(),0,0);

    m_ports[0]->moveTo(x()+rr.x(w+2,0), y()+rr.y(w+2,0));
    switch (numPorts())
      {
      case 1: break;
      case 2: 
        m_ports[1]->moveTo(x()+rr.x(-w,0), y()+rr.y(-w,0));
        break;
      case 3: default:
        m_ports[1]->moveTo(x()+rr.x(-w,0), y()+rr.y(-w,textFlipped? h-3: -h+3));
        m_ports[2]->moveTo(x()+rr.x(-w,0), y()+rr.y(-w,textFlipped? -h+3: h-3));
        break;
      }
    if (type()==OperationType::userFunction)
      {
        cairo_set_source_rgb(cairo,0,0,0);
        DrawBinOp drawBinOp(cairo, zoomFactor());
        drawBinOp.drawPort([&](){drawBinOp.drawSymbol("x");},-1.1*w,-1.1*h,rotation());
        drawBinOp.drawPort([&](){drawBinOp.drawSymbol("y");},-1.1*w,1.1*h,rotation());
      }
    if (mouseFocus)
      {
        drawPorts(cairo);
        displayTooltip(cairo,tooltip());
        if (onResizeHandles) drawResizeHandles(cairo);             
      }
    clipPath.appendToCurrent(cairo);
    cairo_clip(cairo);
    if (selected) drawSelected(cairo);
  }
  
  void OperationBase::setCachedText(cairo_t* cairo, const std::string& text, double size) const
  {
    if (cachedPango && cairo==cachedPango->cairoContext()) return;
    cachedPango=make_shared<Pango>(cairo);
    cachedPango->setMarkup(text);
    cachedPango->setFontSize(size);
  }

  
  void OperationBase::draw(cairo_t* cairo) const
  {
    // if rotation is in 1st or 3rd quadrant, rotate as
    // normal, otherwise flip the text so it reads L->R
    const double angle=rotation() * M_PI / 180.0;
    const bool textFlipped=flipped(rotation());
    const float z=zoomFactor();

    {
      const CairoSave cs(cairo);
      cairo_scale(cairo,z,z);
      iconDraw(cairo);
    }

        
    CairoSave cs(cairo);
    cairo_rotate(cairo, angle);
        
    float l=OperationBase::l*z, r=OperationBase::r*z, 
      h=OperationBase::h*z;
          
    if (fabs(l)<0.5*iWidth()*z) l=-0.5*iWidth()*z;        
    if (r<0.5*iWidth()*z) r=0.5*iWidth()*z;    
    if (h<0.5*iHeight()*z) h=0.5*iHeight()*z;    
        
    cairo_move_to(cairo,-r,-h);
    cairo_line_to(cairo,-r,h);
    cairo_line_to(cairo,r,h);
    cairo_line_to(cairo,r+2*z,0);
    cairo_line_to(cairo,r,-h);      
    
    cairo_close_path(cairo);		  	 
    
    cairo_set_source_rgb(cairo,0,0,1);
    cairo_stroke_preserve(cairo);
        
    cairo::Path clipPath(cairo);
    
    // compute port coordinates relative to the icon's
    // point of reference. Move outport 2 pixels right for ticket For ticket 362.
    double x0=r, y0=0, x1=l, y1=numPorts() > 2? -h+3: 0, 
      x2=l, y2=numPorts() > 2? h-3: 0;
                      
    if (textFlipped) swap(y1,y2);
    
    {
      const CairoSave cs(cairo);
      cairo_identity_matrix(cairo);
      cairo_translate(cairo, x(), y());
      cairo_rotate(cairo, angle);
      cairo_user_to_device(cairo, &x0, &y0);
      cairo_user_to_device(cairo, &x1, &y1);
      cairo_user_to_device(cairo, &x2, &y2);
    }
    
    if (numPorts()>0) 
      m_ports[0]->moveTo(x0, y0);
    if (numPorts()>1) 
      {
#ifdef DISPLAY_POW_UPSIDE_DOWN
        if (type()==OperationType::pow)
          ports[1]->moveTo(x2, y2);
        else
#endif
          m_ports[1]->moveTo(x1, y1);
      }
    
    if (numPorts()>2)
      {
#ifdef DISPLAY_POW_UPSIDE_DOWN
        if (type()==OperationType::pow)
          ports[2]->moveTo(x1, y1);
        else
#endif
          m_ports[2]->moveTo(x2, y2);
      }

    cs.restore(); // undo rotation
    if (mouseFocus)
      {
        drawPorts(cairo);
        displayTooltip(cairo,tooltip());
        if (onResizeHandles) drawResizeHandles(cairo);
      }
          
    cairo_new_path(cairo);          
    clipPath.appendToCurrent(cairo);          
    cairo_clip(cairo);          
    if (selected) drawSelected(cairo);          
  }    
  
  void OperationBase::resize(const LassoBox& b)
  {
    const float invZ=1/zoomFactor();  
    moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));
    iWidth(std::abs(b.x1-b.x0)*invZ);
    iHeight(std::abs(b.y1-b.y0)*invZ);
  }

  double OperationBase::value() const
  {
    try
      {
        unique_ptr<ScalarEvalOp> e(ScalarEvalOp::create(type(),itemPtrFromThis()));
        if (e)
          switch (e->numArgs())
            {
            case 0: return e->evaluate(0,0);
            case 1: return e->evaluate(m_ports[1]->value());
            case 2: return e->evaluate(m_ports[1]->value(),m_ports[2]->value());
            }
      }
    catch (...)
      {/* absorb exception here - some operators cannot be evaluated this way*/}
    return nan("");
  }


  vector<string> OperationBase::dimensions() const
  {
    set<string> names;
    for (size_t i=1; i<m_ports.size(); ++i)
      if (auto vv=m_ports[i]->getVariableValue())
        for (auto& xv: vv->hypercube().xvectors)
          names.insert(xv.name);
      else if (!m_ports[i]->wires().empty())
        if (auto f=m_ports[i]->wires()[0]->from())
          if (auto r=f->item().ravelCast())
            for (auto& xv: r->hypercube().xvectors)
              names.insert(xv.name);
              
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
        for (size_t i=1; i<item.portsSize(); ++i)
          for (auto w: item.ports(i).lock()->wires())
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

  Units OperationBase::unitsBinOpCase(bool check) const
  {
    switch (type())
      {
        // these binops need to have dimensionless units
      case log: case and_: case or_: case polygamma: case userFunction:

        if (check && !m_ports[1]->units(check).empty())
          throw_error("function inputs not dimensionless");
        return {};
      case pow:
        {
          auto r=m_ports[1]->units(check);

          if (!r.empty())
            {
              if (!m_ports[2]->wires().empty())
                if (auto v=dynamic_cast<VarConstant*>(&m_ports[2]->wires()[0]->from()->item()))
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
          if (!m_ports[1]->wires().empty())
            return m_ports[1]->wires()[0]->units(check);
          if (!m_ports[2]->wires().empty())
            return m_ports[2]->wires()[0]->units(check);
          return {};
        }
        // multiply and divide are especially computed
      case multiply: case divide:
        {
          Units units;
          for (auto w: m_ports[1]->wires())
            {
              auto tmp=w->units(check);
              for (auto& i: tmp)
                units[i.first]+=i.second;
            }
          const int f=(type()==multiply)? 1: -1; //indices are negated for division
          for (auto w: m_ports[2]->wires())
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
  }
  
  Units OperationBase::units(bool check) const
  {
    // default operations are dimensionless, but check that inputs are also
    switch (classify(type()))
      {
      case function: case reduction: case scan: case tensor:
        {
          if (check && !m_ports[1]->units(check).empty())
            throw_error("function input not dimensionless");
          return {};
        }
      case constop:
        // Add % sign to units from input to % operator. Need the first conditional otherwise Minsky crashes		
        if (type()==percent && !m_ports[1]->wires().empty()) {
          auto r=m_ports[1]->units(check);	 	 
          if (auto vV=dynamic_cast<VariableValue*>(&m_ports[1]->wires()[0]->from()->item())) 
            {    
              vV->setUnits("%"+r.str());
              vV->units.normalise();
              return vV->units; 
            }
          return r; 
        }
        return {};
      case binop:
        return unitsBinOpCase(check);
      case statistics:
        switch (type())
          {
          case mean: case median: case stdDev:
            return m_ports[1]->units(check);
          case moment:
            {
              auto argUnits=m_ports[1]->units(check);
              for (auto& i: argUnits)
                i.second*=arg;
              return argUnits;
            }
          case histogram:
            return {};
          case covariance:
            {
              auto argUnits=m_ports[1]->units(check);
              for (auto& i: m_ports[2]->units(check))
                {
                  argUnits.emplace(i.first,1); // ensure base unit is present
                  argUnits[i.first]*=i.second;
                }
              return argUnits;
            }
          case linearRegression:
            return m_ports[1]->units(check);
          case correlation:
            return {};
          default:
            throw_error("Statistics operation does not have units() defined");
          }
      default:
        if (check)
          throw_error("Operation<"+OperationType::typeName(type())+">::units() should be overridden");
        return {};
      }
  }

  Units Time::units(bool) const {return cminsky().timeUnit;}
  Units Derivative::units(bool check) const {
    Units r=m_ports[1]->units(check);
    if (!cminsky().timeUnit.empty())
      r[cminsky().timeUnit]--;
    r.normalise();
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
      case time: return new Time;
      case copy: return new Copy;
      case integrate: return new IntOp;
      case differentiate: return new Derivative;
      case data: return new DataOp;
      case ravel: return new Ravel;
      case constant: throw error("Constant deprecated");
      case userFunction: return new UserFunction;
      default: return operationFactory.create(type);
      }
  }

  string OperationBase::portValues() const
  {
    string r="equations not yet constructed, please reset";
    if (!m_ports.empty())
      r="[out]="+str(m_ports[0]->value());
    if (m_ports.size()>1)
      r+=" [in1]="+ str(m_ports[1]->value());
    if (m_ports.size()>2)
      r+=" [in2]="+ str(m_ports[2]->value());
    return r;
  }
  
  string NamedOp::description() const
  {
    return m_description;  
  }
   
  string NamedOp::description(const std::string& x)
  {
    m_description=x;
    updateBB(); // adjust icon bounding box - see ticket #1121
    return m_description;
  }    

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
    const double sf = scaleFactor();  
    cairo_translate(cairo,-1,0);
    cairo_scale(cairo,1.5*sf,0.75*sf);
    cairo_arc(cairo,0,-3,3,0,2*M_PI);
    cairo_arc(cairo,0,3,3,0,M_PI);
    cairo_move_to(cairo,-3,3);
    cairo_line_to(cairo,-3,-3);
    cairo_move_to(cairo,3,3);
    cairo_line_to(cairo,3,-3);
    cairo_identity_matrix(cairo);
    cairo_set_line_width(cairo,1.0);  
    cairo_stroke(cairo); 
  }

  template <> void Operation<OperationType::time>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor();  
    cairo_scale(cairo,sf,sf);	  	  
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"t");
  }
  
  template <> void Operation<OperationType::euler>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor();  
    cairo_scale(cairo,sf,sf);		  
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"e");
  }
  
  template <> void Operation<OperationType::pi>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor();  
    cairo_scale(cairo,sf,sf);		  
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"π");
  }           
   
  template <> void Operation<OperationType::zero>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor();  
    cairo_scale(cairo,sf,sf);		  
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"0");
  }
  
  template <> void Operation<OperationType::one>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor();  
    cairo_scale(cairo,sf,sf);		  
    cairo_move_to(cairo,-4,2);
    cairo_show_text(cairo,"1");
  }
  
  template <> void Operation<OperationType::inf>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor();  	  
    cairo_move_to(cairo,-4,-10);
    setCachedText(cairo,"∞",9);
    cairo_scale(cairo,sf,sf);		  
    cachedPango->show();    
  }

  template <> void Operation<OperationType::percent>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-4,-7);
    setCachedText(cairo,"%",7);
    cairo_scale(cairo,sf,sf);		  
    cachedPango->show();
  }   

  template <> void Operation<OperationType::copy>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor();  	  	  
    cairo_move_to(cairo,-4,-5);
    setCachedText(cairo, "→",7);
    cairo_scale(cairo,sf,sf);		  
    cachedPango->show();
  }

  template <> void Operation<OperationType::integrate>::iconDraw(cairo_t* cairo) const
  {/* moved to IntOp::draw() but needs to be here, and is actually called */}
  
  template <> void Operation<OperationType::differentiate>::iconDraw(cairo_t* cairo) const
  { 
    const CairoSave cs(cairo);
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_move_to(cairo,-7,-1);
    cairo_set_font_size(cairo,8);
    cairo_show_text(cairo,"d");
    cairo_move_to(cairo,-7,0);cairo_line_to(cairo,2,0);
    cairo_set_line_width(cairo,0.5);cairo_stroke(cairo);
    cairo_move_to(cairo,-7,7);
    cairo_show_text(cairo,"dt");
  }

  template <> void Operation<OperationType::sqrt>::iconDraw(cairo_t* cairo) const
  {	  
    const CairoSave cs(cairo);
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,10);   
    cairo_move_to(cairo,-7,6);
    cairo_show_text(cairo,"\xE2\x88\x9a");
    cairo_set_line_width(cairo,0.5);
    cairo_rel_move_to(cairo,0,-9);
    cairo_rel_line_to(cairo,5,0);
    cairo_set_source_rgb(cairo,0,0,0);
    cairo_stroke(cairo);    
  }

  template <> void Operation<OperationType::exp>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	    
    cairo_move_to(cairo,-7,3);
    cairo_show_text(cairo,"e");
    cairo_rel_move_to(cairo,0,-4);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"x");
  }

  template <> void Operation<OperationType::pow>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  	  
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"x");
    cairo_rel_move_to(cairo,0,-4);
    cairo_set_font_size(cairo,7);
    cairo_show_text(cairo,"y");
    DrawBinOp d(cairo);
#ifdef DISPLAY_POW_UPSIDE_DOWN
    d.drawPort([&](){d.drawSymbol("y");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("x");}, l, h, rotation());
#else
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("y");}, l, h, rotation());
#endif
  }

  template <> void Operation<OperationType::le>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	 	  
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"x≤y");
    DrawBinOp d(cairo);
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("y");}, l, h, rotation());
  }

  template <> void Operation<OperationType::lt>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	 	  
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"x<y");
    DrawBinOp d(cairo);
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("y");}, l, h, rotation());
  }

  template <> void Operation<OperationType::eq>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	 	  
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"x=y");
    DrawBinOp d(cairo);
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("y");}, l, h, rotation());
  }

  template <> void Operation<OperationType::min>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	   
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"min");
  }

  template <> void Operation<OperationType::max>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"max");
  }

  template <> void Operation<OperationType::and_>::iconDraw(cairo_t* cairo) const
  {	  
    const CairoSave cs(cairo);
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	   
    cairo_set_source_rgb(cairo,0,0,0);
    cairo_move_to(cairo,-4,3);
    cairo_line_to(cairo,-1,-3);
    cairo_line_to(cairo,2,3);
    cairo_stroke(cairo);
  }

  template <> void Operation<OperationType::or_>::iconDraw(cairo_t* cairo) const
  {  
    const CairoSave cs(cairo);
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	 
    cairo_set_source_rgb(cairo,0,0,0);
    cairo_move_to(cairo,-4,-3);
    cairo_line_to(cairo,-1,3);
    cairo_line_to(cairo,2,-3);
    cairo_stroke(cairo);
  }

  template <> void Operation<OperationType::not_>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"¬");
  }
  
  template <> void Operation<OperationType::covariance>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf*.7,sf); 
    cairo_move_to(cairo,-16,3);
    cairo_show_text(cairo,"<ΔxΔy>");
  }
  
  template <> void Operation<OperationType::correlation>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 
    cairo_move_to(cairo,-3,3);
    cairo_show_text(cairo,"ρ");
  }

  template <> void Operation<OperationType::linearRegression>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);
    cairo_move_to(cairo,-7,7);
    cairo_line_to(cairo,7,-7);
    cairo_stroke(cairo);
    cairo_arc(cairo,-4,0,0.2,0,2*M_PI);
    cairo_stroke(cairo);
    cairo_arc(cairo,3,3,0.2,0,2*M_PI);
    cairo_stroke(cairo);
    cairo_arc(cairo,4,-6,0.2,0,2*M_PI);
    cairo_stroke(cairo);
  }
  template <> void Operation<OperationType::ln>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo," ln");
  }

  template <> void Operation<OperationType::log>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"log");
    DrawBinOp d(cairo);
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("b");}, l, h, rotation());
  }

  template <> void Operation<OperationType::sin>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sin");
  }

  template <> void Operation<OperationType::cos>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cos");
  }

  template <> void Operation<OperationType::tan>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tan");
  }

  template <> void Operation<OperationType::asin>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 
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
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);  
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
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);  
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
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sinh");
  }

  template <> void Operation<OperationType::cosh>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);  
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"cosh");
  }

  template <> void Operation<OperationType::tanh>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"tanh");
  }

  template <> void Operation<OperationType::abs>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,9);
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"|x|");
  }
  template <> void Operation<OperationType::floor>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-5,-5);
    // what we're trying to draw, but Windows' deficient fontsets don't allow it
    //setCachedText(cairo, "⌊x⌋",7);
    setCachedText(cairo, "x",7);
    cairo_scale(cairo,sf,sf);	  
    cachedPango->show();
    cairo_move_to(cairo,-5,-4);
    cairo_rel_line_to(cairo,0,cachedPango->height()-2);
    cairo_rel_line_to(cairo,1,0);
    cairo_move_to(cairo,-5+cachedPango->width(),-4);
    cairo_rel_line_to(cairo,0,cachedPango->height()-2);
    cairo_rel_line_to(cairo,-1,0);
    cairo_stroke(cairo);
  }
  template <> void Operation<OperationType::frac>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"frac");
  }
  template <> void Operation<OperationType::Gamma>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 	  
    cairo_move_to(cairo,-6,3);
    cairo_show_text(cairo,"Γ");
  }     
  template <> void Operation<OperationType::polygamma>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 	  
    cairo_move_to(cairo,-7,3);
    cairo_show_text(cairo,"ψ");
    cairo_rel_move_to(cairo,0,-3);
    cairo_set_font_size(cairo,7);
    // show order of polygamma function. 0 is default.
    const std::string order="("+to_string(static_cast<unsigned>(m_ports[2]->value()))+")";
    cairo_show_text(cairo,order.c_str());
    cairo_rel_move_to(cairo,0,-2);
    DrawBinOp d(cairo);
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("n");}, l, h, rotation());
  }     
  template <> void Operation<OperationType::fact>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 	  
    cairo_move_to(cairo,-3,3);
    cairo_show_text(cairo,"!");
  }      
  template <> void Operation<OperationType::add>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 	  
    DrawBinOp d(cairo);
    d.drawPlus();
    d.drawPort([&](){d.drawPlus();}, l, h, rotation());
    d.drawPort([&](){d.drawPlus();}, l, -h, rotation());
  }

  template <> void Operation<OperationType::subtract>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    DrawBinOp d(cairo);
    d.drawMinus();
    d.drawPort([&](){d.drawPlus();}, l, -h, rotation());
    d.drawPort([&](){d.drawMinus();}, l, h, rotation());
  }

  template <> void Operation<OperationType::multiply>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);  
    DrawBinOp d(cairo);
    d.drawMultiply();
    d.drawPort([&](){d.drawMultiply();}, l, h, rotation());
    d.drawPort([&](){d.drawMultiply();}, l, -h, rotation());
  }

  template <> void Operation<OperationType::divide>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    DrawBinOp d(cairo);
    d.drawDivide();
    d.drawPort([&](){d.drawMultiply();}, l, -h, rotation());
    d.drawPort([&](){d.drawDivide();}, l, h, rotation());
  }

  template <> void Operation<OperationType::sum>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-4,-7);
    setCachedText(cairo, "∑", 7);
    cairo_scale(cairo,sf,sf);		  
    cachedPango->show();
  }

  template <> void Operation<OperationType::product>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-4,-7);
    setCachedText(cairo, "∏",7);
    cairo_scale(cairo,sf,sf);		  
    cachedPango->show();
  }

  template <> void Operation<OperationType::infimum>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"inf");
  }

  template <> void Operation<OperationType::supremum>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"sup");
  }
  
  template <> void Operation<OperationType::infIndex>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"infi");
  }

  template <> void Operation<OperationType::supIndex>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"supi");
  }

  template <> void Operation<OperationType::any>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf); 
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"any");
  }

  template <> void Operation<OperationType::all>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"all");
  }

  
  
   template <> void Operation<OperationType::size>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"nᵢ");
  }

  template <> void Operation<OperationType::shape>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"{nᵢ}");
  }

  template <> void Operation<OperationType::mean>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-8,3);
    cairo_show_text(cairo,"<x>");
  }

  template <> void Operation<OperationType::median>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-3,3);
    cairo_show_text(cairo,"x");
    cairo_move_to(cairo,-4,-1);
    cairo_show_text(cairo,"~");
 }

  template <> void Operation<OperationType::stdDev>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);	  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-3,3);
    cairo_show_text(cairo,"σ");
  }

  template <> void Operation<OperationType::moment>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf*.85,sf);	  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-12,3);
    cairo_show_text(cairo,"<Δxᵏ>");
  }

  template <> void Operation<OperationType::histogram>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_translate(cairo,-0.5*iWidth(),-0.5*iHeight());
    cminsky().histogramResource.render(cairo,iWidth(),iHeight());
  }

  

  template <> void Operation<OperationType::runningSum>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-7,-7);
    setCachedText(cairo, "∑+",7);
    cairo_scale(cairo,sf,sf);	  
    cachedPango->show();
  }

  template <> void Operation<OperationType::runningProduct>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-6,-7);
    setCachedText(cairo, "∏×",7);
    cairo_scale(cairo,sf,sf);	  
    cachedPango->show();
  }

  template <> void Operation<OperationType::difference>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-4,-7);
    setCachedText(cairo, "Δ<sup>-</sup>",7);
    cairo_scale(cairo,sf,sf);	  
    cachedPango->show();
  }

  template <> void Operation<OperationType::differencePlus>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-4,-7);
    setCachedText(cairo, "Δ<sup>+</sup>",7);
    cairo_scale(cairo,sf,sf);	  
    cachedPango->show();
  }

  template <> void Operation<OperationType::innerProduct>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-4,-10);
    setCachedText(cairo, "·",14);
    cairo_scale(cairo,sf,sf);	  
    cachedPango->show();
  }

  template <> void Operation<OperationType::outerProduct>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-4,-10);
    // this is the character we want, but draw it explicitly because
    //of Windows' deficient fontsets.
    // setCachedText(cairo, "⊗",10);
    cairo_scale(cairo,sf,sf);	  
    constexpr const double r=6;
    static const double d=0.5*r*std::sqrt(2);
    cairo_move_to(cairo,d,d);
    cairo_line_to(cairo,-d,-d);
    cairo_move_to(cairo,-d,d);
    cairo_line_to(cairo,d,-d);
    cairo_move_to(cairo,r,0);
    cairo_arc(cairo,0,0,r,0,2*M_PI);
    cairo_stroke(cairo);
  }

  template <> void Operation<OperationType::index>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);  
    cairo_set_font_size(cairo,10);
    cairo_move_to(cairo,-9,3);
    cairo_show_text(cairo,"idx");
  }

  template <> void Operation<OperationType::gather>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_scale(cairo,sf,sf);
    cairo_set_font_size(cairo,8);
    cairo_move_to(cairo,-7,3);
    cairo_show_text(cairo,"x[i]");
    DrawBinOp drawBinOp(cairo);
    drawBinOp.drawPort([&](){drawBinOp.drawSymbol("x");},l,-h,rotation());
    drawBinOp.drawPort([&](){drawBinOp.drawSymbol("i");},l,h,rotation());
  }

  template <> void Operation<OperationType::meld>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-4,-5);
    //setCachedText(cairo, "⭄",10);
    cairo_scale(cairo,sf,sf);  
    cairo_rel_line_to(cairo,4,0);
    cairo_rel_line_to(cairo,2,5);
    cairo_rel_line_to(cairo,-2,5);
    cairo_rel_line_to(cairo,-4,0);
    cairo_move_to(cairo,-4,0);
    cairo_rel_line_to(cairo,10,0);
    cairo_stroke(cairo);
  }

  template <> void Operation<OperationType::merge>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-4,-3);
    //setCachedText(cairo, "⫤",10);
    cairo_scale(cairo,sf,sf);  
    cairo_rel_line_to(cairo,8,0);
    cairo_move_to(cairo,-4,3);
    cairo_rel_line_to(cairo,8,0);
    cairo_rel_move_to(cairo,0,-10);
    cairo_rel_line_to(cairo,0,14);
    cairo_stroke(cairo);
  }

  template <> void Operation<OperationType::slice>::iconDraw(cairo_t* cairo) const
  {
    const double sf = scaleFactor(); 	     
    cairo_move_to(cairo,-10,-10);
    setCachedText(cairo, "[...",10);
    cairo_scale(cairo,sf,sf);  
    cachedPango->show();
    cairo_move_to(cairo,-10+cachedPango->width(),-9);
    cairo_rel_line_to(cairo,0,cachedPango->height()-2);
    cairo_stroke(cairo);
  }

  

  template <> void Operation<OperationType::numOps>::iconDraw(cairo_t* cairo) const
  {/* needs to be here, and is actually called */}

}

