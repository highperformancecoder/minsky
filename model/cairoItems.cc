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
// Implementations of canvas items representing operations and variables.

// invert the display of the power operator so that y is on top, and x
// below, for ticket #327
#define DISPLAY_POW_UPSIDE_DOWN


#include <boost/geometry/geometry.hpp>
#include "cairoItems.h"
#include "operation.h"
#include "minsky.h"
#include "init.h"
#include "latexMarkup.h"
#include <arrays.h>
#include <pango.h>
#include <ecolab_epilogue.h>

using namespace ecolab;
using namespace std;
using namespace minsky;
using namespace boost::geometry;

RenderOperation::RenderOperation(const OperationBase& op, cairo_t* cairo):
  op(op), cairo(cairo), hoffs(0)
{
  cairo_t *lcairo=cairo;
  cairo_surface_t* surf=NULL;
  if (!lcairo)
    {
      surf = cairo_image_surface_create(CAIRO_FORMAT_A1, 100,100);
      lcairo = cairo_create(surf);
    }

  const float l=op.l, r=op.r;
  w=0.5*(-l+r);
  h=op.h;

  switch (op.type())
    {
    case OperationType::constant:
    case OperationType::data:
      {
        cairo_text_extents_t bbox;
        const NamedOp& c=dynamic_cast<const NamedOp&>(op);

        Pango pango(lcairo);
        pango.setFontSize(10);
        pango.setMarkup(latexToPango(c.description));
        w=0.5*pango.width()+2; 
        h=0.5*pango.height()+4;
        hoffs=pango.top();
        break;
      }
    case OperationType::integrate:
      {
        const IntOp& i=dynamic_cast<const IntOp&>(op);
        if (i.coupled())
          {
            RenderVariable rv(*i.intVar,cairo);
            w+=i.intVarOffset+rv.width(); 
            h=max(h, rv.height());
          }
        break;
      }
    default: break;
    }
 if (surf) //cleanup temporary surface
    {
      cairo_destroy(lcairo);
      cairo_surface_destroy(surf);
    }
}

Polygon RenderOperation::geom() const
{
  Rotate rotate(op.rotation, op.x(), op.y());
  float zl=op.l*op.zoomFactor, zh=op.h*op.zoomFactor, 
    zr=op.r*op.zoomFactor;
  Polygon r;
  // TODO: handle bound integration variables, and constants
  r+= rotate(op.x()+zl, op.y()-zh), rotate(op.x()+zl, op.y()+zh), 
    rotate(op.x()+zr, op.y());
  correct(r);
  return r;
}

void RenderOperation::draw()
{
  op.draw(cairo);
}

