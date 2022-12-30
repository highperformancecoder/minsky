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

#include "minsky.h"
#include "item.h"
#include "group.h"
#include "zoom.h"
#include "variable.h"
#include "latexMarkup.h"
#include "geometry.h"
#include "selection.h"
#include "lasso.h"
#include <pango.h>
#include <cairo_base.h>
#include "item.rcd"
#include "noteBase.rcd"
#include "noteBase.xcd"
#include "polyRESTProcessBase.h"
#include "polyRESTProcessBase.rcd"
#include "polyRESTProcessBase.xcd"
#include "minsky_epilogue.h"
#include <exception>

using ecolab::Pango;
using ecolab::cairo::CairoSave;
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
        cairo::CairoSave cs(surf.cairo());
        cairo_rotate(surf.cairo(),-x.rotation()*M_PI/180);
        x.draw(surf.cairo());
      }
#ifndef NDEBUG
    catch (const std::exception& e) 
      {cerr<<"illegal exception caught in draw(): "<<e.what()<<endl;}
    catch (...) {cerr<<"illegal exception caught in draw()";}
#else
    catch(...) {}
#endif
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
    return m_x;
  }

  float Item::y() const 
  {
    if (auto g=group.lock())
      return zoomFactor()*m_y+g->y();
    return m_y;
  }

  float Item::zoomFactor() const
  {
    if (auto g=group.lock())
      return g->zoomFactor()*g->relZoom;
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
    for (auto& p: m_ports)
      p->deleteWires();
  }

  namespace
  {
    inline bool near(float x0, float y0, float x1, float y1, float d)
    {
      return abs(x0-x1)<d && abs(y0-y1)<d;
    }
    inline bool near(float x0, float y0, float x1, float y1, float d, const Rotate& r)
    {
      return near(x0,y0,r.x(x1,y1),r.y(x1,y1),d);
    }
  }

  std::vector<Point> Item::corners() const
  {
    ensureBBValid();
    auto left=x()+bb.left()*zoomFactor(), right=x()+bb.right()*zoomFactor();
    auto top=y()+bb.top()*zoomFactor(), bottom=y()+bb.bottom()*zoomFactor();
    memoisedRotator.update(rotation(),x(),y());
    return {memoisedRotator(left,top),memoisedRotator(left,bottom),
        memoisedRotator(right,bottom),memoisedRotator(right,top)};
  }
  
  float Item::left()   const
  {
    auto left=x();
    for (auto& p: corners())
      if (p.x()<left) left=p.x();
    return left;
  }
  float Item::right()   const
  {
    auto right=x();
    for (auto& p: corners())
      if (p.x()>right) right=p.x();
    return right;
  }
  float Item::top()   const
  {
    auto top=y();
    for (auto& p: corners())
      if (p.y()<top) top=p.y();
    return top;
  }
  float Item::bottom()   const
  {
    auto bottom=y();
    for (auto& p: corners())
      if (p.y()>bottom) bottom=p.y();
    return bottom;
  }

  void Item::adjustBookmark() const
  {
    auto& bookmarks=minsky().model->bookmarks;
    if (bookmark)
      minsky().model->addBookmarkXY(left(),top(),bookmarkId());
    else
      bookmarks.erase(bookmarkId());
  }
  
  Point BottomRightResizerItem::resizeHandleCoords() const
  {
    // ensure resize handle is always active on the same corner of variable/items. for ticket 1232
    ensureBBValid();
    memoisedRotator.update(rotation(),x(),y());
    auto left=x()+bb.left()*zoomFactor(), right=x()+bb.right()*zoomFactor();
    auto top=y()+bb.top()*zoomFactor(), bottom=y()+bb.bottom()*zoomFactor();
    switch (quadrant(rotation()))
      {
      case 0:
        return memoisedRotator(right,bottom);
      case 1:
        return memoisedRotator(right,top);
      case 2:
        return memoisedRotator(left,top);
      case 3:
        return memoisedRotator(left,bottom);
      default:
        assert(false);
        return {};
      }
  }
  
  bool Item::onResizeHandle(float x, float y) const
  {
    float rhSize=resizeHandleSize();
    auto cnrs=corners();
    return any_of(cnrs.begin(), cnrs.end(), [&](const Point& p)
                  {return near(x,y,p.x(),p.y(),rhSize);});
  }

  bool BottomRightResizerItem::onResizeHandle(float x, float y) const
  {
    Point p=resizeHandleCoords();
    return near(x,y,p.x(),p.y(),resizeHandleSize());
  }

 
  bool Item::visible() const 
  {
    if (attachedToDefiningVar()) return false;   
    auto g=group.lock();
    return (!g || g->displayContents());
  }

  bool Item::visibleWithinGroup() const 
  {return !attachedToDefiningVar();}

  

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
    if (bookmark) adjustBookmark();
    assert(abs(x-this->x())<1 && abs(y-this->y())<1);
  }

  ClickType::Type Item::clickType(float x, float y)
  {     	    
    // if selecting a contained variable, the delegate to that
    if (auto item=select(x,y))
      return item->clickType(x,y);
    
    // firstly, check whether a port has been selected
    for (auto& p: m_ports)
      {
        if (hypot(x-p->x(), y-p->y()) < portRadius*zoomFactor())
          return ClickType::onPort;
      }          

    if (onResizeHandle(x,y)) return ClickType::onResize;         
    if (inItem(x,y)) return ClickType::inItem;
    
    ecolab::cairo::Surface dummySurf
      (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,nullptr));
    draw(dummySurf.cairo());
    if (cairo_in_clip(dummySurf.cairo(), (x-this->x()), (y-this->y())))
      return ClickType::onItem;               
    return ClickType::outside;
  }

  void Item::drawPorts(cairo_t* cairo) const
  {
    CairoSave cs(cairo);
    cairo_new_path(cairo);
    for (auto& p: m_ports)
      {
        cairo_new_sub_path(cairo);
        cairo_arc(cairo, p->x()-x(), p->y()-y(), portRadius*zoomFactor(), 0, 2*M_PI);
      }
    cairo_set_source_rgb(cairo, 0,0,0);
    cairo_set_line_width(cairo,1);
    cairo_stroke(cairo);
  }

  void Item::drawSelected(cairo_t* cairo)
  {
    // implemented by filling the clip region with a transparent grey
    CairoSave cs(cairo);
    cairo_set_source_rgba(cairo, 0.5,0.5,0.5,0.4);
    cairo_paint(cairo);
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
    double angle=0.5*M_PI;
    for (auto& p: corners())
      {
        angle+=0.5*M_PI;
        drawResizeHandle(cairo,p.x()-x(),p.y()-y(),sf,angle);
      }
    cairo_stroke(cairo);
  }

  void BottomRightResizerItem::drawResizeHandles(cairo_t* cairo) const
  { 			  			
    Point p=resizeHandleCoords();
    drawResizeHandle(cairo,p.x()-x(),p.y()-y(),resizeHandleSize(),0);
    cairo_stroke(cairo);
  }
  
  bool Item::attachedToDefiningVar(std::set<const Item*>& visited) const
  {
    if (!visited.insert(this).second) return false; // break network cycles
    if ((variableCast() || operationCast()) && !m_ports.empty())  
      for (auto w: m_ports[0]->wires())
        if (w->attachedToDefiningVar(visited)) return true;
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
    return portsSize()>0 && !ports(0).lock()->input()? ports(0).lock(): nullptr;
  }
  
  shared_ptr<Port> Item::closestInPort(float x, float y) const
  {
    if (auto v=select(x,y))
      return v->closestInPort(x,y);
    shared_ptr<Port> r;
    for (size_t i=0; i<m_ports.size(); ++i)
      if (m_ports[i]->input() &&
          (!r || sqr(m_ports[i]->x()-x)+sqr(m_ports[i]->y()-y) <
           sqr(r->x()-x)+sqr(r->y()-y)))
        r=m_ports[i];
    return r;
  }

  void ItemExclude::removeControlledItems() const
  {
    if (auto g=group.lock())
      removeControlledItems(*g);
  }

  ItemPtr Item::itemPtrFromThis() const
  {
    if (auto g=group.lock())
      return g->findItem(*this);
    return {};
  }
  
}

CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::NoteBase);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Item);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::ClickType);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::BottomRightResizerItem);
