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
#include "../engine/cairoShimCairo.h"
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

//  void OperationBase::drawUserFunction(cairo_t* cairo) const
//  {
//    // if rotation is in 1st or 3rd quadrant, rotate as
//    // normal, otherwise flip the text so it reads L->R
//    const double angle=rotation() * M_PI / 180.0;
//    const bool textFlipped=flipped(rotation());
//    const float z=zoomFactor();
//
//    auto& c=dynamic_cast<const NamedOp&>(*this);
//          
//    Pango pango(cairo);
//    pango.setFontSize(10.0*scaleFactor()*z);
//    pango.setMarkup(latexToPango(c.description()));
//    pango.angle=angle + (textFlipped? M_PI: 0);
//    const Rotate r(rotation()+ (textFlipped? 180: 0),0,0);
//
//    // parameters of icon in userspace (unscaled) coordinates
//    float w, h, hoffs;
//    w=0.5*pango.width()+2*z; 
//    h=0.5*pango.height()+4*z;        
//    hoffs=pango.top()/z;
//    
//    {
//      const cairo::CairoSave cs(cairo);
//      cairo_move_to(cairo,r.x(-w+1,-h-hoffs+2*z), r.y(-w+1,-h-hoffs+2*z));
//      pango.show();
//    }
//
//    cairo_rotate(cairo, angle);
//               
//    cairo_set_source_rgb(cairo,0,0,1);
//    cairo_move_to(cairo,-w,-h);
//    cairo_line_to(cairo,-w,h);
//    cairo_line_to(cairo,w,h);
//
//    cairo_line_to(cairo,w+2*z,0);
//    cairo_line_to(cairo,w,-h);
//    cairo_close_path(cairo);
//    cairo::Path clipPath(cairo);
//    cairo_stroke(cairo);
//          
//    cairo_rotate(cairo,-angle); // undo rotation
//
//    // set the output ports coordinates
//    // compute port coordinates relative to the icon's
//    // point of reference
//    const Rotate rr(rotation(),0,0);
//
//    m_ports[0]->moveTo(x()+rr.x(w+2,0), y()+rr.y(w+2,0));
//    switch (numPorts())
//      {
//      case 1: break;
//      case 2: 
//        m_ports[1]->moveTo(x()+rr.x(-w,0), y()+rr.y(-w,0));
//        break;
//      case 3: default:
//        m_ports[1]->moveTo(x()+rr.x(-w,0), y()+rr.y(-w,textFlipped? h-3: -h+3));
//        m_ports[2]->moveTo(x()+rr.x(-w,0), y()+rr.y(-w,textFlipped? -h+3: h-3));
//        break;
//      }
//    if (type()==OperationType::userFunction)
//      {
//        cairo_set_source_rgb(cairo,0,0,0);
//        DrawBinOp drawBinOp(cairo, zoomFactor());
//        drawBinOp.drawPort([&](){drawBinOp.drawSymbol("x");},-1.1*w,-1.1*h,rotation());
//        drawBinOp.drawPort([&](){drawBinOp.drawSymbol("y");},-1.1*w,1.1*h,rotation());
//      }
//    if (mouseFocus)
//      {
//        drawPorts(cairo);
//        displayTooltip(cairo,tooltip());
//        if (onResizeHandles) drawResizeHandles(cairo);             
//      }
//    clipPath.appendToCurrent(cairo);
//    cairo_clip(cairo);
//    if (selected) drawSelected(cairo);
//  }
  
  void OperationBase::drawUserFunction(const ICairoShim& cairoShim) const
  {
    // if rotation is in 1st or 3rd quadrant, rotate as
    // normal, otherwise flip the text so it reads L->R
    const double angle=rotation() * M_PI / 180.0;
    const bool textFlipped=flipped(rotation());
    const float z=zoomFactor();

    auto& c=dynamic_cast<const NamedOp&>(*this);
          
    auto& pango = cairoShim.pango();
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
      cairoShim.save();
      cairoShim.moveTo(r.x(-w+1,-h-hoffs+2*z), r.y(-w+1,-h-hoffs+2*z));
      pango.show();
      cairoShim.restore();
    }

    cairoShim.rotate(angle);
               
    cairoShim.setSourceRGB(0,0,1);
    cairoShim.moveTo(-w,-h);
    cairoShim.lineTo(-w,h);
    cairoShim.lineTo(w,h);

    cairoShim.lineTo(w+2*z,0);
    cairoShim.lineTo(w,-h);
    cairoShim.closePath();
    cairoShim.save(); // Save the clip path shape
    cairoShim.stroke();
    cairoShim.restore();
          
    cairoShim.rotate(-angle); // undo rotation

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
        cairoShim.setSourceRGB(0,0,0);
        DrawBinOpShim drawBinOp(cairoShim, zoomFactor());
        drawBinOp.drawPort([&](){drawBinOp.drawSymbol("x");},-1.1*w,-1.1*h,rotation());
        drawBinOp.drawPort([&](){drawBinOp.drawSymbol("y");},-1.1*w,1.1*h,rotation());
      }
    if (mouseFocus)
      {
        drawPorts(cairoShim);
        displayTooltip(cairoShim,tooltip());
        if (onResizeHandles) drawResizeHandles(cairoShim);             
      }
    // Re-create the clip path
    cairoShim.rotate(angle);
    cairoShim.moveTo(-w,-h);
    cairoShim.lineTo(-w,h);
    cairoShim.lineTo(w,h);
    cairoShim.lineTo(w+2*z,0);
    cairoShim.lineTo(w,-h);
    cairoShim.closePath();
    cairoShim.clip();
    cairoShim.rotate(-angle);
    if (selected) drawSelected(cairoShim);
  }
  
  void OperationBase::setCachedText(cairo_t* cairo, const std::string& text, double size) const
  {
    if (cachedPango && cairo==cachedPango->cairoContext()) return;
    cachedPango=make_shared<Pango>(cairo);
    cachedPango->setMarkup(text);
    cachedPango->setFontSize(size);
  }

  void OperationBase::draw(const ICairoShim& cairoShim) const
  {
    // if rotation is in 1st or 3rd quadrant, rotate as
    // normal, otherwise flip the text so it reads L->R
    const double angle=rotation() * M_PI / 180.0;
    const bool textFlipped=flipped(rotation());
    const float z=zoomFactor();

    {
      cairoShim.save();
      cairoShim.scale(z,z);
      iconDraw(cairoShim);
      cairoShim.restore();
    }

        
    cairoShim.save();
    cairoShim.rotate(angle);
        
    float l=OperationBase::l*z, r=OperationBase::r*z, 
      h=OperationBase::h*z;
          
    if (fabs(l)<0.5*iWidth()*z) l=-0.5*iWidth()*z;        
    if (r<0.5*iWidth()*z) r=0.5*iWidth()*z;    
    if (h<0.5*iHeight()*z) h=0.5*iHeight()*z;    
        
    cairoShim.moveTo(-r,-h);
    cairoShim.lineTo(-r,h);
    cairoShim.lineTo(r,h);
    cairoShim.lineTo(r+2*z,0);
    cairoShim.lineTo(r,-h);      
    
    cairoShim.closePath();		  	 
    
    cairoShim.setSourceRGB(0,0,1);
    cairoShim.strokePreserve();

    auto& shimImpl = dynamic_cast<const CairoShimCairo&>(cairoShim);
    cairo::Path clipPath(shimImpl._internalGetCairoContext());

    // compute port coordinates relative to the icon's
    // point of reference. Move outport 2 pixels right for ticket For ticket 362.
    double x0=r, y0=0, x1=l, y1=numPorts() > 2? -h+3: 0, 
      x2=l, y2=numPorts() > 2? h-3: 0;
                      
    if (textFlipped) swap(y1,y2);
    
    {
      cairoShim.save();
      cairoShim.identityMatrix();
      cairoShim.translate(x(), y());
      cairoShim.rotate(angle);
      cairoShim.userToDevice(x0, y0);
      cairoShim.userToDevice(x1, y1);
      cairoShim.userToDevice(x2, y2);
      cairoShim.restore();
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

    cairoShim.restore(); // undo rotation
    if (mouseFocus)
      {
        drawPorts(cairoShim);
        displayTooltip(cairoShim,tooltip());
        if (onResizeHandles) drawResizeHandles(cairoShim);
      }
          
    cairoShim.newPath();
    clipPath.appendToCurrent(shimImpl._internalGetCairoContext());
    cairoShim.clip();          
    if (selected) drawSelected(cairoShim);          
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
          case bulkLinearRegression:
            if (check)
              {
                // second port needs to be dimensionless, otherwise dimensions are mixed
                if (m_ports[2])
                  {
                    if (!m_ports[2]->units(check).empty())
                      throw_error("X port not dimensionless");
                  }
                else
                  // TODO - how can we check the X-Vector of Y is dimensionless?
                  ;
              }
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

  template <> void Operation<OperationType::constant>::iconDraw(const ICairoShim& cairoShim) const
  {
    assert(false); //shouldn't be here
  }


  template <> void Operation<OperationType::ravel>::iconDraw(const ICairoShim& cairoShim) const
  {
    assert(false); //shouldn't be here
  }


  template <> void Operation<OperationType::data>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.translate(-1,0);
    cairoShim.scale(1.5*sf,0.75*sf);
    cairoShim.arc(0,-3,3,0,2*M_PI);
    cairoShim.arc(0,3,3,0,M_PI);
    cairoShim.moveTo(-3,3);
    cairoShim.lineTo(-3,-3);
    cairoShim.moveTo(3,3);
    cairoShim.lineTo(3,-3);
    cairoShim.identityMatrix();
    cairoShim.setLineWidth(1.0);
    cairoShim.stroke();
  }


  template <> void Operation<OperationType::time>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-4,2);
    cairoShim.showText("t");
  }


  template <> void Operation<OperationType::euler>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-4,2);
    cairoShim.showText("e");
  }

  template <> void Operation<OperationType::pi>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-4,2);
    cairoShim.showText("π");
  }

  template <> void Operation<OperationType::zero>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-4,2);
    cairoShim.showText("0");
  }

  template <> void Operation<OperationType::one>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-4,2);
    cairoShim.showText("1");
  }

  template <> void Operation<OperationType::inf>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("∞");
    pango.setFontSize(9);
    cairoShim.moveTo(-4,-10);
    cairoShim.scale(sf,sf);
    pango.show();
  }


  template <> void Operation<OperationType::percent>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("%");
    pango.setFontSize(7);
    cairoShim.moveTo(-4,-7);
    cairoShim.scale(sf,sf);
    pango.show();
  }

  template <> void Operation<OperationType::copy>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("→");
    pango.setFontSize(7);
    cairoShim.moveTo(-4,-5);
    cairoShim.scale(sf,sf);
    pango.show();
  }

  template <> void Operation<OperationType::integrate>::iconDraw(const ICairoShim& cairoShim) const
  {/* moved to IntOp::draw() but needs to be here, and is actually called */}

  template <> void Operation<OperationType::differentiate>::iconDraw(const ICairoShim& cairoShim) const
  {
    cairoShim.save();
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-7,-1);
    cairoShim.setFontSize(8);
    cairoShim.showText("d");
    cairoShim.moveTo(-7,0); cairoShim.lineTo(2,0);
    cairoShim.setLineWidth(0.5); cairoShim.stroke();
    cairoShim.moveTo(-7,7);
    cairoShim.showText("dt");
    cairoShim.restore();
  }

  template <> void Operation<OperationType::sqrt>::iconDraw(const ICairoShim& cairoShim) const
  {
    cairoShim.save();
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-7,6);
    cairoShim.showText("\xE2\x88\x9a");
    cairoShim.setLineWidth(0.5);
    cairoShim.relMoveTo(0,-9);
    cairoShim.relLineTo(5,0);
    cairoShim.setSourceRGB(0,0,0);
    cairoShim.stroke();
    cairoShim.restore();
  }

  template <> void Operation<OperationType::exp>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-7,3);
    cairoShim.showText("e");
    cairoShim.relMoveTo(0,-4);
    cairoShim.setFontSize(7);
    cairoShim.showText("x");
  }


  template <> void Operation<OperationType::pow>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-6,3);
    cairoShim.showText("x");
    cairoShim.relMoveTo(0,-4);
    cairoShim.setFontSize(7);
    cairoShim.showText("y");
    DrawBinOpShim d(cairoShim);
