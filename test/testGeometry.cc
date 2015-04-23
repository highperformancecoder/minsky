#include "../GUI/geometry.h"
#include "../GUI/cairoItems.h"
#include <ecolab_epilogue.h>
#include <UnitTest++/UnitTest++.h>

#include <boost/geometry/geometry.hpp>

using namespace minsky;
using namespace boost::geometry;
using namespace boost::geometry::model;

SUITE(Geometry)
{
  TEST(RenderVariable)
  {
    VariablePtr var(VariableType::flow,"hello");
    var->moveTo(100,200);
    RenderVariable rv(*var);
    float rvArea=4*rv.width()*rv.height();
    CHECK_CLOSE(rvArea, area(rv.geom()), 1e-5);
    box<Point> bbox=return_envelope<box<Point> >(rv.geom());
    CHECK_CLOSE(var->x()-rv.width(), bbox.min_corner().x(), 1e-5);
    CHECK_CLOSE(var->x()+rv.width(), bbox.max_corner().x(), 1e-5);
    CHECK_CLOSE(var->y()-rv.height(), bbox.min_corner().y(), 1e-5);
    CHECK_CLOSE(var->y()+rv.height(), bbox.max_corner().y(), 1e-5);

    var->zoom(var->x(),var->y(),2);
    CHECK_CLOSE(4*rvArea, area(rv.geom()), 1e-5);
  
    var->rotation=35;
    // area should be unaffected by rotation
    CHECK_CLOSE(4*rvArea, area(rv.geom()), 1e-2);
    bbox=return_envelope<box<Point> >(rv.geom());
    Rotate rotate(var->rotation,var->x(), var->y());
    CHECK_CLOSE(rotate(var->x()-rv.width()*2, var->y()+rv.height()*2).x(),
                bbox.min_corner().x(), 1e-4);
    CHECK_CLOSE(rotate(var->x()+rv.width()*2, var->y()-rv.height()*2).x(), 
                bbox.max_corner().x(), 1e-4);
    CHECK_CLOSE(rotate(var->x()-rv.width()*2, var->y()-rv.height()*2).y(),
                bbox.min_corner().y(), 1e-4);
    CHECK_CLOSE(rotate(var->x()+rv.width()*2, var->y()+rv.height()*2).y(),
                bbox.max_corner().y(), 1e-4);
  }

  TEST(RenderOperation)
  {
    OperationPtr op(OperationType::time);
    op->moveTo(100,200);
    RenderOperation ro(*op);
    float roArea=2*ro.width()*ro.height();
    CHECK_CLOSE(roArea, area(ro.geom()), 1e-5);
    box<Point> bbox=return_envelope<box<Point> >(ro.geom());
    CHECK_CLOSE(op->x()+op->l, bbox.min_corner().x(), 1e-5);
    CHECK_CLOSE(op->x()+op->r, bbox.max_corner().x(), 1e-5);
    CHECK_CLOSE(op->y()-op->h, bbox.min_corner().y(), 1e-5);
    CHECK_CLOSE(op->y()+op->h, bbox.max_corner().y(), 1e-5);

    op->zoom(op->x(),op->y(),2);
    CHECK_CLOSE(4*roArea, area(ro.geom()), 1e-5);
  
    op->rotation=15;
    // area should be unaffected by rotation
    CHECK_CLOSE(4*roArea, area(ro.geom()), 1e-2);
    bbox=return_envelope<box<Point> >(ro.geom());
    Rotate rotate(op->rotation,op->x(), op->y());
    CHECK_CLOSE(rotate(op->x()+op->l*2, op->y()+op->h*2).x(),
                bbox.min_corner().x(), 1e-5);
    CHECK_CLOSE(rotate(op->x()+op->r*2, op->y()).x(), 
                bbox.max_corner().x(), 1e-5);
    CHECK_CLOSE(rotate(op->x()+op->l*2, op->y()-op->h*2).y(),
                bbox.min_corner().y(), 1e-5);
    CHECK_CLOSE(rotate(op->x()+op->l*2, op->y()+op->h*2).y(),
                bbox.max_corner().y(), 1e-5);
  }


}
