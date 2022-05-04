#include "cairoItems.h"
#include "lasso.h"
#include "variableNameRender.h"
#include "variableValue.h"
#include "pango.h"
#include "minsky.h"
#include "minsky_epilogue.h"

#include <iostream>

namespace minsky
{
  VariableNameRender::VariableNameRender(const VariableValue& var): var(var.type(), var.name)
  {
    ecolab::cairo::Surface surf
      (cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA,NULL));
    cairo_move_to(surf.cairo(),0,0);
    RenderVariable rv(*this->var, surf.cairo());
    rv.draw();
    double l,t;
    cairo_recording_surface_ink_extents(surf.surface(),&l,&t,&m_width,&m_height);
  }

  
  bool VariableNameRender::redraw(int x0, int y0, int width, int height)
  {
    if (!surface.get() || !var) return false;
    cairo_move_to(surface->cairo(),0,0);
    RenderVariable rv(*var, surface->cairo());
    cairo_translate(surface->cairo(),0.5*m_width,0.5*m_height);
    rv.draw();
    return true;
  }

  void VariableNameRender::emplace() const
  {
    if (var)
      minsky().canvas.addVariable(var->rawName(), var->type());
  }

}
