/*
  @copyright Steve Keen 2021
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
#include "cairoItems.h"
#include "intOp.h"
#include "intOp.rcd"
#include "itemT.rcd"
#include "minsky_epilogue.h"

namespace minsky
{
  IntOpAccessor::IntOpAccessor(): ecolab::TCLAccessor<IntOp, std::string>
    ("description",(Getter)&IntOp::description,(Setter)&IntOp::description) {}
  
  Units IntOp::units(bool check) const {
    Units r=m_ports[1]->units(check);
    if (!cminsky().timeUnit.empty())
      r[cminsky().timeUnit]++;
    r.normalise();
    return r;
  }
  
  void IntOp::draw(cairo_t* cairo) const
  { 	  
      // if rotation is in 1st or 3rd quadrant, rotate as
      // normal, otherwise flip the text so it reads L->R
    auto [angle,textFlipped]=rotationAsRadians();
      double coupledIntTranslation=0;
      const float z=zoomFactor();
    
      float l=OperationBase::l*z, r=OperationBase::r*z, 
        h=OperationBase::h*z;
      
      if (fabs(l)<iWidth()*z) l=-iWidth()*z;        
      if (r<iWidth()*z) r=iWidth()*z;    
      if (h<iHeight()*z) h=iHeight()*z;   

      if (coupled())
        {
          cairo::CairoSave cs(cairo);
          auto& iv=*intVar;
          const RenderVariable rv(iv,cairo);
          // we need to add some translation if the variable is bound
          cairo_rotate(cairo,angle);
          coupledIntTranslation=-0.5*(intVarOffset+2*rv.width()+2+r)*z;
          if (rv.width()<iv.iWidth()) coupledIntTranslation=-0.5*(intVarOffset+2*iv.iWidth()+2+r)*z;
        }
    

      {
        cairo::CairoSave cs(cairo);
        cairo_rotate(cairo, angle); 
        cairo_scale(cairo,z,z);
        if (textFlipped) cairo_rotate(cairo, M_PI);
        const double sf = scaleFactor();  
        cairo_scale(cairo,sf,sf);		  
        cairo_move_to(cairo,-7,3.5);
        cairo_show_text(cairo,"∫dt");
      }
      DrawBinOp d(cairo, zoomFactor());
      d.drawPort([&](){d.drawSymbol("0");}, l,h,rotation()); 
      d.drawPort([&](){d.drawSymbol("f");}, l,-h,rotation()); 
      
      cairo_save(cairo); 
      cairo_rotate(cairo, angle); 
       
      int intVarWidth=0;
    
   
      cairo_move_to(cairo,l,h);
      cairo_line_to(cairo,l,-h);
      cairo_line_to(cairo,r,0);     
    
      cairo_close_path(cairo);		  	 
    
      cairo_set_source_rgb(cairo,0,0,1);    
      cairo_stroke_preserve(cairo);    
    
      if (coupled())
        {
          const float ivo=intVarOffset*z;
          cairo_new_path(cairo);
          cairo_move_to(cairo,r,0);
          cairo_line_to(cairo,r+ivo,0);
          cairo_set_source_rgb(cairo,0,0,0);
          cairo_stroke(cairo);
     
          // display an integration variable next to it
          RenderVariable rv(*intVar, cairo);
          // save the render width for later use in setting the clip
          intVarWidth=rv.width()*z;
          if (rv.width()<intVar->iWidth()) intVarWidth=0.5*intVar->iWidth()*z;
          // set the port location...
          const Rotate rot(rotation(), x(), y());
          auto ivp=rot(x()+r+ivo+intVarWidth, y());
          intVar->moveTo(ivp.x(), ivp.y());
         
          cairo_save(cairo);
          cairo_translate(cairo,r+ivo+intVarWidth,0);
          // to get text to render correctly, we need to set
          // the var's rotation, then antirotate it
          intVar->rotation(rotation());
          cairo_rotate(cairo, -M_PI*rotation()/180.0);
          rv.draw();
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
    
      double x0=r, y0=0, x1=l, y1=numPorts() > 2? -h+3: 0, 
        x2=l, y2=numPorts() > 2? h-3: 0;
                  
      if (textFlipped) swap(y1,y2);
	
      // adjust for integration variable
      if (coupled())
        x0+=intVarOffset+2*intVarWidth+2;
	
      cairo_save(cairo);
      cairo_identity_matrix(cairo);
      cairo_translate(cairo, x(), y());
      cairo_rotate(cairo, angle);
      cairo_user_to_device(cairo, &x0, &y0);
      cairo_user_to_device(cairo, &x1, &y1);
      cairo_user_to_device(cairo, &x2, &y2);
      cairo_restore(cairo);
    
      if (numPorts()>0) 
        m_ports[0]->moveTo(x0, y0);
      if (numPorts()>1) 
        m_ports[1]->moveTo(x1, y1);
      if (numPorts()>2)
        m_ports[2]->moveTo(x2, y2);
	
      cairo_translate(cairo,-coupledIntTranslation,0);        
      cairo_restore(cairo); // undo rotation
      if (mouseFocus)
        {
          drawPorts(cairo);
          displayTooltip(cairo,tooltip());
        }
      if (onResizeHandles) drawResizeHandles(cairo);  
	
      cairo_new_path(cairo);
      clipPath.appendToCurrent(cairo);
      cairo_clip(cairo);          
      if (selected) drawSelected(cairo);       
  }
  
  void IntOp::resize(const LassoBox& b)
  {
    const float invZ=1.0/zoomFactor();
    this->moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));
    iWidth(0.5*std::abs(b.x1-b.x0)*invZ);
    // Ensure int op height and var height similar to make gripping resize handle easier. for ticket 1203.
    iHeight(0.25*std::abs(b.y1-b.y0)*invZ);
    intVar->iWidth(0.5*std::abs(b.x1-b.x0)*invZ);
    intVar->iHeight(0.5*std::abs(b.y1-b.y0)*invZ);
    bb.update(*this);	  
  }
  
  void IntOp::insertControlled(Selection& selection)
  {
    selection.ensureItemInserted(intVar);
  }
  
  IntOp& IntOp::operator=(const IntOp& x)
  {
    Super::operator=(x); 
    intVar.reset(x.intVar->clone());
    return *this;
  }

  void IntOp::removeControlledItems(minsky::GroupItems& g) const
  {
    if (intVar)
      g.removeItem(*intVar);
  }
  
  string IntOp::description(const string& a_desc)
  {
    auto desc=a_desc;
    
    // set a default name if none given
    if (desc.empty())
      desc=minsky().variableValues.newName(minsky::valueId(group.lock(),"int"));

    // disallow global integration variables
    if (desc[0]==':') desc=desc.substr(1);
    
    if (intVar && intVar->group.lock() == group.lock() && intVar->name()==desc)
      return description(); // nothing to do

    vector<Wire> savedWires;
    if (intVar && intVar->portsSize()>0)
      {
        // save any attached wires for later use
        for (auto w: intVar->ports(0).lock()->wires())
          savedWires.push_back(*w);
      }

    // if the variable name exists, and already has a connected
    // input, then it is not a candidate for being an integral
    // variable, so generate a new name that doesn't currently
    // exist

    const string vid=minsky::valueId(group.lock(),desc);
    auto i=minsky().variableValues.find(vid);      
    if (i!=minsky().variableValues.end()) 
      {
        if (i->second->type()!=VariableType::integral) 
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
    m_coupled=true;
    
    // recreate any previously attached wires, initially in global group.
    for (auto& w: savedWires)
      minsky().model->addWire(new Wire(intVar->ports(0), w.to(), w.coords()));

    bb.update(*this); // adjust icon bounding box - see ticket #704
    
    // this should also adjust the wire's group ownership appropriately
    if (auto g=group.lock())
      g->addItem(intVar);
    return description();
  }

  bool IntOp::toggleCoupled()
  {
    if (type()!=integrate) return false;

    assert(intVar);

    assert(m_ports.size()==3);
    if (m_coupled) 
      {
        const WirePtr newWire(new Wire(m_ports[0], intVar->ports(1)));
        if (auto g=group.lock())
          g->addWire(newWire);
        else
          minsky().model->addWire(newWire);
        intVar->controller.reset();
        intVar->rotation(rotation());
      }
    else
      {
        if (auto g=group.lock())
          {
            for (auto w: intVar->ports(1).lock()->wires())
              g->removeWire(*w);
            intVar->controller=g->findItem(*this);
          }
        intVar->mouseFocus=false; // prevent drawing of variable ports when coupled
      }
    m_coupled=!m_coupled;
    bb.update(*this); // adjust bounding box for coupled integral operation - see ticket #1055  
    return coupled();
  }

  void IntOp::pack(pack_t& x, const string& d) const
  {::pack(x,d,*this);}
      
  void IntOp::unpack(unpack_t& x, const string& d)
  {::unpack(x,d,*this);}

}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::IntOp);
