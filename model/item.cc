/*
  @copyright Steve Keen 2015
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

#include "item.h"
#include "group.h"
#include "zoom.h"
#include "variable.h"
#include "latexMarkup.h"
#include "geometry.h"
#include "selection.h"
#include "minsky.h"
#include <pango.h>
#include <cairo_base.h>
#include "minsky_epilogue.h"
#include <exception>

using ecolab::Pango;
using namespace std;

namespace minsky
{

  void BoundingBox::update(const Item& x)
  {
    ecolab::cairo::Surface surf
       (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));
    auto savedMouseFocus=x.mouseFocus;
    x.mouseFocus=false; // do not mark up icon with tooltips etc, which might invalidate this calc
    x.onResizeHandles=false;
    try
      {
        x.draw(surf.cairo());
		cairo_rotate(surf.cairo(),-x.rotation()*M_PI/180);  // perform transformation after drawing, otherwise ink extents not calculated correctly below. For ticket 1232      
      }
    catch (const std::exception& e) 
      {cerr<<"illegal exception caught in draw(): "<<e.what()<<endl;}
    catch (...) {cerr<<"illegal exception caught in draw()";}
    x.mouseFocus=savedMouseFocus;
    
    double l,t,w,h;
    cairo_recording_surface_ink_extents(surf.surface(),
                                        &l,&t,&w,&h);
    // note (0,0) is relative to the (x,y) of icon.
    double invZ=1/x.zoomFactor();
    m_left=l*invZ;
    m_right=(l+w)*invZ;
    m_top=t*invZ;
    m_bottom=(t+h)*invZ; //coordinates increase down the page
  }

  void Item::throw_error(const std::string& msg) const
  {
    cminsky().displayErrorItem(*this);
    throw runtime_error(msg);
  }
  
  float Item::x() const 
  {
    if (auto g=group.lock()) 
      return zoomFactor()*m_x+g->x();
    else
      return m_x;
  }

  float Item::y() const 
  {
    if (auto g=group.lock())
      return zoomFactor()*m_y+g->y();
    else
      return m_y;
  }

  float Item::zoomFactor() const
  {
    if (auto g=group.lock())
      return g->zoomFactor()*g->relZoom;
    else
      return 1;
  }
  
  float Item::scaleFactor() const
  { 
    return m_sf;
  }  
  
  float Item::scaleFactor(const float& sf) {
    m_sf=sf;
    bb.update(*this);
    return m_sf;
  }   
  
  void Item::deleteAttachedWires()
  {
    for (auto& p: ports)
      p->deleteWires();
  }

  namespace
  {
    bool near(float x0, float y0, float x1, float y1, float d, const Rotate& r)
    {
      return abs(x0-r.x(x1,y1))<d && abs(y0-r.y(x1,y1))<d;
    }
}
    
   std::pair<double,Point> Item::rotatedPoints() const
   {
     // ensure resize handle is always active on the same corner of variable/items for 90 and 180 degree rotations. for ticket 1232   
     double fm=std::fmod(rotation(),360), angle;	
     float x1=right(),y1=bottom();  
     if (fm==-90 || fm==270) {
       angle=-rotation();
       Rotate r1(angle,this->x(),this->y());
       x1=r1.x(right(),bottom());
       y1=r1.y(right(),bottom());						  
     }
     else if (abs(fm)==180) {
       angle=rotation();
       x1=right();
       y1=top();					
     }
     else angle=0;	
     Point p(x1,y1);  
     return make_pair(angle,p);  
  }
  
  bool Item::onResizeHandle(float x, float y) const
  {
    float rhSize=resizeHandleSize();
    Rotate r(rotation(),this->x(),this->y());
    return near(x,y,left(),top(),rhSize,r) ||
      near(x,y,right(),top(),rhSize,r) ||
      near(x,y,left(),bottom(),rhSize,r) ||
      near(x,y,right(),bottom(),rhSize,r); 
  }

   bool BottomRightResizerItem::onResizeHandle(float x, float y) const
  {
    double angle=rotatedPoints().first;		  
    Point p=rotatedPoints().second;		  
    Rotate r(angle,this->x(),this->y());		  
    return near(x,y,p.x(),p.y(),resizeHandleSize(),r);
  }

 
  bool Item::visible() const 
  {
	if (auto o=operationCast()) 
	  if (o->attachedToDefiningVar()) return false;
	if (auto v=variableCast()) 
	  if (v->attachedToDefiningVar()) return false;	  
    auto g=group.lock();
    return (!g || g->displayContents());
  }
  

  void Item::moveTo(float x, float y)
  {
    if (auto g=group.lock())
      {
        float invZ=1/zoomFactor();
        m_x=(x-g->x())*invZ;
        m_y=(y-g->y())*invZ;
      }
    else
      {
        m_x=x;
        m_y=y;
      }
    assert(abs(x-this->x())<1 && abs(y-this->y())<1);
  }

  ClickType::Type Item::clickType(float x, float y)
  {     	    
    // if selecting a contained variable, the delegate to that
    if (auto item=select(x,y))
      return item->clickType(x,y);
    
    // firstly, check whether a port has been selected
    for (auto& p: ports)
      {
        if (hypot(x-p->x(), y-p->y()) < portRadius*zoomFactor())
          return ClickType::onPort;
      }          

    if (onResizeHandle(x,y)) return ClickType::onResize;         

    ecolab::cairo::Surface dummySurf
                                (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,nullptr));
    draw(dummySurf.cairo());
    if (cairo_in_clip(dummySurf.cairo(), (x-this->x()), (y-this->y())))
      return ClickType::onItem;               
    else                  
      return ClickType::outside;
  }

  void Item::drawPorts(cairo_t* cairo) const
  {
    cairo_save(cairo);
    cairo_new_path(cairo);
    for (auto& p: ports)
      {
        cairo_new_sub_path(cairo);
        cairo_arc(cairo, p->x()-x(), p->y()-y(), portRadius*zoomFactor(), 0, 2*M_PI);
      }
    cairo_set_source_rgb(cairo, 0,0,0);
    cairo_set_line_width(cairo,1);
    cairo_stroke(cairo);
    cairo_restore(cairo);
  }

  void Item::drawSelected(cairo_t* cairo) const
  {
    // implemented by filling the clip region with a transparent grey
    cairo_save(cairo);
    cairo_set_source_rgba(cairo, 0.5,0.5,0.5,0.4);
    cairo_paint(cairo);
    cairo_restore(cairo);
  }

  namespace
  {
    void drawResizeHandle(cairo_t* cairo, double x, double y, double sf, double angle)
    {
      cairo::CairoSave cs(cairo);
      cairo_translate(cairo,x,y);
      cairo_rotate(cairo,angle);
      cairo_scale(cairo,sf,sf);
      cairo_move_to(cairo,-1,-.2);
      cairo_line_to(cairo,-1,-1);
      cairo_line_to(cairo,1,1);
      cairo_line_to(cairo,1,0.2);
      cairo_move_to(cairo,-1,-1);
      cairo_line_to(cairo,-.2,-1);
      cairo_move_to(cairo,.2,1);
      cairo_line_to(cairo,1,1);
    }
  }
  
  // Refactor resize() code for all canvas items here. For feature 25 and 94
  void Item::resize(const LassoBox& b)
  {
    // Set initial iWidth() and iHeight() to initial Pango determined values. This resize method is not very reliable. Probably a Pango issue. 
    float w=iWidth(width()), h=iHeight(height()), invZ=1/zoomFactor();   
    moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));                 
    iWidth(abs(b.x1-b.x0)*invZ);
    iHeight(abs(b.y1-b.y0)*invZ);     
    scaleFactor(std::max(1.0f,std::min(iWidth()/w,iHeight()/h)));
  }
  
  void Item::drawResizeHandles(cairo_t* cairo) const
  {
    auto sf=resizeHandleSize();
    drawResizeHandle(cairo,right()-x(),top()-y(),sf,0.5*M_PI);
    drawResizeHandle(cairo,left()-x(),top()-y(),sf,M_PI);
    drawResizeHandle(cairo,left()-x(),bottom()-y(),sf,1.5*M_PI);
    drawResizeHandle(cairo,right()-x(),bottom()-y(),sf,0);
    cairo_stroke(cairo);
  }

  void BottomRightResizerItem::drawResizeHandles(cairo_t* cairo) const
  { 			  			
    double angle=rotatedPoints().first;		  
    Point p=rotatedPoints().second;			  
    Rotate r(angle,this->x(),this->y());
    drawResizeHandle(cairo,r.x(p.x(),p.y())-x(),r.y(p.x(),p.y())-y(),0.5*resizeHandleSize(),abs(rotation())==180? 0.5*M_PI : 0);
    cairo_stroke(cairo);
  }
  
  bool Item::attachedToDefiningVar() const
  {
	if (variableCast() || operationCast())  
      for (auto w: ports[0]->wires())
        if (w->attachedToDefiningVar()) return true;
    return false;
  }    
  
  // default is just to display the detailed text (ie a "note")
  void Item::draw(cairo_t* cairo) const
  {
    Rotate r(rotation(),0,0);
    //cairo_scale(cairo,scaleFactor(),scaleFactor());   // would like to see this work like operator icon contents, but width() and height() point to nothing at this stage.
    Pango pango(cairo);
    float w, h, z=zoomFactor();
    pango.angle=rotation() * M_PI / 180.0; 
    pango.setFontSize(12*scaleFactor()*z);
    pango.setMarkup(latexToPango(detailedText));         
    // parameters of icon in userspace (unscaled) coordinates
    w=0.5*pango.width()+2*z; 
    h=0.5*pango.height()+4*z;       

    cairo_move_to(cairo,r.x(-w+1,-h+2), r.y(-w+1,-h+2));
    pango.show();

    if (mouseFocus) {
      displayTooltip(cairo,tooltip);	
    }
    if (onResizeHandles) drawResizeHandles(cairo);	
    cairo_move_to(cairo,r.x(-w,-h), r.y(-w,-h));
    cairo_line_to(cairo,r.x(w,-h), r.y(w,-h));
    cairo_line_to(cairo,r.x(w,h), r.y(w,h));
    cairo_line_to(cairo,r.x(-w,h), r.y(-w,h));
    cairo_close_path(cairo);
    cairo_clip(cairo);
    if (selected) drawSelected(cairo);
  }

  void Item::dummyDraw() const
  {
    ecolab::cairo::Surface s(cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));
    draw(s.cairo());
  }

  void Item::displayTooltip(cairo_t* cairo, const std::string& tooltip) const
  {
    string unitstr=units().latexStr();
    if (!tooltip.empty() || !unitstr.empty())
      {
        cairo::CairoSave cs(cairo);
        Pango pango(cairo);
        string toolTipText=latexToPango(tooltip);
        if (!unitstr.empty())
          toolTipText+=" Units:"+latexToPango(unitstr);
        pango.setMarkup(toolTipText);
        float z=zoomFactor();
        cairo_translate(cairo,z*(0.5*bb.width())+10,
                        z*(-0.5*bb.height())-20);
        cairo_rectangle(cairo,0,0,pango.width(),pango.height());
        cairo_set_source_rgb(cairo,1,1,1);
        cairo_fill_preserve(cairo);
        cairo_set_source_rgb(cairo,0,0,0);
        pango.show();
        cairo_stroke(cairo);
      }
  }

  shared_ptr<Port> Item::closestOutPort(float x, float y) const 
  {
    if (auto v=select(x,y))
      return v->closestOutPort(x,y);
    return ports.size()>0 && !ports[0]->input()? ports[0]: nullptr;
  }
  
  shared_ptr<Port> Item::closestInPort(float x, float y) const
  {
    if (auto v=select(x,y))
      return v->closestInPort(x,y);
    shared_ptr<Port> r;
    for (size_t i=0; i<ports.size(); ++i)
      if (ports[i]->input() &&
          (!r || sqr(ports[i]->x()-x)+sqr(ports[i]->y()-y) <
           sqr(r->x()-x)+sqr(r->y()-y)))
        r=ports[i];
    return r;
  }

  ItemPtr Item::select(float x, float y) const
  {return ItemPtr();}

  void Item::removeControlledItems() const
  {
    if (auto g=group.lock())
      removeControlledItems(*g);
  }

}
