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

#include "cairoItems.h"
#include "group.h"
#include "wire.h"
#include "operation.h"
#include "minsky.h"
#include <cairo_base.h>
#include "minsky_epilogue.h"
using namespace std;
using namespace ecolab::cairo;

// size of the top and bottom margins of the group icon
static const int topMargin=10;

namespace minsky
{
  SVGRenderer Group::svgRenderer;

  // assigned the cloned equivalent of a port
  void asgClonedPort(shared_ptr<Port>& p, const map<Item*,ItemPtr>& cloneMap)
  {
    auto clone=cloneMap.find(&p->item());
    auto& oports=p->item().ports;
    if (clone!=cloneMap.end())
      {
        auto opIt=find(oports.begin(), oports.end(), p);
        assert(opIt != oports.end());
        // set the new port to have the equivalent position in the clone
        p=clone->second->ports[opIt-oports.begin()];
      }
  }

  GroupPtr Group::copy() const
  {
    auto r=make_shared<Group>();
    r->self=r;
    // make new group a sibling of this if possible
    if (auto g=group.lock())
      g->addGroup(r);
    else
      return GroupPtr(); // do nothing if we attempt to clone the entire model
    
    // a map of original to cloned items (weak references)
    map<Item*,ItemPtr> cloneMap;
    map<IntOp*,bool> integrals;
    for (auto& i: items)
      if (auto integ=dynamic_cast<IntOp*>(i.get()))
        integrals.emplace(integ, integ->coupled());
    for (auto& i: items)
      cloneMap[i.get()]=r->addItem(i->clone());
    for (auto& i: groups)
      cloneMap[i.get()]=r->addGroup(i->copy());
    for (auto& w: wires) 
      {
        auto f=w->from(), t=w->to();
        asgClonedPort(f,cloneMap);
        asgClonedPort(t,cloneMap);
        r->addWire(new Wire(f,t,w->coords()));
      }
  
    for (auto& v: inVariables)
      {
        assert(cloneMap.count(v.get()));
        r->inVariables.push_back(dynamic_pointer_cast<VariableBase>(cloneMap[v.get()]));
        r->inVariables.back()->controller=self;
      }
    for (auto& v: outVariables)
      {
        assert(cloneMap.count(v.get()));
        r->outVariables.push_back(dynamic_pointer_cast<VariableBase>(cloneMap[v.get()]));
        r->outVariables.back()->controller=self;
      }
    // reattach integral variables to their cloned counterparts
    for (auto i: integrals)
      {
        if (auto newIntegral=dynamic_cast<IntOp*>(cloneMap[i.first].get()))
          {
            auto newIntVar=dynamic_pointer_cast<VariableBase>(cloneMap[i.first->intVar.get()]);
            if (newIntVar && newIntegral->intVar != newIntVar)
              {
                r->removeItem(*newIntegral->intVar);
                newIntegral->intVar=newIntVar;
              }
            if (i.second != newIntegral->coupled())
              newIntegral->toggleCoupled();
          }
      }
    r->computeRelZoom();
    return r;
  }

