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

#include "geometry.h"
#include "wire.h"
#include "port.h"
#include "group.h"
#include "selection.h"
#include <ecolab_epilogue.h>

using namespace std;

// undocumented internal function in the Tk library
extern "C" int TkMakeBezierCurve(Tk_Canvas,double*,int,int,void*,double*);

namespace minsky
{
  vector<float> Wire::_coords() const
  {
    vector<float> c;
    assert(from() && to());
    assert(m_coords.size() % 2 == 0);
    if (auto f=from())
      if (auto t=to())
        {
          c.push_back(f->x());
          c.push_back(f->y());
          float d=sqrt
            (sqr(f->x()-t->x())+sqr(f->y()-t->y()));

          for (size_t i=0; m_coords.size()>1 && i<m_coords.size()-1; i+=2)
            {
              c.push_back(f->x() + d*m_coords[i]);
              c.push_back(f->y() + d*m_coords[i+1]);
            }
          c.push_back(t->x());
          c.push_back(t->y());
        }
    return c;
  }

  vector<float> Wire::_coords(const vector<float>& coords)
  {
    if (coords.size()<6) 
      m_coords.clear();
    else
      {
        assert(coords.size() % 2 == 0);
        
        float d=1/sqrt
          (sqr(coords[coords.size()-2]-coords[0])+sqr(coords[coords.size()-1]-coords[1]));
        m_coords.resize(coords.size()-4);
        for (size_t i=2; i<coords.size()-3; i+=2)
          {
            m_coords[i-2] = (coords[i]-coords[0])*d;
            m_coords[i-1] = (coords[i+1]-coords[1])*d;
          }
      }
    return this->coords();
  }


  Wire::Wire(const shared_ptr<Port>& from, const shared_ptr<Port>& to, 
         const vector<float>& a_coords): 
      m_from(from), m_to(to) 
  {
    if (!from || !to) throw error("wiring defunct ports");
    if (from->input() || !to->input()) throw error("invalid ports for wire");
    coords(a_coords);
    m_from.lock()->m_wires.push_back(this);
    m_to.lock()->m_wires.push_back(this);
  }

  Wire::~Wire()
  {
    if (auto toPort=to())
      toPort->eraseWire(this);
    if (auto fromPort=from())
      fromPort->eraseWire(this);
  }

  bool Wire::visible() const
  {
    auto f=from(), t=to();
    assert(f->item.group.lock() && t->item.group.lock());
    return f && t &&
      (f->item.group.lock()->displayContents() ||
       t->item.group.lock()->displayContents());
  }

  void Wire::moveToPorts(const shared_ptr<Port>& from, const shared_ptr<Port>& to)
  {
    if (auto f=this->from())
      f->m_wires.erase(remove(f->m_wires.begin(), f->m_wires.end(), this), f->m_wires.end());
    if (auto t=this->to())
      t->m_wires.erase(remove(t->m_wires.begin(), t->m_wires.end(), this), t->m_wires.end());
    m_from=from;
    m_to=to;
    from->m_wires.push_back(this);
    to->m_wires.push_back(this);
  }

  
  void Wire::moveIntoGroup(Group& dest)
  {
    WirePtr wp;
    // one hit find and remove wire from its map, saving the wire
    dest.globalGroup().recursiveDo
      (&Group::wires, 
       [&](Wires& wires, Wires::iterator i) {
        if (i->get()==this) 
          {
            wp=*i;
            wires.erase(i);
            return true;
          }
        else
          return false;
      }); 
    if (wp)
      dest.addWire(wp);
  }

  void Wire::draw(cairo_t* cairo) const
  {
    auto coords=this->coords();
    if (coords.size()<4 || !visible()) return;

    double angle, lastx, lasty;
    if (coords.size()==4)
      {
        cairo_move_to(cairo,coords[0],coords[1]);
        cairo_line_to(cairo, coords[2], coords[3]);
        angle=atan2(coords[3]-coords[1], coords[2]-coords[0]);
        lastx=coords[2]; lasty=coords[3];
      }
    else
      {
        // need to convert to double precision for Tk
        vector<double> dcoords(coords.begin(), coords.end());
        // Use Tk's smoothing algorithm for computing curves
        const int numSteps=100;
        // Tk's documentation doesn't say how big this buffer should
        // be, hopefully this is ample.
        vector<double> points(2*numSteps*(dcoords.size()+1));
        // TODO - find a way of doing this that doesn't involve Tk!
        int numPoints=
          TkMakeBezierCurve(0,dcoords.data(),dcoords.size()/2,numSteps,
                            nullptr,points.data());
        
        cairo_move_to(cairo, points[0], points[1]);
        for (int i=2; i<2*numPoints; i+=2)
          cairo_line_to(cairo, points[i], points[i+1]);
        cairo_stroke(cairo);
        angle=atan2(points[2*numPoints-1]-points[2*numPoints-3], 
                    points[2*numPoints-2]-points[2*numPoints-4]);
        lastx=points[2*numPoints-2]; lasty=points[2*numPoints-1];
      }
    cairo_stroke(cairo);

    // draw arrow
    cairo_save(cairo);
    cairo_translate(cairo, lastx, lasty);
    cairo_rotate(cairo,angle);
    cairo_move_to(cairo,0,0);
    cairo_line_to(cairo,-5,-3); 
    cairo_line_to(cairo,-3,0); 
    cairo_line_to(cairo,-5,3);
    cairo_close_path(cairo);
    cairo_fill(cairo);
    cairo_restore(cairo);

    // draw handles
    if (mouseFocus)
      {
        cairo_save(cairo);
        cairo_set_source_rgb(cairo,0,0,1);
        for (size_t i=0; i<coords.size()-3; i+=2)
          {
            double midx=0.5*(coords[i]+coords[i+2]);
            double midy=0.5*(coords[i+1]+coords[i+3]);
            cairo_arc(cairo,midx,midy,handleRadius, 0, 2*M_PI);
            if (i>0) // draw existing interor handle
              cairo_arc(cairo,coords[i],coords[i+1],handleRadius, 0, 2*M_PI);
            cairo_fill(cairo);
          }
        cairo_restore(cairo);
      }
  }

