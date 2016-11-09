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

#include "group.h"
#include "wire.h"
#include "operation.h"
#include "minsky.h"
#include "cairoItems.h"
#include <cairo_base.h>
#include <ecolab_epilogue.h>
using namespace std;
using namespace ecolab::cairo;

namespace minsky
{
  Group& GroupPtr::operator*() const {return dynamic_cast<Group&>(ItemPtr::operator*());}
  Group* GroupPtr::operator->() const {return dynamic_cast<Group*>(ItemPtr::operator->());}

  SVGRenderer Group::svgRenderer;

  // assigned the cloned equivalent of a port
  void asgClonedPort(shared_ptr<Port>& p, const map<Item*,ItemPtr>& cloneMap)
  {
    auto clone=cloneMap.find(&p->item);
    auto& oports=p->item.ports;
    if (clone!=cloneMap.end())
      {
        auto opIt=find(oports.begin(), oports.end(), p);
        assert(opIt != oports.end());
        // set the new port to have the equivalent position in the clone
        p=clone->second->ports[opIt-oports.begin()];
      }
  }

  GroupItems& GroupItems::operator=(const GroupItems& x)
  {
    if (&x==this) return *this;
    clear();
    // a map of original to cloned items (weak references)
    map<Item*,ItemPtr> cloneMap;
    for (auto& i: x.items) cloneMap[i.get()]=addItem(i->clone());
    for (auto& i: x.groups) cloneMap[i.get()]=addGroup(i->clone());
    for (auto& w: x.wires) 
      {
        auto f=w->from(), t=w->to();
        asgClonedPort(f,cloneMap);
        asgClonedPort(t,cloneMap);
        addWire(new Wire(f,t,w->coords()));
      }

    for (auto& v: x.inVariables)
      {
        assert(cloneMap.count(v.get()));
        inVariables.push_back(dynamic_pointer_cast<VariableBase>(cloneMap[v.get()]));
      }
    for (auto& v: x.outVariables)
      {
        assert(cloneMap.count(v.get()));
        outVariables.push_back(dynamic_pointer_cast<VariableBase>(cloneMap[v.get()]));
      }
    return *this;
  }

  shared_ptr<Group> Group::self() const
  {
    if (auto parent=group.lock())
      {
        for (auto& g: parent->groups)
          if (g.get()==this)
            return g;
      }
    else // check if we're the global group
      if (cminsky().model.get()==this)
        return cminsky().model;
    return shared_ptr<Group>();
  }

  ItemPtr GroupItems::removeItem(const Item& it)
  {
    for (auto i=items.begin(); i!=items.end(); ++i)
      if (i->get()==&it)
        {
          ItemPtr r=*i;
          items.erase(i);
          return r;
        }

    for (auto& g: groups)
      if (ItemPtr r=g->removeItem(it))
        return r;
    return ItemPtr();
  }
       
  WirePtr GroupItems::removeWire(const Wire& w)
  {
    for (auto i=wires.begin(); i!=wires.end(); ++i)
      if (i->get()==&w)
        {
          WirePtr r=*i;
          wires.erase(i);
          return r;
        }

    for (auto& g: groups)
      if (WirePtr r=g->removeWire(w))
        return r;
    return WirePtr();
  }

  GroupPtr GroupItems::removeGroup(const Group& group)
  {
    for (auto i=groups.begin(); i!=groups.end(); ++i)
      if (i->get()==&group)
        {
          GroupPtr r=*i;
          groups.erase(i);
          return r;
        }

    for (auto& g: groups)
      if (GroupPtr r=g->removeGroup(group))
        return r;
    return GroupPtr();
  }
       
  ItemPtr GroupItems::findItem(const Item& it) const 
  {
    // start by looking in the group it thnks it belongs to
    if (auto g=it.group.lock())
      if (g.get()!=this) 
        {
          auto i=g->findItem(it);
          if (i) return i;
        }
    return findAny(&Group::items, [&](const ItemPtr& x){return x.get()==&it;});
  }


