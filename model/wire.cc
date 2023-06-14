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

#include "wire.h"
#include "geometry.h"
#include "port.h"
#include "group.h"
#include "selection.h"
#include "lasso.h"
#include "operation.h"
#include "pango.h"
#include "plotWidget.h"
#include "SVGItem.h"
#include "wire.rcd"
#include "minsky_epilogue.h"
#include  <random>
#include  <iterator>

using namespace std;
using ecolab::cairo::CairoSave;

namespace minsky
{
  WireAccessor::WireAccessor(): ecolab::TCLAccessor<Wire, std::vector<float>>
  ("coords", (Getter)&Wire::coords, (Setter)&Wire::coords)  {}


  vector<float> Wire::coords() const
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

  vector<float> Wire::coords(const vector<float>& coords)
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


  Wire::Wire(const weak_ptr<Port>& from, const weak_ptr<Port>& to, 
         const vector<float>& a_coords): 
      m_from(from), m_to(to) 
  {
    if (!from.lock() || !to.lock()) throw error("wiring defunct ports");
    if (from.lock()->input() || !to.lock()->input()) throw error("invalid ports for wire");
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
    if (attachedToDefiningVar()) return false;	  
    auto f=from(), t=to();
    assert(f->item().group.lock() && t->item().group.lock());
    return f && t && 
      (f->item().group.lock()->displayContents() ||
       t->item().group.lock()->displayContents());
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
        return false;
      }); 
    if (wp)
      dest.addWire(wp);
  }
 
namespace
{

 // For ticket 1095. Returns coordinate pairs for both moving and "to be inserted" handles on a curved wire	
 vector<pair<float,float>> allHandleCoords(vector<float> coords) {
        
        vector<pair<float,float>> points(coords.size()-1);
        
        for (size_t i = 0; i < points.size(); i++) {
			if (i%2 == 0) {
				points[i].first = coords[i];
				points[i].second = coords[i+1];
			}
			else {
				points[i].first = 0.5*(coords[i-1]+coords[i+1]);
				points[i].second = 0.5*(coords[i]+coords[i+2]);
			}
		}						
		
   return points;
 } 	  
 	
 // For ticket 991/1092. Returns coordinate pairs for moving handles on a curved wire	
 vector<pair<float,float>> toCoordPair(vector<float> coords) {    
   vector<pair<float,float>> points(coords.size()/2);
   
   for (size_t i = 0; i < coords.size(); i++)
     if (i%2 == 0) {
     	points[i/2].first = coords[i];
     	points[i/2].second = coords[i+1];
     }
   return points;
 } 	

// For ticket 991. Construct tridoagonal matrix A which relates control points c and knots k (curved wire handles): Ac = k.
 vector<vector<float>> constructTriDiag(int length)
 {
   vector<vector<float>> result(length,vector<float>(length));
   
   for (int i = 0; i < length; i++)  // rows
     for (int j = 0; j < length; j++) {  // columns
 		  
       //construct upper diagonal   
       if (j > 0 && i == j-1 && i < length-1) result[i][j] = 1.0; 	 			
       //construct main diagonal
       else if (i == j) {
         if (i == 0) result[i][j] = 2.0;
         else if (i < length-1) result[i][j] = 4.0;
         else if (i == length-1) result[i][j] = 7.0;
       }   
       //construct lower diagonal
       else if (i == j+1) {
         if (j < length-2) result[i][j] = 1.0;
         else if (j == length-2) result[i][j] = 2.0;
       }  
       else result[i][j] = 0.0; 				
     }
  
   return result;
 } 

// For ticket 991. Vector of input knots k (all the handles on a curved wire).  
  vector<pair<float,float>> constructTargetVector(int n, const vector<pair<float,float>>& knots) {
	  
	assert(knots.size() > 2);  
	  
    vector<pair<float,float>> result(n); 

    result[0].first = knots[0].first+2.0*knots[1].first;
    result[0].second = knots[0].second+2.0*knots[1].second; 
    
    for (int i = 1; i < n - 1; i++) {
      result[i].first = 2.0*(2.0*knots[i].first+(knots[i+1].first)); 
      result[i].second = 2.0*(2.0*knots[i].second+(knots[i+1].second));  
    }
    
    result[result.size() - 1].first = 8.0*knots[n-1].first+knots[n].first; 
    result[result.size() - 1].second = 8.0*knots[n-1].second+knots[n].second;

    return result;
  }  

