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
#include <cairo_base.h>
#include <ecolab_epilogue.h>
using namespace std;
using namespace ecolab::cairo;

namespace minsky
{
  Group& GroupPtr::operator*() const {return dynamic_cast<Group&>(ItemPtr::operator*());}
  Group* GroupPtr::operator->() const {return dynamic_cast<Group*>(ItemPtr::operator->());}


  ItemPtr Group::addItem(int id, const shared_ptr<Item>& it)
  {
    if (auto x=dynamic_pointer_cast<Group>(it))
      {
        addGroup(id,x);
        return it;
      }
   
    auto origGroup=it->group.lock();
    assert(origGroup);
    if (origGroup.get()!=this)
      origGroup->items.erase(id);

    if (self.lock())
      it->group=self;
    else
      it->group.reset();

    // move wire to highest common group
    // TODO add in I/O variables if needed, and move wires to same group
    for (auto& p: it->ports)
      {
        assert(p);
        for (auto& w: p->wires)
          {
            assert(w);
            GroupPtr otherGroup;
            if (p->input())
              {
                if (auto from=w->from())
                  otherGroup=from->group();
                assert(p==w->to());
              }
            else
              {
                if (auto to=w->to())
                  otherGroup=to->group();
                assert(p==w->from());
              }
            // if origGroup is null, then it is global
            if (origGroup && otherGroup)
              {
                if (origGroup->higher(*otherGroup))
                  {
                    if (higher(*otherGroup))
                      w->moveGroup(*origGroup,*this);
                    else
                      w->moveGroup(*origGroup,*otherGroup);
                  }
                else
                  if (higher(*otherGroup))
                    w->moveGroup(*otherGroup,*this);
              }
          }
      }

    return *items.insert(Items::value_type(id,it)).first;
  }

  void Group::moveContents(Group& source) {
     if (&source!=this)
       {
         for (auto& i: source.groups)
           if (i->higher(*this))
             throw error("attempt to move a group into itself");
          for (auto& i: source.items)
            addItem(i);
          for (auto& i: source.groups)
            addGroup(i);
          /// no need to move wires, as these are handled above
          source.clear();
       }
  }

  GroupPtr& Group::addGroup(int id, const std::shared_ptr<Group>& g)
  {
    assert(g->group.lock());
    if (auto origGroup=g->group.lock())
      if (origGroup.get()!=this)
        origGroup->groups.erase(id);
    g->group=self;
    g->self=g;
    return *groups.insert(Groups::value_type(id,g)).first;
  }

  WirePtr& Group::addWire(int id, const std::shared_ptr<Wire>& w)
  {
    return *wires.insert(Wires::value_type(id,w)).first;
  }

  template <class T, class C> 
  const ItemPtr& Group::findItem(C c) const
  {
    for (auto& i: items)
      if (c(i))
        return i;

    for (auto& g: groups)
      if (c(g))
        return g;

    for (auto& g: groups)
      if (auto& i=g->findItem<T>(c))
        return i;

    static ItemPtr nullItem;
    return nullItem;
  }

  const ItemPtr& Group::findItem(int id) const
  {return findItem<int>([&](const ItemPtr& i){return i.id()==id;});}
  const ItemPtr& Group::findItem(const Item& it) const
  {return findItem<const Item&>([&](const ItemPtr& i){return i.get()==&it;});}

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

  const Group& Group::globalGroup() const
  {
    auto g=this;
    while (auto g1=g->group.lock())
      g=g1.get();
    return *g;
  }


  bool Group::uniqueKeys(set<int>& idset) const
  {
    for (auto& i: items)
      if (!idset.insert(i.id()).second) return false;
    for (auto& i: wires)
      if (!idset.insert(i.id()).second) return false;
    for (auto& i: groups)
      if (!idset.insert(i.id()).second || !i->uniqueKeys(idset)) 
        return false;
  }

  float Group::contentBounds(double& x0, double& y0, double& x1, double& y1) const
  {
    float localZoom=1;
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
          localZoom=i->zoomFactor;
        }
      catch (const std::exception& e) 
        {cerr<<"illegal exception caught in draw()"<<e.what()<<endl;}
      catch (...) {cerr<<"illegal exception caught in draw()";}
    cairo_recording_surface_ink_extents(surf->surface(),
                                        &x0,&y0,&x1,&y1);

    for (auto& i: groups)
      {
        float w=0.5f*i->width*i->zoomFactor,
          h=0.5f*i->height*i->zoomFactor;
        x0=min(i->x()-0.5*w, x0);
        x1=max(i->x()+0.5*w, x1);
        y0=min(i->y()-0.5*h, y0);
        y1=max(i->x()+0.5*h, y1);
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
    else
      {
        // extend width by 2 pixels to allow for the slightly oversized variable icons
        x0-=2*this->localZoom();
        y0-=2*this->localZoom();
        x1+=2*this->localZoom();
        y1+=2*this->localZoom();
      }

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
    //TODO    margins(l,r);
    l*=readjust; r*=readjust;
    displayZoom = max(displayZoom, 
                      float(max((x1-x())/(0.5f*width-r), (x()-x0)/(0.5f*width-l))));
  
    // displayZoom*=1.1*rotFactor()/lz;

    // displayZoom should never be less than 1
    displayZoom=max(displayZoom, 1.0f);
    return displayZoom;
  }

  int Group::maxId() const
  {
    int r=-1;
    if (!items.empty()) r=items.rbegin()->id();
    if (!wires.empty()) r=max(r,wires.rbegin()->id());
    if (!groups.empty()) r=max(r,groups.rbegin()->id()); 
    for (auto& g: groups) r=max(r,g->maxId());
  }

}
