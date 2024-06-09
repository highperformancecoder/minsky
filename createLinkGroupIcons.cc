#include "plot.h"
#include "cairo_base.h"
#include "ecolab_epilogue.h"
#include <cairo.h>
using namespace ecolab;
using namespace std;

int main()
{
  const double width=30, height=10;
  for (int i=0; i<paletteSz; ++i)
    {
      const cairo::Surface surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height));
      auto cr=surface.cairo();
      auto c=palette[i];
      cairo_set_source_rgba(cr,0.5*c.r,0.5*c.g,0.5*c.b,0.5*c.a);
      cairo_rectangle(cr,0,0,width,height);
      cairo_fill(cr);
      cairo_surface_write_to_png(surface.surface(),("linkGroup"+to_string(i)+".png").c_str());
    }
}
