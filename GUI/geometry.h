/*
  @copyright Steve Keen 2013
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

/// some useful geometry types, defined from boost::geometry

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/assign.hpp>

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

namespace minsky
{
  typedef boost::geometry::model::d2::point_xy<float> Point;
  typedef boost::geometry::model::ring<Point> Polygon;
  typedef boost::geometry::model::box<Point> Rectangle;

#ifndef M_PI
  static const float M_PI = 3.1415926535f;
#endif

  /// convenience method for using += and , to assign multiple points
  /// to a Polygon
  inline boost::assign::list_inserter
  < boost::assign_detail::call_push_back< Polygon >, Point >
  operator+=( Polygon& c, Point v )
  {
    return boost::assign::make_list_inserter
      (boost::assign_detail::call_push_back<Polygon>(c))(v);
  }

  /// rotate (x,y) by \a rot (in degrees) around the origin \a (x0, y0)
  /// can be used for rotating multiple points once constructed
  class Rotate
  {
    float angle; // in radians
    float ca, sa;
    float x0, y0;
  public:
    Rotate(float rot, float x0, float y0):
      angle(rot*M_PI/180.0), ca(cos(angle)), sa(sin(angle)), x0(x0), y0(y0) {}
    /// rotate (x1,y1)
    Point operator()(float x1, float y1) const {
      return Point(x(x1,y1),y(x1,y1));}
    Point operator()(const Point& p) const {return operator()(p.x(), p.y());}
    float x(float x, float y) const {return ca*(x-x0)-sa*(y-y0)+x0;}
    float y(float x, float y) const {return sa*(x-x0)+ca*(y-y0)+y0;}
  };

  // class that reflects about the vertical axis, rotated by angle 
  class Reflect
  {
    float xx, xy, yx, yy; // matrix components
    float x0, y0;
  public:
    Reflect(float angle=0, float x0=0, float y0=1): x0(x0), y0(y0) {
      Rotate r(angle,0,0);
      Point c1=r(-1,0), c2=r(0,1); // columns of UR
      Point r1=r(c1.x(), c2.x()), r2=r(c1.y(),c2.y()); // rows of (URU^-1)^T
      xx=r1.x();
      xy=r1.y();
      yx=r2.x();
      yy=r2.y();
    }
    float x(float x, float y) const {
      return xx*(x-x0)+yx*(y-y0)+x0;
    }
    float y(float x, float y) const {
      return xy*(x-x0)+yy*(y-y0)+y0;
    }
  };
}

#endif
