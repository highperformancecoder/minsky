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

#include "wtGeometry.h"
#include <algorithm>
#include <math.h>

namespace minsky { namespace gui {

// needed for compiling under mingw/msys
#ifndef M_PI
# define M_PI (3.14159265358979323846)
#endif


void rotate(Polygon& polygon, double angle, const Wt::WPointF& center)
{
  std::for_each(polygon.begin(), polygon.end(),
      [angle, &center](Wt::WPointF& point)
      {
        rotate(point, angle, center);
      });
}

void rotate(Polygon& polygon, double angle)
{
  std::for_each(polygon.begin(), polygon.end(),
      [angle](Wt::WPointF& point)
      {
        rotate(point, angle);
      });
}

void rotate(Wt::WPointF& point, double angle, const Wt::WPointF& center)
{
  angle *= (M_PI / 180.0);

  double cosA = cos(angle);
  double sinA = sin(angle);

  Wt::WPointF p(point - center);

  point.setX((cosA * p.x()) - (sinA * p.y()) + center.x());
  point.setY((cosA * p.y()) + (sinA * p.x()) + center.y());
}

void rotate(Wt::WPointF& point, double angle)
{
  angle *= (M_PI / 180.0);

  double cosA = cos(angle);
  double sinA = sin(angle);

  Wt::WPointF p(point);

  point.setX((cosA * p.x()) - (sinA * p.y()));
  point.setY((cosA * p.y()) + (sinA * p.x()));
}

void rotate(Polygon& polygon, const Wt::WRectF& rect, double angle, const Wt::WPointF& center)
{
  polygon.clear();
  polygon.reserve(4);
  polygon.push_back(Wt::WPointF(rect.left(), rect.top()));
  polygon.push_back(Wt::WPointF(rect.right(), rect.top()));
  polygon.push_back(Wt::WPointF(rect.right(), rect.bottom()));
  polygon.push_back(Wt::WPointF(rect.left(), rect.bottom()));

  rotate(polygon, angle, center);
}

void rotate(Polygon& polygon, const Wt::WRectF& rect, double angle)
{
  polygon.clear();
  polygon.reserve(4);
  polygon.push_back(Wt::WPointF(rect.left(), rect.top()));
  polygon.push_back(Wt::WPointF(rect.right(), rect.top()));
  polygon.push_back(Wt::WPointF(rect.right(), rect.bottom()));
  polygon.push_back(Wt::WPointF(rect.left(), rect.bottom()));

  rotate(polygon, angle);
}

void translate(Polygon& polygon, const Wt::WPointF& offset)
{
  std::for_each(polygon.begin(), polygon.end(),
      [&offset](Wt::WPointF& point)
      {
        point = point + offset;
      });
}

void extents(double& left, double& top, double& right, double& bottom, const Polygon& polygon)
{
  if (polygon.size())
  {
    left = polygon[0].x();
    top  = polygon[0].y();
    right = left;
    bottom = top;

    std::for_each(polygon.begin() + 1, polygon.end(),
        [&left, &top, &right, &bottom](const Wt::WPointF& point)
        {
          left = std::min(left, point.x());
          top = std::min(top, point.y());
          right = std::max(right, point.x());
          bottom = std::max(bottom, point.y());
        });
  }
  else
  {
    left = top = right = bottom = 0;
  }
}

void extents(Wt::WRectF& rect, const Polygon& polygon)
{
  double left, top, right, bottom;

  extents(left, top, right, bottom, polygon);

  rect = Wt::WRectF(left, top, right - left, bottom - top);
}

void scale(Polygon& polygon, double factor, const Wt::WPointF& center)
{
  std::for_each(polygon.begin(), polygon.end(),
      [factor, &center](Wt::WPointF& point)
      {
        scale(point, factor, center);
      });
}

void scale(Polygon& polygon, double factor)
{
  std::for_each(polygon.begin(), polygon.end(),
      [factor](Wt::WPointF& point)
      {
        point = factor * point;
      });
}

void scale(Wt::WPointF& point, double factor, const Wt::WPointF& center)
{
  point = (factor * (point - center)) + center;
}

namespace {