  // For ticket 991. Applies Thomas' algorithm to the matrix equation Ac = k.
  /**
   * Thomas' algorithm for solving a matrix equation \f$Ac=k\f$ is based on LU-decomposition into \f$LUc=k\f$ where \f$L\f$ is lower triangular and
   *  \f$U\f$ is upper triangular. The matrix equation is solved by setting \f$Uc = k'\f$ and then solving \f$Lk' = k\f$ for \f$k'\f$ first, and subsequently
   *  \f$Uc\f$ for \f$c\f$.
   *  
   * There are two steps in this calculation. Step 1 involves simultaneously decomposing \f$A = LU\f$ and solving \f$Lk'=k\f$ for \f$k'\f$ in a forward sweep, with
   *  \f$Uc=k'\f$ as result. In step 2, \f$Uc=k'\f$ is solved for \f$c\f$ in a backward sweep. The calculation goes as follows for a small system:
   *  
   *      \f[
   *         \left(\begin{array}{ccc} b_1 & a_1 & 0  \\ d_1 & b_2 & a_2  \\  0 & d_2 & b_3 \end{array}\right) \left(\begin{array}{c} c_1 \\ c_2 \\ c_3 \end{array}\right) = \left(\begin{array}{c} k_1 \\ k_2 \\ k_3 \end{array}\right)
   *      \f] 
   *                      
   *        
   * Forward sweep: row 1:
   *  
   *  \f$ b_1c_1+a_1c_2 = k_1 \f$ 
   *  
   *  \f$ c_1+\frac{c_1}{b_1}c_2 = \frac{k_1}{b_1} \f$ 
   *  after dividing through by \f$b_1\f$
   * 
   *  Rewrite row 1 as \f$ c_1 + \alpha_1c_2 = k'_1\f$, 
   * where \f$\alpha_1=\frac{c_1}{b_1}\f$ 
   * and \f$k'_1 = \frac{k_1}{b_1}\f$
   * 
   *  row 2: 
   * 
   *  \f$ d_1c_1+b_2c_2+a_2c_3 = k_2 \f$   
   * 
   * Now eliminate the first term of this equation by subtracting \f$d_1 \times\f$ row 1 from row 2, yielding:
   * 
   *  \f$ (b_2 - d_1\alpha_1)c_2 + a_2c_3 = k_2 - d_1k'_1 \f$
   * 
   * Then, dividing through by \f$(b_2 - d_1\alpha_1)\f$ and rewriting leads to:
   * 
   *  \f$ c_2 + \alpha_2c_3 = k'_2 \f$, where 
   * \f$\alpha_2 = \frac{a_2}{b_2 - d_1\alpha_1}\f$ and 
   * \f$k'_2 = \frac{k_2 - d_1k'_1}{b_2 - d_1\alpha_1}\f$
   * 
   * row 3:
   * 
   * Finally, subtracting \f$d_2 \times\f$ row 2 from row 3, dividing through by \f$(b_3-d_2\alpha_2)\f$, and rewriting, yields:
   * 
   *  \f$ c_3 = k'_3 \f$, where \f$k'_3 = \frac{k_3-d_2k'2}{b_3 - d_2\alpha_2}\f$ 
   * 
   * At this point, the matrix equation has been reduced to
   * 
   *    \f[
   *       \left(\begin{array}{ccc} 1 & \alpha_1 & 0  \\ 0 & 1 & \alpha_2  \\  0 & 0 & 1 \end{array}\right) \left(\begin{array}{c} c_1 \\ c_2 \\ c_3 \end{array}\right) = \left(\begin{array}{c} k'_1 \\  k'_2 \\ k'_3 \end{array}\right)
 *    \f]          
 *     
 * In this form, one can solve for the \f$c_i\f$ in terms of \f$k'_i\f$ directly by sweeping backwards through the matrix equation.
 *      
 * (Source: http://www.industrial-maths.com/ms6021_thomas.pdf)
 */	 
 vector<pair<float,float>> computeControlPoints(vector<vector<float>> triDiag, const vector<pair<float,float>>& knots, vector<pair<float,float>> target) {
  
    assert(knots.size() > 2); 
    
    size_t n = knots.size() - 1;
    
    vector<pair<float,float>> result(2*n); 
 
    // Vector of knots k' after Thomas' algorithm is applied to the initial system Ac = k
    vector<pair<float,float>> newTarget(n); 
    
    // Matrix A' after Thomas' algorithm is applied to the initial system Ac = k
    vector<vector<float>> newTriDiag(n,vector<float>(n));
 
    // forward sweep for control points c_i,0:
    newTriDiag[0][1] = triDiag[0][1]/ triDiag[0][0]; 
 
    newTarget[0].first = target[0].first*(1.0 / triDiag[0][0]);
    newTarget[0].second = target[0].second*(1.0 / triDiag[0][0]);        
    
 
    for (size_t i = 1; i < n-1; i++)
      for (size_t j = 0; j < n; j++)
        if (i == j-1)
          newTriDiag[i][j] = triDiag[i][j] / (triDiag[i][i] - triDiag[i][j-2] * newTriDiag[i-1][j-1]);
 
    for (size_t i = 1; i < n; i++)
      for (size_t j = 0; j < n; j++) {
         if (i == j + 1)
         {
			float targetScale = 1.0/(triDiag[i][i] - triDiag[i][j] * newTriDiag[i-1][j+1]); 
            newTarget[i].first = (target[i].first-(newTarget[i-1].first*(triDiag[i][j])))*(targetScale);
            newTarget[i].second = (target[i].second-(newTarget[i-1].second*(triDiag[i][j])))*(targetScale);
		}
    }
 
    // backward sweep for control points c_i,0:
    result[n-1].first = newTarget[n-1].first;
    result[n-1].second = newTarget[n-1].second;
    
    for (int i = n-2; i >= 0; i--) 
       for (int j = n-1; j >= 0; j--) {		
          if (i == j-1)
          {
             result[i].first = newTarget[i].first-(newTriDiag[i][j]*result[i+1].first);
             result[i].second = newTarget[i].second-(newTriDiag[i][j]*result[i+1].second);
		  }
    }
 
    // calculate remaining control points c_i,1 directly:
    for (size_t i = 0; i < n-1; i++) {
      result[n+i].first = 2.0*knots[i+1].first-(result[i+1].first);
      result[n+i].second = 2.0*knots[i+1].second-(result[i+1].second);
    }
 
    result[2*n-1].first = 0.5*(knots[n].first+(result[n-1].first));
    result[2*n-1].second = 0.5*(knots[n].second+(result[n-1].second));

    return result;
  }

}

  void Wire::storeCairoCoords(cairo_t* cairo) const
  {
    cairoCoords.clear(); 
    cairo_path_t *path;
    cairo_path_data_t *data;
         
    path = cairo_copy_path_flat(cairo);
         
    for (int j=0; j < path->num_data; j += path->data[j].header.length) {
      data = &path->data[j];
      switch (data->header.type) {
      case CAIRO_PATH_MOVE_TO:
        break;
      case CAIRO_PATH_LINE_TO:
        cairoCoords.push_back(make_pair(data[1].point.x,data[1].point.y));
        break;
      case CAIRO_PATH_CURVE_TO:
      case CAIRO_PATH_CLOSE_PATH:
        break;
      }
    }
    cairo_path_destroy (path);              
  }
  
  bool Wire::attachedToDefiningVar(std::set<const Item*>& visited) const
  {
    assert(to());             
    return (to()->item().attachedToDefiningVar(visited));
  }    
   
  void Wire::draw(cairo_t* cairo, bool reverseArrow) const
  {
    auto coords=this->coords();
    if (coords.size()<4 || !visible()) return;

    float angle, lastx, lasty;
    if (coords.size()==4)
      {
        cairo_move_to(cairo,coords[0],coords[1]);
        cairo_line_to(cairo, coords[2], coords[3]);
        if (reverseArrow)
          {
            angle=atan2(coords[3]-coords[1], coords[2]-coords[0])+M_PI;
            lastx=coords[0]; lasty=coords[1];
          }
        else
          {
            angle=atan2(coords[3]-coords[1], coords[2]-coords[0]) + (reverseArrow? M_PI:0);
            lastx=coords[2]; lasty=coords[3];
          }
      }
    else
      {
        cairo_move_to(cairo, coords[0], coords[1]);
        
        /** 
         *  
         * Two control points are inserted between two adjacent handles (knots) of the curved wires on the canvas.
         * The first and second derivatives of the cubic Bezier curves, representing curved segments of wires, and spanning adjacent knots,
         * are matched at the common knots. The second derivatives are set to zero at the ends to absorb the extra degrees of freedom,
         * thus leading to a matrix equation relating control points \f$c_i\f$ to the knots \f$k_i\f$:
         *
         *     
         *     \f[
         *        \left(\begin{array}{c} b_1 & a_1 & 0  \\ d_1 & b_2 & a_2 \\ 0 & d_2 & b_3  \end{array}\right) \left(\begin{array}{c} c_1 \\ c_2 \\ c_3 \end{array}\right) = \left(\begin{array}{c}  k_1 \\ k_2 \\ k_3 \end{array}\right)
         *     \f] 
         * 
         */   
        
        // For ticket 991/1092. Convert to coordinate pairs.
        vector<pair<float,float>> points = toCoordPair(coords);
       
        int n = points.size()-1;         
        
        // Initial vector of knots in the matrix equation Ac = k
        vector<pair<float,float>> target = constructTargetVector(n, points);
      
        vector<vector<float>> A(n,vector<float>(n));
        // Initial matrix A which relates control points c_i to knots k_i by matching first and second derivatives of cubic Bezier curves at common points (knots) between curves.
        A = constructTriDiag(n);
                                             
        // For ticket 991. Apply Thomas' algorithm to matrix equation Ac=k
        vector<pair<float,float>> controlPoints = computeControlPoints(A, points, target);

        // Decrease tolerance a bit, since it's going to be magnified
        cairo_set_tolerance (cairo, 0.01);
        
        for (int i = 0; i < n; i++) {      
          cairo_curve_to(cairo, controlPoints[i].first,controlPoints[i].second,controlPoints[n+i].first,controlPoints[n+i].second,points[i+1].first,points[i+1].second);
        }		    
        
        // Stash the internal cairo coordinates used to draw curved wires. for ticket 1079.
        if (coords.size()>4) storeCairoCoords(cairo);    
                                         
        cairo_stroke(cairo);     
        angle=atan2(coords[coords.size()-1]-coords[coords.size()-3], 
                    coords[coords.size()-2]-coords[coords.size()-4]);                         
        lastx=coords[coords.size()-2]; lasty=coords[coords.size()-1];
      } 
    cairo_stroke(cairo);

    // draw arrow
    {
      CairoSave cs(cairo);
      auto lw=cairo_get_line_width(cairo);
      cairo_translate(cairo, lastx, lasty);
      cairo_rotate(cairo,angle);
      cairo_move_to(cairo,0,0);
      cairo_line_to(cairo,-5*lw,-3*lw); 
      cairo_line_to(cairo,-3*lw,0); 
      cairo_line_to(cairo,-5*lw,3*lw);
      cairo_close_path(cairo);
      cairo_fill(cairo);
    }

    // draw handles
    if (mouseFocus)
      {
          cairo::CairoSave cs(cairo);
          cairo_set_source_rgb(cairo,0,0,1);
          if (cairoCoords.empty() || coords.size()==4)
            {
              double midx=0.5*(coords[0]+coords[2]);
              double midy=0.5*(coords[1]+coords[3]);
              cairo_arc(cairo,midx,midy,1.5*handleRadius, 0, 2*M_PI);
              cairo_fill(cairo);
            } 
          else 
            {
              size_t numSmallHandles=0.5*(coords.size()-4)+1, numCairoCoords=cairoCoords.size()-1;
              for (size_t i=0; i<coords.size()-3; i+=2)
                {
                  double midx=cairoCoords[(i+1)*numCairoCoords/(2*numSmallHandles)].first;
                  double midy=cairoCoords[(i+1)*numCairoCoords/(2*numSmallHandles)].second;
                  cairo_arc(cairo,midx,midy,handleRadius, 0, 2*M_PI);
                  if (i>0) // draw existing interior gripping handle            
                    cairo_arc(cairo,coords[i],coords[i+1],1.5*handleRadius, 0, 2*M_PI); 
                  cairo_fill(cairo);
                } 
            }
      }
    if (mouseFocus && !tooltip.empty())
      {
        cairo::CairoSave cs(cairo);
        string toolTipText=latexToPango(tooltip);
        ecolab::Pango pango(cairo);
        pango.setMarkup(toolTipText);
        // place tooltip on centre dot if an odd number of control points, or halfway between otherwise
        auto dd=div(coords.size(),4);
        assert(dd.rem%2==0);
        if (dd.quot%2==1) dd.quot+=dd.rem-1; // adjust so the dd.quot points to an x coordinate
        double midx=coords[dd.quot], midy=coords[dd.quot+1];
        if (dd.rem==0)
          {
            midx=0.5*(coords[dd.quot]+coords[dd.quot+2]);
            midy=0.5*(coords[dd.quot+1]+coords[dd.quot+3]);
          }
        cairo_translate(cairo,midx,midy);
        cairo_rectangle(cairo,0,0,pango.width(),pango.height());
        cairo_set_source_rgb(cairo,1,1,1);
        cairo_fill_preserve(cairo);
        cairo_set_source_rgb(cairo,0,0,0);
        pango.show();
        cairo_stroke(cairo);
      }
  }

  void Wire::split()
  {
    // add I/O variables if this wire crosses a group boundary
    if (auto fg=from()->item().group.lock())
      if (auto tg=to()->item().group.lock())
        if (fg!=tg && !from()->item().ioVar() && !to()->item().ioVar()) // crosses boundary
          {
            // check if this wire is in from group
            auto cmp=[&](const WirePtr& w) {return w.get()==this;};
            auto i=find_if(fg->wires.begin(), fg->wires.end(), cmp);
            if (i==fg->wires.end())
              {
                fg->addOutputVar();
                assert(fg->outVariables.back()->portsSize()>1);
                fg->addWire(new Wire(from(),fg->outVariables.back()->ports(1)));
                moveToPorts(fg->outVariables.back()->ports(0).lock(), to());
              }
            // check if this wire is in to group
            i=find_if(tg->wires.begin(), tg->wires.end(), cmp);
            if (i==tg->wires.end())
              {
                tg->addInputVar();
                assert(tg->inVariables.back()->portsSize()>1);
                tg->addWire(new Wire(tg->inVariables.back()->ports(0),to()));
                moveToPorts(from(), tg->inVariables.back()->ports(1).lock());
              }
          }
  }

  Units Wire::units(bool check) const
  {
      
    if (auto f=from())
      {
        // we allow possible traversing twice over a wire, to allow an
        // integral to break the cycle
        if (unitsCtr>2)
          f->item().throw_error("wiring loop detected");
        IncrDecrCounter idc(unitsCtr);
        return f->item().units(check);
      }
    return {};
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
    
    inline float d2(float x0, float y0, float x1, float y1)
    {return sqr(x1-x0)+sqr(y1-y0);}
    	
  }    