void OperationBase::draw(cairo_t* cairo) const
{
  // if rotation is in 1st or 3rd quadrant, rotate as
  // normal, otherwise flip the text so it reads L->R
  double angle=rotation * M_PI / 180.0;
  double fm=std::fmod(rotation,360);
  bool textFlipped=!((fm>-90 && fm<90) || fm>270 || fm<-270);
  double coupledIntTranslation=0;

  auto t=type();
  // call the iconDraw method if data description is empty
  if (t==OperationType::data && dynamic_cast<const NamedOp&>(*this).description.empty())
    t=OperationType::numOps;

  switch (t)
    {
      // at the moment its too tricky to get all the information
      // together for rendering constants
    case OperationType::constant:
    case OperationType::data:
      {
        
        const NamedOp& c=dynamic_cast<const NamedOp&>(*this);
        cairo_save(cairo);
        
        Pango pango(cairo);
        pango.setFontSize(10*zoomFactor);
        pango.setMarkup(latexToPango(c.description));
        pango.angle=angle + (textFlipped? M_PI: 0);
        Rotate r(rotation+ (textFlipped? 180: 0),0,0);

        // parameters of icon in userspace (unscaled) coordinates
        float w, h, hoffs;
        w=0.5*pango.width()+2*zoomFactor; 
        h=0.5*pango.height()+4*zoomFactor;
        hoffs=pango.top()/zoomFactor;
    
        cairo_move_to(cairo,r.x(-w+1,-h-hoffs+2*zoomFactor), r.y(-w+1,-h-hoffs+2*zoomFactor));
        pango.show();
        cairo_restore(cairo);
        cairo_save(cairo);
        cairo_rotate(cairo, angle);
               
        cairo_set_source_rgb(cairo,0,0,1);
        cairo_move_to(cairo,-w,-h);
        cairo_line_to(cairo,-w,h);
        cairo_line_to(cairo,w,h);

        cairo_line_to(cairo,w+2*zoomFactor,0);
        cairo_line_to(cairo,w,-h);
        cairo_close_path(cairo);
        cairo_clip_preserve(cairo);
        cairo_stroke(cairo);

        // set the output ports coordinates
        // compute port coordinates relative to the icon's
        // point of reference
        Rotate rr(rotation,0,0);

        ports[0]->moveTo(x()+rr.x(w+2,0), y()+rr.y(w+2,0));
        if (numPorts()>1)
          ports[1]->moveTo(x()+rr.x(-w,0), y()+rr.y(-w,0));
        cairo_restore(cairo); // undo rotation
        if (mouseFocus)
          {
            drawPorts(cairo);
            displayTooltip(cairo);
          }
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
            cairo_rotate(cairo,rotation*M_PI/180.0);
            coupledIntTranslation=-0.5*(i->intVarOffset+2*rv.width()+2+r)*zoomFactor;
            //            cairo_translate(cairo, coupledIntTranslation, 0);
            cairo_rotate(cairo,-rotation*M_PI/180.0);
          }
      cairo_save(cairo);
      cairo_scale(cairo,zoomFactor,zoomFactor);
      iconDraw(cairo);
      cairo_restore(cairo);
      break;
    default:
      cairo_save(cairo);
      cairo_scale(cairo,zoomFactor,zoomFactor);
      iconDraw(cairo);
      cairo_restore(cairo);
      break;
    }

  float l=OperationBase::l*zoomFactor, r=OperationBase::r*zoomFactor, 
    h=OperationBase::h*zoomFactor;
  int intVarWidth=0;
  cairo_save(cairo);
  cairo_rotate(cairo, angle);
  cairo_move_to(cairo,l,h);
  cairo_line_to(cairo,l,-h);
  cairo_line_to(cairo,r,0);
              
  cairo_close_path(cairo);

  //  cairo_save(cairo);
  cairo_set_source_rgb(cairo,0,0,1);
  cairo_stroke_preserve(cairo);
  //  cairo_restore(cairo);

  if (const IntOp* i=dynamic_cast<const IntOp*>(this))
    if (i->coupled())
      {
        float ivo=i->intVarOffset*zoomFactor;
        cairo_new_path(cairo);
        cairo_move_to(cairo,r,0);
        cairo_line_to(cairo,r+ivo,0);
        cairo_set_source_rgb(cairo,0,0,0);
        cairo_stroke(cairo);
        
        VariablePtr intVar=i->intVar;
        intVar->zoomFactor=zoomFactor;
        // display an integration variable next to it
        RenderVariable rv(*intVar, cairo);
        // save the render width for later use in setting the clip
        intVarWidth=rv.width()*zoomFactor; 
        // set the port location...
        intVar->moveTo(i->x()+r+ivo+intVarWidth, i->y());
            
        cairo_save(cairo);
        cairo_translate(cairo,r+ivo+intVarWidth,0);
        // to get text to render correctly, we need to set
        // the var's rotation, then antirotate it
        i->intVar->rotation=i->rotation;
        cairo_rotate(cairo, -M_PI*i->rotation/180.0);
        rv.draw();
        //i->getIntVar()->draw(cairo);
        cairo_restore(cairo);

        // build clip path the hard way grr...
        cairo_move_to(cairo,l,h);
        cairo_line_to(cairo,l,-h);
        cairo_line_to(cairo,r,0);
        cairo_line_to(cairo,r+ivo,0);
        float rvw=rv.width()*zoomFactor, rvh=rv.height()*zoomFactor;
        cairo_line_to(cairo,r+ivo,-rvh);
        cairo_line_to(cairo,r+ivo+2*rvw,-rvh);
        cairo_line_to(cairo,r+ivo+2*rvw+2*zoomFactor,0);
        cairo_line_to(cairo,r+ivo+2*rvw,rvh);
        cairo_line_to(cairo,r+ivo,rvh);
        cairo_line_to(cairo,r+ivo,0);
        cairo_line_to(cairo,r,0);
        cairo_close_path(cairo);
      }

  cairo_clip(cairo);
  if (selected) drawSelected(cairo);

  // compute port coordinates relative to the icon's
  // point of reference
  double x0=r-2, y0=0, x1=l, y1=numPorts() > 2? -h+3: 0, 
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
      displayTooltip(cairo);
    }

}