  ItemPtr GroupItems::addItem(const shared_ptr<Item>& it)
  {
    assert(it);
    if (auto x=dynamic_pointer_cast<Group>(it))
      return addGroup(x);
   
    // stash position
    float x=it->x(), y=it->y();
    auto origGroup=it->group.lock();

    if (origGroup.get()==this) return it; // nothing to do.
    if (origGroup)
      origGroup->removeItem(*it);

    if (auto _this=dynamic_cast<Group*>(this))
      it->group=_this->self();
    it->moveTo(x,y);

    // move wire to highest common group
    // TODO add in I/O variables if needed
    for (auto& p: it->ports)
      {
        assert(p);
        for (auto& w: p->wires)
          {
            assert(w);
            adjustWiresGroup(*w);
          }
      }

    // need to deal with integrals especially because of the attached variable
    if (auto intOp=dynamic_cast<IntOp*>(it.get()))
      if (intOp->intVar)
        {
          if (auto oldG=intOp->intVar->group.lock())
            {
              if (oldG.get()!=this)
                addItem(oldG->removeItem(*intOp->intVar));
            }
          else
            addItem(intOp->intVar);
        }
    items.push_back(it);
    return items.back();
  }

  void GroupItems::adjustWiresGroup(Wire& w)
  {
    // Find common ancestor group, and move wire to it
    assert(w.from() && w.to());
    shared_ptr<Group> p1=w.from()->item.group.lock(), p2=w.to()->item.group.lock();
    assert(p1 && p2);
    unsigned l1=p1->level(), l2=p2->level();
    for (; l1>l2; l1--) p1=p1->group.lock();
    for (; l2>l1; l2--) p2=p2->group.lock();
    while (p1!=p2) 
      {
        assert(p1 && p2);
        p1=p1->group.lock();
        p2=p2->group.lock();
      }
    w.moveIntoGroup(*p1);
  }
  
  void GroupItems::splitBoundaryCrossingWires()
  {
    // Wire::split will invalidate the Items::iterator, so collect
    // wires to split first
    set<Wire*> wiresToSplit;
    for (auto& i: items)
      for (auto& p: i->ports)
        for (auto w: p->wires)
          wiresToSplit.insert(w);
    for (auto w: wiresToSplit)
      w->split();
  }


  void Group::moveContents(Group& source) {
     if (&source!=this)
       {
         for (auto& i: source.groups)
           if (i->higher(*this))
             throw error("attempt to move a group into itself");
         // make temporary copies as addItem removes originals
         auto copyOfItems=source.items;
         for (auto& i: copyOfItems)
           addItem(i);
         auto copyOfGroups=source.groups;
         for (auto& i: source.groups)
           addGroup(i);
         /// no need to move wires, as these are handled above
         source.clear();
       }
  }

  VariablePtr Group::addIOVar()
  {
    VariablePtr v(VariableType::flow, cminsky().variableValues.newName(to_string(id)+":"));
    addItem(v);
    return v;
  }
  
  void Group::resizeOnContents()
  {
    double x0, x1, y0, y1;
    contentBounds(x0,y0,x1,y1);
    double xx=0.5*(x0+x1), yy=0.5*(y0+y1);
    double dx=xx-x(), dy=yy-y();
    float l,r; margins(l,r);
    width=(x1-x0)+l+r;
    height=(y1-y0);

    // adjust contents by the offset
    for (auto& i: items)
      i->moveTo(i->x()-dx, i->y()-dy);
    for (auto& i: groups)
      i->moveTo(i->x()-dx, i->y()-dy);

    moveTo(xx,yy);

  }

  bool Group::nocycles() const
  {
    set<const Group*> sg;
    sg.insert(this);
    for (auto i=group.lock(); i; i=i->group.lock())
      if (!sg.insert(i.get()).second)
        return false;
    return true;
  }

  GroupPtr GroupItems::addGroup(const std::shared_ptr<Group>& g)
  {
    auto origGroup=g->group.lock();
    if (origGroup.get()==this) return g; // nothing to do
    if (origGroup)
      origGroup->removeGroup(*g);
    if (auto _this=dynamic_cast<Group*>(this))
      g->group=_this->self();
    groups.push_back(g);
    assert(nocycles());
    return groups.back();
  }