  ItemPtr Group::removeItem(const Item& it)
  {
    for (auto i=items.begin(); i!=items.end(); ++i)
      if (i->get()==&it)
        {
          ItemPtr r=*i;
          items.erase(i);
          if (auto v=r->variableCast())
              if (v->ioVar())
                {
                 remove(inVariables, r);
                 remove(outVariables, r);
                 remove(createdIOvariables, r);
                 v->controller.reset();
                }
          return r;
        }

    for (auto i=groups.begin(); i!=groups.end(); ++i)
      if (i->get()==&it)
        {
          ItemPtr r=*i;
          groups.erase(i);
          return r;
        }
    
    removeDisplayPlot();
    
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


  ItemPtr GroupItems::addItem(const shared_ptr<Item>& it, bool inSchema)
  {
    if (!it) return it;
    if (auto x=dynamic_pointer_cast<Group>(it))
      return addGroup(x);
   
    // stash position
    float x=it->x(), y=it->y();
    auto origGroup=it->group.lock();

    if (origGroup.get()==this) return it; // nothing to do.
    if (origGroup)
      origGroup->removeItem(*it);

    // stash init value to initialise new variableValue
    string init;
    if (auto v=it->variableCast())
      init=v->init();
    
    it->group=self;
    if (!inSchema) it->moveTo(x,y);

    // take into account new scope
    if (auto v=it->variableCast())
      {
        if (!inSchema && origGroup)
          if (auto destGroup=self.lock())
            {
              if (origGroup->higher(*destGroup))
                {
                  // moving local var into an inner group, check if other variables of
                  // same name exist in old group, and retain linkage
                  if (v->name()[0]!=':')
                    for (auto& i: origGroup->items)
                      if (auto vv=i->variableCast())
                        if (vv->name()==v->name())
                          v->name(':'+v->name());
                }
              else if (destGroup->higher(*origGroup))
                {
                  // moving global var into an outer group, link up with variable of same name (if existing)
                  if (v->name()[0]==':')
                    for (auto& i: items)
                      if (auto vv=i->variableCast())
                        if (vv->name()==v->name().substr(1))
                          v->name(v->name().substr(1));
                }
              else
                // moving between unrelated groups
                if (v->name()[0]==':' && VariableValue::valueId(destGroup,v->name()) !=
                    VariableValue::valueId(origGroup,v->name()))
                  // maintain linkage if possible, otherwise make local
                  v->name(v->name().substr(1));
            }
        v->init(init); //NB calls ensureValueExists()
      }
     
    // move wire to highest common group
    for (auto& p: it->ports)
      {
        assert(p);
        for (auto& w: p->wires())
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
                addItem(oldG->removeItem(*intOp->intVar),inSchema);
            }
          else
            addItem(intOp->intVar,inSchema);
          if (intOp->coupled())
            intOp->intVar->controller=it;
          else
            intOp->intVar->controller.reset();
        }
         
    items.push_back(it);
    return items.back();
  }