namespace
{
  cairo::Surface dummySurf(cairo_image_surface_create(CAIRO_FORMAT_A1, 100,100));
}

RenderVariable::RenderVariable(const VariableBase& var, cairo_t* cairo):
  Pango(cairo? cairo: dummySurf.cairo()), var(var), cairo(cairo)
{
  setFontSize(12);
  setMarkup(latexToPango(var.name()));
  w=0.5*Pango::width()+12; // enough space for numerical display 
  h=0.5*Pango::height()+4;
  hoffs=Pango::top();
}

Polygon RenderVariable::geom() const
{
  float x=var.x(), y=var.y();
  float wz=w*var.zoomFactor, hz=h*var.zoomFactor;
  Rotate rotate(var.rotation, x, y);

  Polygon r;
  r+= rotate(x-wz, y-hz), rotate(x-wz, y+hz), 
    rotate(x+wz, y+hz), rotate(x+wz, y-hz);
  correct(r);
  return r;
}

void RenderVariable::draw()
{
  //  updatePortLocs();
  var.draw(cairo);
}

void VariableBase::draw(cairo_t *cairo) const
{
  double angle=rotation * M_PI / 180.0;
  double fm=std::fmod(rotation,360);

  //  cairo_save(cairo);
  //  cairo_save(cairo);

  //cairo_scale(cairo,zoomFactor,zoomFactor);

  double v=value();
  int sciExp=(v!=0)? floor(log(fabs(v))/log(10)): 0;
  int engExp=3*(sciExp/3);
  const char* conv;
  switch (sciExp%3)
    {
    case 0: case -2: conv="%5.2f"; break;
    case 1: case -1: conv="%5.1f"; break;
    case 2: conv="%5.0f"; break;
    }
  char val[6];
  sprintf(val,conv,v*pow(10,-engExp));

  RenderVariable rv(*this,cairo);
  rv.setFontSize(12*zoomFactor);
  // if rotation is in 1st or 3rd quadrant, rotate as
  // normal, otherwise flip the text so it reads L->R
  bool notflipped=(fm>-90 && fm<90) || fm>270 || fm<-270;
  Rotate r(rotation + (notflipped? 0: 180),0,0);
  rv.angle=angle+(notflipped? 0: M_PI);

  Pango pangoVal(cairo);
  pangoVal.setFontSize(6*zoomFactor);
  pangoVal.setMarkup(val);
  pangoVal.angle=angle+(notflipped? 0: M_PI);

  // parameters of icon in userspace (unscaled) coordinates
  float w, h, hoffs;
  w=rv.width()*zoomFactor; 
  h=rv.height()*zoomFactor;
  hoffs=rv.top()/zoomFactor;

  cairo_move_to(cairo,r.x(-w+1,-h-hoffs+2), r.y(-w+1,-h-hoffs+2)/*h-2*/);
  rv.show();
  cairo_move_to(cairo,r.x(w-pangoVal.width()-2,-h-hoffs+2),
                r.y(w-pangoVal.width()-2,-h-hoffs+2));
  pangoVal.show();
//  cairo_move_to(cairo,r.x(pango.width()-w+1,0),r.y(pango.width()-w+1,0));
//  cairo_line_to(cairo,r.x(w,0), r.y(w,r.y(w,0)));
//  cairo_stroke(cairo);
  if (engExp!=0)
    {
      pangoVal.setMarkup("×10<sup>"+to_string(engExp)+"</sup>");
      cairo_move_to(cairo,r.x(w-pangoVal.width()-2,0),r.y(w-pangoVal.width()-2,0));
      pangoVal.show();
    }
  //  cairo_restore(cairo);

  cairo_save(cairo);
  cairo_rotate(cairo, angle);
  // constants and parameters should be rendered in blue, all others in red
  switch (type())
    {
    case constant: case parameter:
      cairo_set_source_rgb(cairo,0,0,1);
      break;
    default:
      cairo_set_source_rgb(cairo,1,0,0);
      break;
    }
  cairo_move_to(cairo,-w,-h);
  if (lhs())
    cairo_line_to(cairo,-w+2*zoomFactor,0);
  cairo_line_to(cairo,-w,h);
  cairo_line_to(cairo,w,h);
  cairo_line_to(cairo,w+2*zoomFactor,0);
  cairo_line_to(cairo,w,-h);
  cairo_close_path(cairo);
  cairo_stroke_preserve(cairo);
  cairo_save(cairo);
  cairo_clip(cairo);
  if (selected) drawSelected(cairo);
  cairo_restore(cairo);
  
  // draw slider
  if (!isStock() && (type()!=VariableType::flow || !inputWired()))
    {
      cairo_save(cairo);
      cairo_set_source_rgb(cairo,0,0,0);
      cairo_arc(cairo,(notflipped?1:-1)*zoomFactor*rv.handlePos(), (notflipped? -h: h), sliderHandleRadius, 0, 2*M_PI);
      cairo_fill(cairo);
      cairo_restore(cairo);
    }

  cairo_restore(cairo); // undo rotation
   
 {
    double x0=w, y0=0, x1=-w+2, y1=0;
    double sa=sin(angle), ca=cos(angle);
    if (ports.size()>0)
      ports[0]->moveTo(x()+(x0*ca-y0*sa), 
                       y()+(y0*ca+x0*sa));
    if (ports.size()>1)
      ports[1]->moveTo(x()+(x1*ca-y1*sa), 
                       y()+(y1*ca+x1*sa));
  }

  if (mouseFocus)
    {
      drawPorts(cairo);
      displayTooltip(cairo);
    }
}