  void Wire::split()
  {
    // add I/O variables if this wire crosses a group boundary
    if (auto fg=from()->item.group.lock())
      if (auto tg=to()->item.group.lock())
        if (fg!=tg && !from()->item.ioVar() && !to()->item.ioVar()) // crosses boundary
          {
            // check if this wire is in from group
            auto cmp=[&](WirePtr w) {return w.get()==this;};
            auto i=find_if(fg->wires.begin(), fg->wires.end(), cmp);
            if (i==fg->wires.end())
              {
                fg->addOutputVar();
                assert(fg->outVariables.back()->ports.size()>1);
                fg->addWire(new Wire(from(),fg->outVariables.back()->ports[1]));
                moveToPorts(fg->outVariables.back()->ports[0], to());
              }
            // check if this wire is in to group
            i=find_if(tg->wires.begin(), tg->wires.end(), cmp);
            if (i==tg->wires.end())
              {
                tg->addInputVar();
                assert(tg->inVariables.back()->ports.size()>1);
                tg->addWire(new Wire(tg->inVariables.back()->ports[0],to()));
                moveToPorts(from(), tg->inVariables.back()->ports[1]);
              }
          }
  }

  Units Wire::units() const
  {
      
    if (auto f=from())
      {
        // we allow possible traversing twice over a wire, to allow an
        // integral to break the cycle
        if (unitsCtr>2)
          f->item.throw_error("wiring loop detected");
        IncrDecrCounter idc(unitsCtr);
        return f->item.units();
      }
    else return {};
  }

  namespace
  {

    // returns true if x,y lies close to the line segment (x0,y0)-(x1,y1)
    bool segNear(float x0, float y0, float x1, float y1, float x, float y)
    {
      float d=sqrt(sqr(x1-x0)+sqr(y1-y0));
      float d1=sqrt(sqr(x-x0)+sqr(y-y0)), d2=sqrt(sqr(x-x1)+sqr(y-y1));
      return d1+d2<=d+5;
    }
    
    // returns true if x,y lies in the triangle (x0,y0),(x1,y1),(x2,y2)
    bool inTriangle(float x0, float y0, float x1, float y1,
                    float x2, float y2, float x, float y)
    {
      float l1 = (x-x0)*(y2-y0) - (x2-x0)*(y-y0), 
        l2 = (x-x1)*(y0-y1) - (x0-x1)*(y-y1), 
        l3 = (x-x2)*(y1-y2) - (x1-x2)*(y-y2);
      return (l1>0 && l2>0  && l3>0) || (l1<0 && l2<0 && l3<0);
    }

    inline float d2(float x0, float y0, float x1, float y1)
    {return sqr(x0-x1)+sqr(y0-y1);}
  }
  
  bool Wire::near(float x, float y) const
  {
    auto c=coords();
    assert(c.size()>=4);
    if (c.size()==4)
      return segNear(c[0],c[1],c[2],c[3],x,y);
    else
      for (size_t i=0; i<c.size()-4; i+=2)
        if (inTriangle(c[i],c[i+1],c[i+2],c[i+3],c[i+4],c[i+5],x,y))
          return true;
    return false;
  }

  unsigned Wire::nearestHandle(float x, float y)
  {
    auto c=coords();
    unsigned n=0; // nearest index
    float closestD=d2(c[0],c[1],x,y);
    for (size_t i=2; i<c.size()-1; i+=2)
      {
        float d=d2(c[i],c[i+1],x,y);
        if (d<closestD)
          {
            closestD=d;
            n=i;
          }
      }
    // now work out if we need to insert a midpoint handle
    if (n>0)
      {
        float mx=0.5*(c[n]+c[n-2]), my=0.5*(c[n+1]+c[n-1]);
        float d=d2(mx,my,x,y);
        if (n==c.size()-2 || d<closestD)
          {
            insertHandle((n>>1)-1, mx, my);
            return (n>>1)-1;
          }
      }
    if (n<c.size()-3)
      {
        float mx=0.5*(c[n+2]+c[n]), my=0.5*(c[n+3]+c[n+1]);
        float d=d2(mx,my,x,y);
        if (n==0 || d<closestD)
          {
            insertHandle(n>>1, mx, my);
            return (n>>1);
          }
      }
    return (n>>1)-1;
  }
  
  void Wire::insertHandle(unsigned n, float x, float y)
  {
    n++;
    n*=2;
    auto c=coords();
    assert(n<c.size()-1);
    vector<float> h{x,y};
    c.insert(c.begin()+n,h.begin(), h.end());
    coords(c);
  }
  
  void Wire::editHandle(unsigned position, float x, float y)
  {
    position++;
    position*=2;
    auto c=coords();
    assert(position<c.size()-2);
    c[position]=x;
    c[position+1]=y;
    coords(c);
  }

  
}