  void GroupItems::adjustWiresGroup(Wire& w)
  {
    // Find common ancestor group, and move wire to it
    assert(w.from() && w.to());
    shared_ptr<Group> p1=w.from()->item().group.lock(), p2=w.to()->item().group.lock();
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
  
  void Group::splitBoundaryCrossingWires()
  {
    // Wire::split will invalidate the Items::iterator, so collect
    // wires to split first
    set<Wire*> wiresToSplit;
    for (auto& i: items)
      for (auto& p: i->ports)
        for (auto w: p->wires())
          wiresToSplit.insert(w);

    for (auto w: wiresToSplit)
      w->split();

    // check if any created I/O variables can be removed
    auto varsToCheck=createdIOvariables;
    for (auto& iv: varsToCheck)
      {
        assert(iv->ports[1]->input() && !iv->ports[1]->multiWireAllowed());
        // firstly join wires that don't cross boundaries
        // determine if this is input or output var
        if (iv->ports[1]->wires().size()>0)
          {
            auto fromGroup=iv->ports[1]->wires()[0]->from()->item().group.lock();
            if (fromGroup.get() == this)
              {
                // not an input var
                for (auto& w: iv->ports[0]->wires())
                  if (w->to()->item().group.lock().get() == this)
                    // join wires, as not crossing boundary
                    {
                      auto to=w->to();
                      iv->ports[0]->eraseWire(w);
                      removeWire(*w);
                      addWire(iv->ports[1]->wires()[0]->from(), to);
                    }
              }
            else
              for (auto& w: iv->ports[0]->wires())
                if (w->to()->item().group.lock() == fromGroup)
                  // join wires, as not crossing boundary
                  {
                    auto to=w->to();
                    iv->ports[0]->eraseWire(w);
                    globalGroup().removeWire(*w);
                    adjustWiresGroup(*addWire(iv->ports[1]->wires()[0]->from(), to));
                  }
          }
        
        if (iv->ports[0]->wires().empty() || iv->ports[1]->wires().empty())
          removeItem(*iv);
      }
  }

  size_t GroupItems::numItems() const
  {
    size_t count=items.size();
    for (auto& i: groups) count+=i->numItems();
    return count;
  }

  size_t GroupItems::numWires() const
  {
    size_t count=wires.size();
    for (auto& i: groups) count+=i->numWires();
    return count;
  }

  size_t GroupItems::numGroups() const
  {
    size_t count=groups.size();
    for (auto& i: groups) count+=i->numGroups();
    return count;
  }


  void Group::moveContents(Group& source) {
     if (&source!=this)
       {
         if (source.higher(*this))
             throw error("attempt to move a group into itself");
         // make temporary copies as addItem removes originals
         auto copyOfItems=source.items;
         for (auto& i: copyOfItems)
           {
             addItem(i);
             assert(!i->ioVar());
           }
         auto copyOfGroups=source.groups;
         for (auto& i: copyOfGroups)
           addGroup(i);
         /// no need to move wires, as these are handled above
         source.clear();
       }
  }

  VariablePtr Group::addIOVar()
  {
    VariablePtr v(VariableType::flow,
                  VariableValue::uqName(cminsky().variableValues.newName(to_string(size_t(this))+":")));
    addItem(v,true);
    createdIOvariables.push_back(v);
    v->rotation(rotation());
    v->controller=self;
    bb.update(*this);
    return v;
  }
  
  Group::IORegion::type Group::inIORegion(float x, float y) const
  {
    float left, right, z=zoomFactor();
    margins(left,right);    
    float dx=(x-this->x())*cos(rotation()*M_PI/180)-
      (y-this->y())*sin(rotation()*M_PI/180);
    float dy=(x-this->x())*sin(rotation()*M_PI/180)+
      (y-this->y())*cos(rotation()*M_PI/180);      
    float w=0.5*iconWidth*z,h=0.5*iconHeight*z;
    if (w-right*edgeScale()<dx)
      return IORegion::output;
    else if (-w+left*edgeScale()>dx)
      return IORegion::input;
    else if ((-h-topMargin*z<dy && dy<0) || (h+topMargin*z>dy && dy>0))     
      return IORegion::topBottom;  
    else     
      return IORegion::none;
  }

  void Group::checkAddIORegion(const ItemPtr& x)
  {
    if (auto v=dynamic_pointer_cast<VariableBase>(x))
      {
        remove(inVariables, v);
        remove(outVariables, v);
        switch (inIORegion(v->x(),v->y()))
          {
          case IORegion::input:
            inVariables.push_back(v);
            v->controller=self;
            break;
          case IORegion::output:
            outVariables.push_back(v);
            v->controller=self;
            break;
          default:
            v->controller.reset();
            break;
          }
      }
  }

  
  void Group::resizeOnContents()
  {
    double x0, x1, y0, y1;
    contentBounds(x0,y0,x1,y1);
    double xx=0.5*(x0+x1), yy=0.5*(y0+y1);
    double dx=xx-x(), dy=yy-y();
    float l,r; margins(l,r);    
    float z=zoomFactor();
    iconWidth=((x1-x0)+l+r)/z;
    iconHeight=((y1-y0)+20*z)/z;

    // adjust contents by the offset
    for (auto& i: items)
      i->moveTo(i->x()-dx, i->y()-dy);
    for (auto& i: groups)
      i->moveTo(i->x()-dx, i->y()-dy);

    moveTo(xx,yy);
    bb.update(*this);
  }

  namespace
  {
    template <class T>
    void resizeItems(T& items, double sx, double sy)
    {
      for (auto& i: items)
        {
          i->m_x*=sx;
          i->m_y*=sy;
          //i->zoomFactor*=std::max(sx,sy);
        }
    }
  }
  
  void Group::resize(const LassoBox& b)
  {
    float z=zoomFactor();
    iconWidth=fabs(b.x0-b.x1)/z;
    iconHeight=(fabs(b.y0-b.y1)-2*topMargin)/z;
    // account for margins
    float l, r;
    margins(l,r);    
    // rescale contents to fit
    double x0, x1, y0, y1;
    contentBounds(x0,y0,x1,y1);
    double sx=(fabs(b.x0-b.x1)-z*(l+r)>0? fabs(b.x0-b.x1)-z*(l+r) : z*(l+r) )/(x1-x0), sy=(fabs(b.y0-b.y1)-20*z>0 ? fabs(b.y0-b.y1)-20*z : 20*z )/(y1-y0);    
    resizeItems(items,sx,sy);
    resizeItems(groups,sx,sy);
    moveTo(0.5*(b.x0+b.x1), 0.5*(b.y0+b.y1));
    bb.update(*this);
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
    assert(g);
    auto origGroup=g->group.lock();
    if (origGroup.get()==this) return g; // nothing to do
    if (origGroup)
      origGroup->removeGroup(*g);
    groups.push_back(g);
    g->group=self;
    g->self=groups.back();
    assert(nocycles());
    return groups.back();
  }

  WirePtr GroupItems::addWire(const std::shared_ptr<Wire>& w)
  {
    assert(w->from() && w->to());
    wires.push_back(w);
    return wires.back();
  }
  WirePtr GroupItems::addWire
  (const shared_ptr<Port>& fromP, const shared_ptr<Port>& toP, const vector<float>& coords)
  {
    // disallow self-wiring
    if (&fromP->item()==&toP->item()) 
      return WirePtr();

    // wire must go from an output port to an input port
    if (fromP->input() || !toP->input())
      return WirePtr();

    // check that multiple input wires are only to binary ops.
    if (toP->wires().size()>=1 && !toP->multiWireAllowed())
      return WirePtr();

    // check that a wire doesn't already exist connecting these two ports
    for (auto& w: toP->wires())
      if (w->from()==fromP)
        return WirePtr();

    // disallow wiring the input of an already defined variable
    if (auto v=toP->item().variableCast())
      if (cminsky().inputWired(v->valueId()))
        return {};
    
    auto w=addWire(new Wire(fromP, toP, coords));
    adjustWiresGroup(*w);

    return w;
  }


  bool Group::higher(const Group& x) const
  {
    for (auto i: groups)
      if (i.get()==&x) return true;
    for (auto i: groups)
      if (i->higher(x))
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
      for (auto i=start.group.lock(); i; i=i->group.lock())
        g=i.get();
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

    for (auto& i: items)
      if (!i->ioVar())
        {
          if (i->left()<x0) x0=i->left();
          if (i->right()>x1) x1=i->right();
          if (i->bottom()<y0) y0=i->bottom();
          if (i->top()>y1) y1=i->top();
        }
    for (auto& i: groups)
      {
        if (i->left()<x0) x0=i->left();
        if (i->right()>x1) x1=i->right();
        if (i->bottom()<y0) y0=i->bottom();
        if (i->top()>y1) y1=i->top();
      }
   
    // if there are no contents, result is not finite. In this case,
    // set the content bounds to a 10x10 sized box around the centroid of the I/O variables.

    if (x0==numeric_limits<float>::max())
      {
        float cx=0, cy=0;
        for (auto& i: inVariables)
          {
            cx+=i->x();
            cy+=i->y();
          }
        for (auto& i: outVariables)
          {
            cx+=i->x();
            cy+=i->y();
          }
        int n=inVariables.size()+outVariables.size();
        if (n>0)
          {
            cx/=n;
            cy/=n;
          }
        x0=cx-10;
        x1=cx+10;
        y0=cy-10;
        y1=cy+10;
      }

    return localZoom;
  }

  float Group::computeDisplayZoom()
  {
    double x0, x1, y0, y1;
    //float l, r, t, bm;
    float l, r;    
    float lz=contentBounds(x0,y0,x1,y1);
    x0=min(x0,double(x()));
    x1=max(x1,double(x()));
    y0=min(y0,double(y()));
    y1=max(y1,double(y()));
    // first compute the value assuming margins are of zero width
    displayZoom = 2*max( max(x1-x(), x()-x0)/iconWidth, max(y1-y(), y()-y0)/iconHeight );

    // account for shrinking margins
    float readjust=zoomFactor()/edgeScale() / (displayZoom>1? displayZoom:1);
    margins(l,r);
    l*=readjust; r*=readjust;    
    displayZoom = max(displayZoom, 
                  float(max((x1-x())/(0.5f*iconWidth-r), (x()-x0)/(0.5f*iconWidth-l))));
  
    displayZoom*=1.1*rotFactor()/lz;

    // displayZoom should never be less than 1
    displayZoom=max(displayZoom, 1.0f);
    return displayZoom;
  }

  void Group::computeRelZoom()
  {
    double x0, x1, y0, y1, z=zoomFactor();
    relZoom=1;
    contentBounds(x0,y0,x1,y1);
    float l, r;
    margins(l,r);    
    double dx=x1-x0, dy=y1-y0;
    if (iconWidth-l-r>0 && dx>0 && dy>0)
      relZoom=std::min(1.0, std::min((iconWidth-l-r)/(z*dx), (iconHeight-20*z)/(z*dy)));    
  }
  
  const Group* Group::minimalEnclosingGroup(float x0, float y0, float x1, float y1, const Item* ignore) const
  {
    float z=zoomFactor();
    if (x0<x()-0.5*z*iconWidth || x1>x()+0.5*z*iconWidth || 
        y0<y()-0.5*z*iconHeight || y1>y()+0.5*z*iconHeight)
      return nullptr;
    // at this point, this is a candidate. Check if any child groups are also
    for (auto& g: groups)
      if (auto mg=g->minimalEnclosingGroup(x0,y0,x1,y1, ignore))
        if (mg->visible())
          return mg;
    return this!=ignore? this: nullptr;
  }

  void Group::setZoom(float factor)
  {
    bool dpc=displayContents();
    //    zoomFactor=factor;
    if (!group.lock())
      relZoom=factor;
    else
      computeRelZoom();
    float lzoom=localZoom();
//    for (auto& i: items)
//      i->zoomFactor=lzoom;
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
    minsky::zoom(m_x,xOrigin+m_x-x(),factor);
    minsky::zoom(m_y,yOrigin+m_y-y(),factor);
    m_displayContentsChanged = dpc!=displayContents();
    if (!group.lock())
      relZoom*=factor;
    for (auto& i: groups)
      {
        if (displayContents() && !m_displayContentsChanged)
          i->zoom(i->x(), i->y(), factor);
        m_displayContentsChanged|=i->displayContentsChanged();
      }
  }

  ClickType::Type Group::clickType(float x, float y)
  {
    double dx=x-this->x(), dy=y-this->y();
    auto z=zoomFactor();
    double w=0.5*iconWidth*z, h=0.5*iconHeight*z;
    // check if (x,y) is within portradius of the 4 corners
    if ((abs(x-left()) < portRadius*z || abs(x-right()) < portRadius*z) &&
      (abs(y-top()) < portRadius*z || abs(y-bottom()*z) < portRadius*z))
      return ClickType::onResize;         
    if (displayContents() && inIORegion(x,y)==IORegion::none)
      return ClickType::outside;
    if (auto item=select(x,y))
      return item->clickType(x,y);
    if ((abs(x-this->x())<w && abs(y-this->y())<h) || inIORegion(x,y)==IORegion::topBottom) // check also if (x,y) is within top and bottom margins of group. for feature 88
      return ClickType::onItem;
    return ClickType::outside;
  }

  void Group::draw(cairo_t* cairo) const
  {
    double angle=rotation() * M_PI / 180.0;

    // determine how big the group icon should be to allow
    // sufficient space around the side for the edge variables
    float leftMargin, rightMargin;
    margins(leftMargin, rightMargin);    
    float z=zoomFactor();
    leftMargin*=edgeScale(); rightMargin*=edgeScale();

    unsigned width=z*this->iconWidth, height=z*this->iconHeight;

    {
      // draw default group icon
      cairo::CairoSave cs(cairo);

      // display I/O region in grey
      drawIORegion(cairo);

      cairo_translate(cairo, -0.5*width+leftMargin, -0.5*height);


              
      double scalex=double(width-leftMargin-rightMargin)/width;
      cairo_scale(cairo, scalex, 1);

      // draw a simple frame 
      cairo_rectangle(cairo,0,0,width,height);
      {
        cairo::CairoSave cs(cairo);
        cairo_identity_matrix(cairo);
        cairo_set_line_width(cairo,1);
        cairo_stroke(cairo);
      }

      if (!displayContents())
        {
          if (displayPlot)
            {
              displayPlot->Plot::draw(cairo, width, height);
            }
          else
            {
              cairo_scale(cairo,width/svgRenderer.width(),height/svgRenderer.height());
              cairo_rectangle(cairo,0, 0,svgRenderer.width(), svgRenderer.height());
              cairo_clip(cairo);
              svgRenderer.render(cairo);
            }
        }
    }

    drawEdgeVariables(cairo);


    // display text label
    if (!title.empty())
      {
        cairo::CairoSave cs(cairo);
        cairo_scale(cairo, z, z);
        cairo_select_font_face
          (cairo, "sans-serif", CAIRO_FONT_SLANT_ITALIC, 
           CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cairo,12);
              
        // extract the bounding box of the text
        cairo_text_extents_t bbox;
        cairo_text_extents(cairo,title.c_str(),&bbox);       
        double w=0.5*bbox.width+2; 
        double h=0.5*bbox.height+5;
        double fm=std::fmod(rotation(),360);

        // if rotation is in 1st or 3rd quadrant, rotate as
        // normal, otherwise flip the text so it reads L->R
        if ((fm>-90 && fm<90) || fm>270 || fm<-270)
          cairo_rotate(cairo, angle);
        else
          cairo_rotate(cairo, angle+M_PI);

        // prepare a background for the text, partially obscuring graphic
        double transparency=displayContents()? 0.25: 1;

        // display text
        cairo_move_to(cairo, -w+1, h-12-0.5*(height)/z);
        cairo_set_source_rgba(cairo,0,0,0,transparency);
        cairo_show_text(cairo,title.c_str());
      }

    if (mouseFocus)
      {
        displayTooltip(cairo,tooltip);
        // Resize handles always visible on mousefocus. For ticket 92.
        drawResizeHandles(cairo);
      }

    cairo_rectangle(cairo,-0.5*width,-0.5*height,width,height);
    cairo_clip(cairo);
    if (selected)
      drawSelected(cairo);
  }

  namespace
  {
    // ratio of variable size to text height
    const float varToTextRatio=1.8;
  }
  
  void Group::draw1edge(const vector<VariablePtr>& vars, cairo_t* cairo, 
                        float x) const
  {
    float top=0, bottom=0, z=zoomFactor();
    for (size_t i=0; i<vars.size(); ++i)
      {
        float y=i%2? top:bottom;
        Rotate r(rotation(),0,0);
        auto& v=vars[i];
        v->moveTo(r.x(x,y)+this->x(), r.y(x,y)+this->y());
        v->rotation(rotation());
        cairo::CairoSave cs(cairo);
        cairo_translate(cairo,x,y);
        cairo_rotate(cairo,M_PI*rotation()/180);
        v->draw(cairo);
        if (i==0)
          {
            top=0.5*varToTextRatio*v->height()*z; //??? should be 0.5*varToTextRatio
            bottom=-top;
          }
        else if (i%2)
          top+=0.5*varToTextRatio*v->height()*z;
        else
          bottom-=0.5*varToTextRatio*v->height()*z;
      }
  }

  void Group::drawEdgeVariables(cairo_t* cairo) const
  {
    float left, right; margins(left,right);    
    left*=edgeScale();
    right*=edgeScale();
    cairo::CairoSave cs(cairo);
    cairo_rotate(cairo,-M_PI*rotation()/180);
    float z=zoomFactor();
    draw1edge(inVariables, cairo, -0.5*(z*iconWidth-left));
    draw1edge(outVariables, cairo, 0.5*(z*iconWidth-right));
  }

  // draw notches in the I/O region to indicate docking capability
  void Group::drawIORegion(cairo_t* cairo) const
  {
    cairo_save(cairo);
    float left, right, z=zoomFactor();
    margins(left,right);    
    left*=edgeScale();
    right*=edgeScale();
    float y=0, dy=topMargin*edgeScale();
    for (auto& i: inVariables)
      {
        RenderVariable rv(*i);
        y=max(y, fabs(i->y()-this->y())+varToTextRatio*rv.height()*edgeScale());
      }
    cairo_set_source_rgba(cairo,0,1,1,0.5);
    float w=0.5*z*iconWidth, h=0.5*z*iconHeight;
    cairo_rotate(cairo,rotation()*M_PI/180);
    
    cairo_move_to(cairo,-w,-h);
    // create notch in input region
    cairo_line_to(cairo,-w,y-dy);
    cairo_line_to(cairo,left-w-4*z,y-dy);
    cairo_line_to(cairo,left-w,y);
    cairo_line_to(cairo,left-w-4*z,y+dy);
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
        y=max(y, fabs(i->y()-this->y())+varToTextRatio*rv.height()*edgeScale());
      }
    cairo_move_to(cairo,w,-h);
    // create notch in output region
    cairo_line_to(cairo,w,y-dy);
    cairo_line_to(cairo,w-right-2*z,y-dy);
    cairo_line_to(cairo,w-right+2*z,y);
    cairo_line_to(cairo,w-right-2*z,y+dy);
    cairo_line_to(cairo,w,y+dy);
    cairo_line_to(cairo,w,h);
    cairo_line_to(cairo,w-right,h);
    cairo_line_to(cairo,w-right,-h);
    cairo_close_path(cairo);
    cairo_fill(cairo);
    
    // draw top margin. for feature 88
    cairo_rectangle(cairo,-w,-h,2*w,-topMargin*z);
    cairo_fill(cairo);    
    
    // draw bottom margin. for feature 88
    cairo_rectangle(cairo,-w,h,2*w,topMargin*z);
    cairo_fill(cairo);    
    
    cairo_restore(cairo);
  }