void RenderVariable::updatePortLocs() const
{
  double angle=var.rotation * M_PI / 180.0;
  double x0=w, y0=0, x1=-w+2, y1=0;
  double sa=sin(angle), ca=cos(angle);
  var.ports[0]->moveTo(var.x()+var.zoomFactor*(x0*ca-y0*sa), 
                           var.y()+var.zoomFactor*(y0*ca+x0*sa));
  var.ports[1]->moveTo(var.x()+var.zoomFactor*(x1*ca-y1*sa), 
                           var.y()+var.zoomFactor*(y1*ca+x1*sa));
}

bool RenderVariable::inImage(float x, float y)
{
  float dx=x-var.x(), dy=y-var.y();
  float rx=dx*cos(var.rotation*M_PI/180)-dy*sin(var.rotation*M_PI/180);
  float ry=dy*cos(var.rotation*M_PI/180)+dx*sin(var.rotation*M_PI/180);
  return rx>=-w && rx<=w && ry>=-h && ry <= h;
}

double RenderVariable::handlePos() const
{
  var.initSliderBounds();
  var.adjustSliderBounds();
  return w*(var.value()-0.5*(var.sliderMin+var.sliderMax))/(var.sliderMax-var.sliderMin);
}

void minsky::drawTriangle
(cairo_t* cairo, double x, double y, const cairo::Colour& col, double angle)
{
  cairo_save(cairo);
  cairo_new_path(cairo);
  cairo_set_source_rgba(cairo,col.r,col.g,col.b,col.a);
  cairo_translate(cairo,x,y);
  cairo_rotate(cairo, angle);
  cairo_move_to(cairo,10,0);
  cairo_line_to(cairo,0,-3);
  cairo_line_to(cairo,0,3);
  cairo_fill(cairo);
  cairo_restore(cairo);
}