  WirePtr GroupItems::addWire(const std::shared_ptr<Wire>& w)
  {
    assert(w->from() && w->to());
    wires.push_back(w);
    return wires.back();
  }
  WirePtr GroupItems::addWire(const Item& from, const Item& to, unsigned toPortIdx, const std::vector<float>& coords) {
    // disallow self-wiring
    if (&from==&to) 
      return WirePtr();

    if (toPortIdx>=to.ports.size()) 
      return WirePtr();

    auto& fromP=from.ports[0];
    auto& toP=to.ports[toPortIdx];
    // wire must go from an output port to an input port
    if (fromP->input() || !toP->input())
      return WirePtr();

    // check that multiple input wires are only to binary ops.
    if (toP->wires.size()>=1 && !toP->multiWireAllowed())
      return WirePtr();

    // check that a wire doesn't already exist connecting these two ports
    for (auto& w: toP->wires)
      if (w->from()==fromP)
        return WirePtr();

    auto w=addWire(new Wire(fromP, toP, coords));
    adjustWiresGroup(*w);

    return w;
  }


  bool Group::higher(const Group& x) const
  {
    //if (!x) return false; // global group x is always higher
    for (auto i: groups)
      if (i.get()==&x) return true;
    for (auto i: groups)
      if (higher(*i))
        return true;
    return false;
  }

  unsigned Group::level() const
  {
    assert(nocycles());
    unsigned l=0;
    for (auto i=group.lock(); i; i=i->group.lock()) l++;
    return l;
  }

  namespace
  {
    template <class G>
    G& globalGroup(G& start)
    {
      auto g=&start;
      while (auto g1=g->group.lock())
        g=g1.get();
      return *g;
    }
  }

  const Group& Group::globalGroup() const
  {return minsky::globalGroup(*this);}
  Group& Group::globalGroup()
  {return minsky::globalGroup(*this);}


  bool Group::uniqueItems(set<void*>& idset) const
  {
    for (auto& i: items)
      if (!idset.insert(i.get()).second) return false;
    for (auto& i: wires)
      if (!idset.insert(i.get()).second) return false;
    for (auto& i: groups)
      if (!idset.insert(i.get()).second || !i->uniqueItems(idset)) 
        return false;
    return true;
  }

  float Group::contentBounds(double& x0, double& y0, double& x1, double& y1) const
  {
    float localZoom=1;
    x0=numeric_limits<float>::max();
    x1=-numeric_limits<float>::max();
    y0=numeric_limits<float>::max();
    y1=-numeric_limits<float>::max();

#ifndef CAIRO_HAS_RECORDING_SURFACE
#error "Please upgrade your cairo to a version implementing recording surfaces"
#endif
    SurfacePtr surf
      (new Surface
       (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL)));
    for (auto& i: items)
      try 
        {
          i->draw(surf->cairo());  
          x0=min(double(i->x())-0.5*surf->width(), x0);
          x1=max(double(i->x())+0.5*surf->width(), x1);
          y0=min(double(i->y())-0.5*surf->height(), y0);
          y1=max(double(i->y())+0.5*surf->height(), y1);
          localZoom=i->zoomFactor;
        }
      catch (const std::exception& e) 
        {cerr<<"illegal exception caught in draw()"<<e.what()<<endl;}
      catch (...) {cerr<<"illegal exception caught in draw()";}


//
//
//    double xx=0.5*(x0+x1), yy=0.5*(y0+y1);
//    x0+=x()-xx;
//    x1+=x()-xx;
//    y0+=y()-yy;
//    y1+=y()-yy;

    for (auto& i: groups)
      {
        float w=0.5f*i->width*i->zoomFactor,
          h=0.5f*i->height*i->zoomFactor;
        x0=min(i->x()-0.5*w, x0);
        x1=max(i->x()+0.5*w, x1);
        y0=min(i->y()-0.5*h, y0);
        y1=max(i->y()+0.5*h, y1);
      }