#undef near
  bool Wire::near(float x, float y) const
  {
    auto c=coords();
    assert(c.size()>=4);
    if (c.size()==4)
      return segNear(c[0],c[1],c[2],c[3],x,y);

    // fixes for tickets 991/1095
    vector<pair<float,float>> p=allHandleCoords(c);
    if (!cairoCoords.empty()) p=cairoCoords;   
         
    unsigned k=0; // nearest index
    float closestD=d2(p[0].first,p[0].second,x,y);      
    for (size_t i=0; i<p.size(); i++)
      {
        float d=d2(p[i].first,p[i].second,x,y);
        if (d<=closestD)
          {
            closestD=d;
            k=i;
          }
      }
      
    // Check for proximity to line segments about index k
    if (k>0 && k<p.size()-1)  
      return (segNear(p[k-1].first,p[k-1].second,p[k].first,p[k].second,x,y) || segNear(p[k].first,p[k].second,p[k+1].first,p[k+1].second,x,y));      
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
        if (d<=closestD)
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

  // For ticket 1092. Reinstate delete handle user interaction
  void Wire::deleteHandle(float x, float y)
  {	    
    auto c=coords();
    auto n=c.begin(); // nearest index
    float closestD=d2(c[0],c[1],x,y);
    for (auto i=c.begin()+2; i<c.end()-1; i+=2)
      {
        float d=d2(*i,*(i+1),x,y);
        if (d<closestD)
          {
            closestD=d;
            n=i;
          }
      }	  
    assert(n<c.end()-1);    
    c.erase(n, n+2);
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
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::Wire);