#ifdef DISPLAY_POW_UPSIDE_DOWN
    d.drawPort([&](){d.drawSymbol("y");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("x");}, l, h, rotation());
#else
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("y");}, l, h, rotation());
#endif
  }

  template <> void Operation<OperationType::le>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("x≤y");
    DrawBinOpShim d(cairoShim);
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("y");}, l, h, rotation());
  }

  template <> void Operation<OperationType::lt>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("x<y");
    DrawBinOpShim d(cairoShim);
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("y");}, l, h, rotation());
  }

  template <> void Operation<OperationType::eq>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("x=y");
    DrawBinOpShim d(cairoShim);
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("y");}, l, h, rotation());
  }

  template <> void Operation<OperationType::min>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("min");
  }

  template <> void Operation<OperationType::max>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("max");
  }

  template <> void Operation<OperationType::and_>::iconDraw(const ICairoShim& cairoShim) const
  {
    cairoShim.save();
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setSourceRGB(0,0,0);
    cairoShim.moveTo(-4,3);
    cairoShim.lineTo(-1,-3);
    cairoShim.lineTo(2,3);
    cairoShim.stroke();
    cairoShim.restore();
  }

  template <> void Operation<OperationType::or_>::iconDraw(const ICairoShim& cairoShim) const
  {
    cairoShim.save();
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setSourceRGB(0,0,0);
    cairoShim.moveTo(-4,-3);
    cairoShim.lineTo(-1,3);
    cairoShim.lineTo(2,-3);
    cairoShim.stroke();
    cairoShim.restore();
  }

  template <> void Operation<OperationType::not_>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-6,3);
    cairoShim.showText("¬");
  }
  template <> void Operation<OperationType::covariance>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf*.7,sf);
    cairoShim.moveTo(-16,3);
    cairoShim.showText("<ΔxΔy>");
  }

  template <> void Operation<OperationType::correlation>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-3,3);
    cairoShim.showText("ρ");
  }

  template <> void Operation<OperationType::linearRegression>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-7,7);
    cairoShim.lineTo(7,-7);
    cairoShim.stroke();
    cairoShim.arc(-4,0,0.2,0,2*M_PI);
    cairoShim.stroke();
    cairoShim.arc(3,3,0.2,0,2*M_PI);
    cairoShim.stroke();
    cairoShim.arc(4,-6,0.2,0,2*M_PI);
    cairoShim.stroke();
  }

  template <> void Operation<OperationType::bulkLinearRegression>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor(); 	     
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-6,6);
    cairoShim.lineTo(6,-6);
    cairoShim.stroke();
    cairoShim.arc(-4,0,0.2,0,2*M_PI);
    cairoShim.stroke();
    cairoShim.arc(3,3,0.2,0,2*M_PI);
    cairoShim.stroke();
    cairoShim.arc(4,-6,0.2,0,2*M_PI);
    cairoShim.stroke();
    cairoShim.moveTo(-6,-7.5);
    cairoShim.lineTo(-7.5,-7.5);
    cairoShim.lineTo(-7.5,7.5);
    cairoShim.lineTo(-6,7.5);
    cairoShim.stroke();
    cairoShim.moveTo(6,-7.5);
    cairoShim.lineTo(7.5,-7.5);
    cairoShim.lineTo(7.5,7.5);
    cairoShim.lineTo(6,7.5);
    cairoShim.stroke();
  }
  template <> void Operation<OperationType::ln>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-9,3);
    cairoShim.showText(" ln");
  }

  template <> void Operation<OperationType::log>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("log");
    DrawBinOpShim d(cairoShim);
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("b");}, l, h, rotation());
  }

  template <> void Operation<OperationType::sin>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("sin");
  }

  template <> void Operation<OperationType::cos>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("cos");
  }

  template <> void Operation<OperationType::tan>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("tan");
  }

  template <> void Operation<OperationType::asin>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(9);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("sin");
    cairoShim.relMoveTo(0,-3);
    cairoShim.setFontSize(7);
    cairoShim.showText("-1");
    cairoShim.relMoveTo(0,-2);
  }

  template <> void Operation<OperationType::acos>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(9);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("cos");
    cairoShim.relMoveTo(0,-3);
    cairoShim.setFontSize(7);
    cairoShim.showText("-1");
    cairoShim.relMoveTo(0,-2);
  }

  template <> void Operation<OperationType::atan>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(9);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("tan");
    cairoShim.relMoveTo(0,-3);
    cairoShim.setFontSize(7);
    cairoShim.showText("-1");
    cairoShim.relMoveTo(0,-2);
  }

  template <> void Operation<OperationType::sinh>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(8);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("sinh");
  }

  template <> void Operation<OperationType::cosh>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(8);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("cosh");
  }

  template <> void Operation<OperationType::tanh>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(8);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("tanh");
  }

  template <> void Operation<OperationType::abs>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(9);
    cairoShim.moveTo(-6,3);
    cairoShim.showText("|x|");
  }

  template <> void Operation<OperationType::floor>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("x");
    pango.setFontSize(7);
    cairoShim.moveTo(-5,-5);
    cairoShim.scale(sf,sf);
    pango.show();
    cairoShim.moveTo(-5,-4);
    cairoShim.relLineTo(0,pango.height()-2);
    cairoShim.relLineTo(1,0);
    cairoShim.moveTo(-5+pango.width(),-4);
    cairoShim.relLineTo(0,pango.height()-2);
    cairoShim.relLineTo(-1,0);
    cairoShim.stroke();
  }

  template <> void Operation<OperationType::frac>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(8);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("frac");
  }

  template <> void Operation<OperationType::Gamma>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-6,3);
    cairoShim.showText("Γ");
  }

  template <> void Operation<OperationType::polygamma>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-7,3);
    cairoShim.showText("ψ");
    cairoShim.relMoveTo(0,-3);
    cairoShim.setFontSize(7);
    const std::string order="("+to_string(static_cast<unsigned>(m_ports[2]->value()))+")";
    cairoShim.showText(order);
    cairoShim.relMoveTo(0,-2);
    DrawBinOpShim d(cairoShim);
    d.drawPort([&](){d.drawSymbol("x");}, l, -h, rotation());
    d.drawPort([&](){d.drawSymbol("n");}, l, h, rotation());
  }

  template <> void Operation<OperationType::fact>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.moveTo(-3,3);
    cairoShim.showText("!");
  }

  template <> void Operation<OperationType::add>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    DrawBinOpShim d(cairoShim);
    d.drawPlus();
    d.drawPort([&](){d.drawPlus();}, l, h, rotation());
    d.drawPort([&](){d.drawPlus();}, l, -h, rotation());
  }


  template <> void Operation<OperationType::subtract>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    DrawBinOpShim d(cairoShim);
    d.drawMinus();
    d.drawPort([&](){d.drawPlus();}, l, -h, rotation());
    d.drawPort([&](){d.drawMinus();}, l, h, rotation());
  }


  template <> void Operation<OperationType::multiply>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    DrawBinOpShim d(cairoShim);
    d.drawMultiply();
    d.drawPort([&](){d.drawMultiply();}, l, h, rotation());
    d.drawPort([&](){d.drawMultiply();}, l, -h, rotation());
  }


  template <> void Operation<OperationType::divide>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    DrawBinOpShim d(cairoShim);
    d.drawDivide();
    d.drawPort([&](){d.drawMultiply();}, l, -h, rotation());
    d.drawPort([&](){d.drawDivide();}, l, h, rotation());
  }

  template <> void Operation<OperationType::sum>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("∑");
    pango.setFontSize(7);
    cairoShim.moveTo(-4,-7);
    cairoShim.scale(sf,sf);
    pango.show();
  }

  template <> void Operation<OperationType::product>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("∏");
    pango.setFontSize(7);
    cairoShim.moveTo(-4,-7);
    cairoShim.scale(sf,sf);
    pango.show();
  }

  template <> void Operation<OperationType::infimum>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("inf");
  }

  template <> void Operation<OperationType::supremum>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("sup");
  }

  template <> void Operation<OperationType::infIndex>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("infi");
  }

  template <> void Operation<OperationType::supIndex>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("supi");
  }

  template <> void Operation<OperationType::any>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("any");
  }

  template <> void Operation<OperationType::all>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("all");
  }

  template <> void Operation<OperationType::size>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("nᵢ");
  }

  template <> void Operation<OperationType::shape>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("{nᵢ}");
  }

  template <> void Operation<OperationType::mean>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-8,3);
    cairoShim.showText("<x>");
  }

  template <> void Operation<OperationType::median>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-3,3);
    cairoShim.showText("x");
    cairoShim.moveTo(-4,-1);
    cairoShim.showText("~");
  }

  template <> void Operation<OperationType::stdDev>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-3,3);
    cairoShim.showText("σ");
  }

  template <> void Operation<OperationType::moment>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf*.85,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-12,3);
    cairoShim.showText("<Δxᵏ>");
  }

  template <> void Operation<OperationType::histogram>::iconDraw(const ICairoShim& cairoShim) const
  {
    cairoShim.translate(-0.5*iWidth(),-0.5*iHeight());
    cairoShim.renderSVG(cminsky().histogramResource, iWidth(), iHeight());
  }

  template <> void Operation<OperationType::runningSum>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("∑+");
    pango.setFontSize(7);
    cairoShim.moveTo(-7,-7);
    cairoShim.scale(sf,sf);
    pango.show();
  }

  template <> void Operation<OperationType::runningAv>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("av+");
    pango.setFontSize(7);
    cairoShim.moveTo(-7,-7);
    cairoShim.scale(sf,sf);
    pango.show();
  }

  template <> void Operation<OperationType::runningProduct>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("∏×");
    pango.setFontSize(7);
    cairoShim.moveTo(-6,-7);
    cairoShim.scale(sf,sf);
    pango.show();
  }

  template <> void Operation<OperationType::difference>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("Δ<sup>-</sup>");
    pango.setFontSize(7);
    cairoShim.moveTo(-4,-7);
    cairoShim.scale(sf,sf);
    pango.show();
  }

  template <> void Operation<OperationType::differencePlus>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("Δ<sup>+</sup>");
    pango.setFontSize(7);
    cairoShim.moveTo(-4,-7);
    cairoShim.scale(sf,sf);
    pango.show();
  }

  template <> void Operation<OperationType::innerProduct>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("·");
    pango.setFontSize(14);
    cairoShim.moveTo(-4,-10);
    cairoShim.scale(sf,sf);
    pango.show();
  }

  template <> void Operation<OperationType::outerProduct>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    constexpr const double r=6;
    static const double d=0.5*r*std::sqrt(2);
    cairoShim.moveTo(d,d);
    cairoShim.lineTo(-d,-d);
    cairoShim.moveTo(-d,d);
    cairoShim.lineTo(d,-d);
    cairoShim.moveTo(r,0);
    cairoShim.arc(0,0,r,0,2*M_PI);
    cairoShim.stroke();
  }

  template <> void Operation<OperationType::index>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(10);
    cairoShim.moveTo(-9,3);
    cairoShim.showText("idx");
  }

  template <> void Operation<OperationType::gather>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.scale(sf,sf);
    cairoShim.setFontSize(8);
    cairoShim.moveTo(-7,3);
    cairoShim.showText("x[i]");
    DrawBinOpShim drawBinOp(cairoShim);
    drawBinOp.drawPort([&](){drawBinOp.drawSymbol("x");},l,-h,rotation());
    drawBinOp.drawPort([&](){drawBinOp.drawSymbol("i");},l,h,rotation());
  }

  template <> void Operation<OperationType::meld>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.moveTo(-4,-5);
    cairoShim.scale(sf,sf);
    cairoShim.relLineTo(4,0);
    cairoShim.relLineTo(2,5);
    cairoShim.relLineTo(-2,5);
    cairoShim.relLineTo(-4,0);
    cairoShim.moveTo(-4,0);
    cairoShim.relLineTo(10,0);
    cairoShim.stroke();
  }

  template <> void Operation<OperationType::merge>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    cairoShim.moveTo(-4,-3);
    cairoShim.scale(sf,sf);
    cairoShim.relLineTo(8,0);
    cairoShim.moveTo(-4,3);
    cairoShim.relLineTo(8,0);
    cairoShim.relMoveTo(0,-10);
    cairoShim.relLineTo(0,14);
    cairoShim.stroke();
  }

  template <> void Operation<OperationType::slice>::iconDraw(const ICairoShim& cairoShim) const
  {
    const double sf = scaleFactor();
    auto& pango = cairoShim.pango();
    pango.setMarkup("[...");
    pango.setFontSize(10);
    cairoShim.moveTo(-10,-10);
    cairoShim.scale(sf,sf);
    pango.show();
    cairoShim.moveTo(-10+pango.width(),-9);
    cairoShim.relLineTo(0,pango.height()-2);
    cairoShim.stroke();
  }

  template <> void Operation<OperationType::numOps>::iconDraw(const ICairoShim& cairoShim) const
  {/* needs to be here, and is actually called */}


}

