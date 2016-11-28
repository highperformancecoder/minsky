/*
  @copyright Steve Keen 2012
  @author Michael Roy
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

#ifndef WTGEOMETRY_H_
#define WTGEOMETRY_H_

#include <Wt/WPointF>
#include <Wt/WRectF>
#include <vector>

namespace minsky { namespace gui {

/**
 *  A list of points.
 */
typedef std::vector<Wt::WPointF>  PointsList;

/**
 * A simple polygon representation
 */
typedef PointsList  Polygon;

/**
 * An array of polygons.
 */
typedef std::vector<Polygon> PolygonsList;

/**
 * 2-d Rotation of points, polygons, etc..
 *
 * @param poly  [in/out] Polygon to rotate.
 * @param angle Angle of rotation in degrees.
 * @param center Center of the rotation.
 */
void rotate(Polygon& polygon, double angle, const Wt::WPointF& center);

/**
 * 2-d Rotation of points, polygons, etc.. around (0, 0)
 *
 * @param poly  [in/out] Polygon to rotate.
 * @param angle Angle of rotation in degrees.
 */
void rotate(Polygon& polygon, double angle);

/**
 * 2-d Rotation of a single point
 *
 * @param poly  Polygon to rotate.
 * @param angle Angle of rotation in degrees.
 * @param center Center of the rotation.
 */
void rotate(Wt::WPointF& point, double angle, const Wt::WPointF& center);

/**
 * 2-d Rotation of a single point around the origin.
 *
 * @param poly  Polygon to rotate.
 * @param angle Angle of rotation in degrees.
 * @param center Center of the rotation.
 */
void rotate(Wt::WPointF& point, double angle);

/**
 * 2-d Rotation of a rectangle
 *
 * @param poly  resulting rectangle, as a polygon.
 * @param rect  rectangle to rotate.
 * @param angle Angle of rotation in degrees.
 * @param center Center of the rotation.
 */
void rotate(Polygon& polygon, const Wt::WRectF& rect, double angle, const Wt::WPointF& center);

/**
 * 2-d Rotation of a rectangle around (0, 0)
 *
 * @param poly  resulting rectangle, as a polygon.
 * @param rect  rectangle to rotate.
 * @param angle Angle of rotation in degrees.
 */
void rotate(Polygon& polygon, const Wt::WRectF& rect, double angle);

/**
 * Polygon translation
 */
void translate(Polygon& polygon, const Wt::WPointF& offset);

/**
 * Returns polygon extents
 */
void extents(double& left, double& top, double& right, double& bottom, const Polygon& polygon);

/**
 * Returns polygon extents as a rectangle
 */
void extents(Wt::WRectF& rect, const Polygon& polygon);

/**
 * Polygon scaling
 */
void scale(Polygon& polygon, double factor, const Wt::WPointF& center);

/**
 * Polygon scaling around (0, 0)
 */
void scale(Polygon& polygon, double factor);

/**
 * single point scaling with offset
 */
void scale(Wt::WPointF& point, double factor, const Wt::WPointF& center);

/**
 *  2-d vector addition.
 */
inline Wt::WPointF operator+ (const Wt::WPointF& a, const Wt::WPointF& b)
{
  return Wt::WPointF(a.x() + b.x(), a.y() + b.y());
}

/**
 *  2-d vector subtraction.
 */
inline Wt::WPointF operator- (const Wt::WPointF& a, const Wt::WPointF& b)
{
  return Wt::WPointF(a.x() - b.x(), a.y() - b.y());
}

/**
 *  2-d vector scaling.
 */
/** @{ */
inline Wt::WPointF operator* (const Wt::WPointF& a, const double& s)
{
  return Wt::WPointF(s * a.x(), s * a.y());
}

inline Wt::WPointF operator* (const double& s, const Wt::WPointF& a)
{
  return Wt::WPointF(s * a.x(), s * a.y());
}

inline Wt::WPointF operator/ (const Wt::WPointF& a, const double& s)
{
  return Wt::WPointF(a.x() / s , a.y() / s);
}
/** @} */

/**
 * Computes the mid point between p0 and p1.
 *
 * @param p0   first reference point.
 * @param p1   second reference point.
 * @returns The point at (p0 + p1) / 2.
 */
inline Wt::WPointF midPoint(const Wt::WPointF& p0, const Wt::WPointF& p1)
{
  return ((p0 + p1) / 2);
}

//
//  Bézier curve support
//

/**
 * Computes the quadratic bezier curve passing through p0 and p2, with summit
 * at p1.
 *
 * @param curve on return, contains the points defining the curve.
 * @param nPoints Number of points desired in the curve.
 * @param p0  starting point for the curve.
 * @param p1  central control point for the curve.
 * @param p2  ending point for the curve.
 */
void quadBezierCurve (PointsList& curve, int nPoints, const Wt::WPointF& p0,
                      const Wt::WPointF& p1, const Wt::WPointF& p2);

/**
 * Create a curved l.ine from a list of control points.  The points given are
 * taken as summints for the curve, with intermediary endpoints midway between
 * them.
 *
 * @param curve on return, contains the points defining the curve.
 * @param nPoints Number of points desired in the curve, note that
 *                this is the number of line segments between control
 *                points and intermediary pass-through points.
 * @param controlPoints List of control points, aka summits.  The
 *                first and last points in the array are endpoints
 *                for the curve.
 */
void quadBezierLine (PointsList& curve, int nPoints, const PointsList& controlPoints);


/**
 *  Creates a polygon around a line.
 *
 *  @param poly   On returns, contains the resulting polygon.
 *  @param width  thickness of the polygon in pixels.
 *  @param line   Line to encapsulate.
 */
void createPolygonArounPolyline(Polygon& poly, const double& width, const PointsList line);

/**
 * Calculates the distance between two points
 * @return The square of the distance between p0 and p1.
 */
double distanceSquared(const Wt::WPointF& p0, const Wt::WPointF& p1);

/**
 * Calculates the distance between two points
 * @return The distance between p0 and p1.
 */
double distance(const Wt::WPointF& p0, const Wt::WPointF& p1);

/**
 * Converts a rectange to a polygon.
 * @param poly  On return contains the polygon.
 * @param rect Rectangle to convert.
 */
void rectToPolygon(Polygon& poly, const Wt::WRectF& rect);


}} // namespace minsky::gui

#endif /* WTGEOMETRY_H_ */