  void Group::margins(float& left, float& right) const
  {
    left=right=10;
    auto tmpMouseFocus=mouseFocus;
    mouseFocus=false; // disable mouseFocus for this calculation
    for (auto& i: inVariables)
      {
        assert(i->type()!=VariableType::undefined);
        i->bb.update(*i);
        if (i->width()>left) left=i->width();
      }
    for (auto& i: outVariables)
      {
        assert(i->type()!=VariableType::undefined);
        i->bb.update(*i);
        if (i->width()>right) right=i->width();
      }
    mouseFocus=tmpMouseFocus;
  }  

  float Group::rotFactor() const
  {
    float rotFactor;
    float ac=abs(cos(rotation()*M_PI/180));
    static const float invSqrt2=1/sqrt(2);
    if (ac>=invSqrt2) 
      rotFactor=1.15/ac; //use |1/cos(angle)| as rotation factor
    else
      rotFactor=1.15/sqrt(1-ac*ac);//use |1/sin(angle)| as rotation factor
    return rotFactor;
  }

  ItemPtr Group::select(float x, float y) const
  {
    for (auto& v: inVariables)
      if (RenderVariable(*v).inImage(x,y)) 
        return v;
    for (auto& v: outVariables)
      if (RenderVariable(*v).inImage(x,y)) 
        return v;
    return nullptr;
  }

  void Group::normaliseGroupRefs(const shared_ptr<Group>& self)
  {
    for (auto& i: items)
        i->group=self;
    for (auto& g: groups)
      {
        g->group=self;
        g->normaliseGroupRefs(g);
      }
  }

  
  void Group::flipContents()
  {
    for (auto& i: items)
      {
        i->moveTo(x()-i->m_x,i->y());
        i->flip();
      }
    for (auto& i: groups)
      {
        i->moveTo(x()-i->m_x,i->y());
        i->flip();
      }
  }

}