    // if there are no contents, result is not finite. In this case,
    // set the content bounds to a 10x10 sized box around the centroid of the I/O variables.

    if (x0==numeric_limits<float>::max())
      {
        // TODO!
//        float cx=0, cy=0;
//        for (int i: inVariables)
//          {
//            cx+=cminsky().variables[i]->x();
//            cy+=cminsky().variables[i]->y();
//          }
//        for (int i: outVariables)
//          {
//            cx+=cminsky().variables[i]->x();
//            cy+=cminsky().variables[i]->y();
//          }
//        int n=inVariables.size()+outVariables.size();
//        cx/=n;
//        cy/=n;
//        x0=cx-10;
//        x1=cx+10;
//        y0=cy-10;
//        y1=cy+10;
      }
//    else
//      {
//        // extend width by 2 pixels to allow for the slightly oversized variable icons
//        x0-=2*this->localZoom();
//        y0-=2*this->localZoom();
//        x1+=2*this->localZoom();
//        y1+=2*this->localZoom();
//      }

    return localZoom;
  }

  float Group::computeDisplayZoom()
  {
    double x0, x1, y0, y1;
    float l, r;
    float lz=contentBounds(x0,y0,x1,y1);
    x0=min(x0,double(x()));
    x1=max(x1,double(x()));
    y0=min(y0,double(y()));
    y1=max(y1,double(y()));
    // first compute the value assuming margins are of zero width
    displayZoom = 2*max( max(x1-x(), x()-x0)/width, max(y1-y(), y()-y0)/height );

    // account for shrinking margins
    float readjust=zoomFactor/edgeScale() / (displayZoom>1? displayZoom:1);
    margins(l,r);
    l*=readjust; r*=readjust;
    displayZoom = max(displayZoom, 
                      float(max((x1-x())/(0.5f*width-r), (x()-x0)/(0.5f*width-l))));
  
    displayZoom*=1.1*rotFactor()/lz;

    // displayZoom should never be less than 1
    displayZoom=max(displayZoom, 1.0f);
    return displayZoom;
  }

  const Group* Group::minimalEnclosingGroup(float x0, float y0, float x1, float y1) const
  {
    if (x0<x()-0.5*zoomFactor*width || x1>x()+0.5*zoomFactor*width || 
        y0<y()-0.5*zoomFactor*height || y1>y()+0.5*zoomFactor*height)
      return nullptr;
    // at this point, this is a candidate. Check if any child groups are also
    for (auto& g: groups)
      if (auto mg=g->minimalEnclosingGroup(x0,y0,x1,y1))
        return mg;
    return this;
  }

  void Group::setZoom(float factor)
  {
    bool dpc=displayContents();
    zoomFactor=factor;
    computeDisplayZoom();
    float lzoom=localZoom();
    for (auto& i: items)
      i->zoomFactor=lzoom;
    m_displayContentsChanged = dpc!=displayContents();
    for (auto& i: groups)
      {
        i->setZoom(lzoom);
        m_displayContentsChanged|=i->displayContentsChanged();
      }
  }

  void Group::zoom(float xOrigin, float yOrigin,float factor)
  {
     bool dpc=displayContents();
     Item::zoom(xOrigin, yOrigin, factor);
     m_displayContentsChanged = dpc!=displayContents();
     for (auto& i: items)
       {
         i->m_visible=displayContents();
         if (displayContents() && !m_displayContentsChanged)
           i->zoom(xOrigin, yOrigin, factor);
       }
     for (auto& i: groups)
       {
         i->m_visible=displayContents();
         if (displayContents() && !m_displayContentsChanged)
           i->zoom(xOrigin, yOrigin, factor);
         m_displayContentsChanged|=i->displayContentsChanged();
       }
  }


  namespace {
    inline float sqr(float x) {return x*x;}
  }

  ClosestPort::ClosestPort(const Group& g, InOut io, float x, float y)
  {
    float minr2=std::numeric_limits<float>::max();
    g.recursiveDo(&Group::items, [&](const Items& m, Items::const_iterator i)
                  {
                    for (auto& p: (*i)->ports)
                      if ((io!=out && p->input()) || (io!=in && !p->input()))
                        {
                          float r2=sqr(p->x()-x)+sqr(p->y()-y);
                          if (r2<minr2)
                            {
                              shared_ptr<Port>::operator=(p);
                              minr2=r2;
                            }
                        }
                    return false;
                  });
  }

  void Group::draw(cairo_t* cairo) const
  {
    double angle=rotation * M_PI / 180.0;

    // determine how big the group icon should be to allow
    // sufficient space around the side for the edge variables
    float leftMargin, rightMargin;
    margins(leftMargin, rightMargin);
    leftMargin*=zoomFactor; rightMargin*=zoomFactor;

    unsigned width=zoomFactor*this->width, height=zoomFactor*this->height;
    // bitmap needs to be big enough to allow a rotated
    // icon to fit on the bitmap.
    float rotFactor=this->rotFactor();


   // draw default group icon
    cairo_save(cairo);

    // display I/O region in grey
    //updatePortLocation();
    drawIORegion(cairo);

    cairo_translate(cairo, -0.5*width+leftMargin, -0.5*height);


              
    double scalex=double(width-leftMargin-rightMargin)/width;
    cairo_scale(cairo, scalex, 1);

    // draw a simple frame 
    cairo_rectangle(cairo,0,0,width,height);
    cairo_save(cairo);
    cairo_identity_matrix(cairo);
    cairo_set_line_width(cairo,1);
    cairo_stroke(cairo);
    cairo_restore(cairo);

    if (!displayContents())
      {
        cairo_scale(cairo,width/svgRenderer.width(),height/svgRenderer.height());
        cairo_rectangle(cairo,0, 0,svgRenderer.width(), svgRenderer.height());
        cairo_clip(cairo);
        svgRenderer.render(cairo);
      }
    cairo_restore(cairo);

    drawEdgeVariables(cairo);


    // display text label
    if (!title.empty())
      {
        cairo_save(cairo);
        cairo_scale(cairo, zoomFactor, zoomFactor);
        cairo_select_font_face
          (cairo, "sans-serif", CAIRO_FONT_SLANT_ITALIC, 
           CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cairo,12);
              
        // extract the bounding box of the text
        cairo_text_extents_t bbox;
        cairo_text_extents(cairo,title.c_str(),&bbox);
        double w=0.5*bbox.width+2; 
        double h=0.5*bbox.height+5;
        double fm=std::fmod(rotation,360);

        // if rotation is in 1st or 3rd quadrant, rotate as
        // normal, otherwise flip the text so it reads L->R
        if ((fm>-90 && fm<90) || fm>270 || fm<-270)
          cairo_rotate(cairo, angle);
        else
          cairo_rotate(cairo, angle+M_PI);
 
        // prepare a background for the text, partially obscuring graphic
        double transparency=displayContents()? 0.25: 1;
        cairo_set_source_rgba(cairo,0,1,1,0.5*transparency);
        cairo_rectangle(cairo,-w,-h,2*w,2*h);
        cairo_fill(cairo);

        // display text
        cairo_move_to(cairo,-w+1,h-4);
        cairo_set_source_rgba(cairo,0,0,0,transparency);
        cairo_show_text(cairo,title.c_str());
        cairo_restore(cairo);
      }

    if (mouseFocus)
      drawPorts(cairo);

    if (selected) drawSelected(cairo);
  }

  void Group::draw1edge(const vector<VariablePtr>& vars, cairo_t* cairo, 
                        float x) const
  {
    float top=0, bottom=0;
    for (size_t i=0; i<vars.size(); ++i)
      {
        float y=i%2? top:bottom;
        Rotate r(rotation,0,0);
        cairo_save(cairo);
        cairo_translate(cairo,x,y);
        cairo_rotate(cairo,M_PI*rotation/180);
        auto& v=vars[i];
        v->m_visible=false;
        v->m_x=r.x(x,y); v->m_y=r.y(x,y);
        RenderVariable rv(*v,cairo);
        rv.draw();
        if (i==0)
          {
            top=0.5*rv.height();
            bottom=-top;
          }
        else if (i%2)
          top+=rv.height();
        else
          bottom-=rv.height();
        cairo_restore(cairo);
      }
  }

  void Group::drawEdgeVariables(cairo_t* cairo) const
  {
    float left, right; margins(left,right);
    cairo_rotate(cairo,-M_PI*rotation/180);
    draw1edge(inVariables, cairo, -zoomFactor*0.5*(width-left));
    draw1edge(outVariables, cairo, zoomFactor*0.5*(width-right));
  }

  // draw notches in the I/O region to indicate docking capability
  void Group::drawIORegion(cairo_t* cairo) const
  {
    cairo_save(cairo);
    float left, right;
    margins(left,right);
    left*=zoomFactor;
    right*=zoomFactor;
    float y=0, dy=5*edgeScale();
    for (auto& i: inVariables)
      {
        RenderVariable rv(*i);
        y=max(y, fabs(i->y()-this->y())+rv.height()*edgeScale());
      }
    cairo_set_source_rgba(cairo,0,1,1,0.5);
    float w=0.5*zoomFactor*width, h=0.5*zoomFactor*height;

    cairo_move_to(cairo,-w,-h);
    // create notch in input region
    cairo_line_to(cairo,-w,y-dy);
    cairo_line_to(cairo,left-w-2,y-dy);
    cairo_line_to(cairo,left-w,y);
    cairo_line_to(cairo,left-w-2,y+dy);
    cairo_line_to(cairo,-w,y+dy);
    cairo_line_to(cairo,-w,h);
    cairo_line_to(cairo,left-w,h);
    cairo_line_to(cairo,left-w,-h);
    cairo_close_path(cairo);
    cairo_fill(cairo);

    y=0;
    for (auto& i: outVariables)
      {
        RenderVariable rv(*i);
        y=max(y, fabs(i->y()-this->y())+rv.height()*edgeScale());
      }
    cairo_move_to(cairo,w,-h);
    // create notch in output region
    cairo_line_to(cairo,w,y-dy);
    cairo_line_to(cairo,w-right,y-dy);
    cairo_line_to(cairo,w-right+2,y);
    cairo_line_to(cairo,w-right,y+dy);
    cairo_line_to(cairo,w,y+dy);
    cairo_line_to(cairo,w,h);
    cairo_line_to(cairo,w-right,h);
    cairo_line_to(cairo,w-right,-h);
    cairo_close_path(cairo);
    cairo_fill(cairo);

    cairo_restore(cairo);
  }


  void Group::margins(float& left, float& right) const
  {
    float scale=edgeScale()/zoomFactor;
    left=right=10*scale;
    for (auto& i: inVariables)
      {
        i->zoomFactor=edgeScale();
        float w= scale*(2*RenderVariable(*i).width()+2);
        assert(i->type()!=VariableType::undefined);
        if (w>left) left=w;
      }
    for (auto& i: outVariables)
      {
        i->zoomFactor=edgeScale();
        float w= scale*(2*RenderVariable(*i).width()+2);
        assert(i->type()!=VariableType::undefined);
        if (w>right) right=w;
      }
  }

  float Group::rotFactor() const
  {
    float rotFactor;
    float ac=abs(cos(rotation*M_PI/180));
    static const float invSqrt2=1/sqrt(2);
    if (ac>=invSqrt2) 
      rotFactor=1.15/ac; //use |1/cos(angle)| as rotation factor
    else
      rotFactor=1.15/sqrt(1-ac*ac);//use |1/sin(angle)| as rotation factor
    return rotFactor;
  }

  VariablePtr Group::select(float x, float y) const
  {
    for (auto& v: inVariables)
      if (RenderVariable(*v).inImage(x,y)) 
        return v;
    for (auto& v: outVariables)
      if (RenderVariable(*v).inImage(x,y)) 
        return v;
    return VariablePtr();
  }

}