  /**
   * same as quadBezierCurve(), but first and last points are omitted,
   * and the output vector is not reset before adding points.
   *
   * @note private.
   */
  size_t _quadBezierCurve (PointsList& curve, int nPoints, const Wt::WPointF& p0,
                         const Wt::WPointF& p1, const Wt::WPointF& p2)
  {
    double stepSize = 1. / nPoints;
    double t = stepSize;
    size_t numPoints = 0;

    Wt::WPointF previousPoint = p0;

    for (int i = 1; i < nPoints - 1; ++i, t += stepSize)
    {
      Wt::WPointF pnt;

      double a = 1 - t;
      double b = a * a;

      pnt = (b * p0) + ((2 * a * t) * p1) + ((t * t) * p2);

      // avoid adding unnecessary points
      if (distanceSquared(pnt, previousPoint) > 1)
      {
        curve.push_back(pnt);
        previousPoint = pnt;
        ++numPoints;
      }
    }
    return numPoints;
  }
}

void quadBezierCurve (PointsList& curve, int nPoints, const Wt::WPointF& p0,
                      const Wt::WPointF& p1, const Wt::WPointF& p2)
{
  curve.clear();
  curve.reserve(nPoints);

  curve.push_back(p0);
  _quadBezierCurve(curve, nPoints, p0, p1, p2);
  curve.push_back(p2);
}

void quadBezierLine (PointsList& curve, int nPoints, const PointsList& controlPoints)
{
  curve.clear();

  if (controlPoints.size() == 2)
  {
    curve.push_back(controlPoints[0]);
    curve.push_back(controlPoints[1]);
  }
  else if (controlPoints.size() > 2)
  {
    curve.reserve(2 + (nPoints * (controlPoints.size() - 2)));

    Wt::WPointF p0 = controlPoints[0];
    curve.push_back(p0);

    for (size_t i = 1; i < controlPoints.size() - 1; ++i)
    {
      Wt::WPointF p1, p2;

      p1 = controlPoints[i];

      if (i == controlPoints.size() - 2)
      {
        p2 = controlPoints[controlPoints.size() - 1];
      }
      else
      {
        p2 = midPoint(p1, controlPoints[i + 1]);
      }

      size_t numPoints = _quadBezierCurve(curve, nPoints, p0, p1, p2);

      if (numPoints
          || (distanceSquared(p0, p2) > 1)
          || ((i == controlPoints.size() - 1) && curve.size() == 1))
      {
        curve.push_back(p2);
      }
      p0 = p2;
    }
  }
}

void createPolygonArounPolyline(Polygon& poly, const double& width, const PointsList line)
{
  if (line.size() < 2)
  {
    poly.clear();
    return;
  }
  if (line.size() == 2 && distanceSquared(line[0], line[1]) < 1)
  {
    poly.clear();
    double d = width * cos(M_PI / 4);
    poly.push_back(Wt::WPointF(line[0].x() - d, line[0].y() - d));
    poly.push_back(Wt::WPointF(line[0].x() + d, line[0].y() - d));
    poly.push_back(Wt::WPointF(line[0].x() + d, line[0].y() + d));
    poly.push_back(Wt::WPointF(line[0].x() - d, line[0].y() + d));
    return;
  }


  poly.resize(2 * line.size());

  size_t i = 0;               //  forward counting index
  size_t j = poly.size() - 1; //  back counting.

  double az1, az2;    //  azimuth of line segments
  double a;           //  azimuths of extension
  Wt::WPointF v;      //  extension vector.
  double dEnd = width / sin(M_PI / 4);  //  length of extension at ends.

  az2 = atan2(line[i + 1].y() - line[i].y(), line[i + 1].x() - line[i].x());
  if (!finite(az2) || isnan(az2))
  {
    az2 = 0;
  }

  a = az2 + (3 * M_PI / 4);
  v = dEnd * Wt::WPointF(cos(a), sin(a));
  poly[i] = line[i] + v;

  a += (M_PI / 2);
  v = dEnd * Wt::WPointF(cos(a), sin(a));
  poly[j] = line[i] + v;

  ++i;
  --j;

  //...

  while (i < (line.size() - 1))
  {
    az1 = az2;
    az2 = atan2(line[i + 1].y() - line[i].y(), line[i + 1].x() - line[i].x());
    if (!finite(az2) || isnan(az2))
    {
      az2 = 0;
    }
    a = (M_PI + (az1 + az2)) / 2;

    v = width * Wt::WPointF(cos(a), sin(a));
    poly[i] = line[i] + v;
    poly[j] = line[i] - v;

    ++i;
    --j;
  }

  // last endpoint.

  a = az2 + (M_PI / 4);
  v = dEnd * Wt::WPointF(cos(a), sin(a));
  poly[i] = line[i] + v;

  a -= (M_PI / 2);
  v = dEnd * Wt::WPointF(cos(a), sin(a));
  poly[j] = line[i] + v;

  //  that's it!
}

double distanceSquared(const Wt::WPointF& p0, const Wt::WPointF& p1)
{
  Wt::WPointF delta = p1 - p0;
  return ((delta.x() * delta.x()) + (delta.y() * delta.y()));
}

double distance(const Wt::WPointF& p0, const Wt::WPointF& p1)
{
  return sqrt(distanceSquared(p0, p1));
}

void rectToPolygon(Polygon& poly, const Wt::WRectF& rect)
{
  poly.clear();
  poly.push_back(Wt::WPointF(rect.left(), rect.top()));
  poly.push_back(Wt::WPointF(rect.right(), rect.top()));
  poly.push_back(Wt::WPointF(rect.right(), rect.bottom()));
  poly.push_back(Wt::WPointF(rect.left(), rect.bottom()));
}

}} // namespace minsky::gui
