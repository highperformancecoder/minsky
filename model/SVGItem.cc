/*
  @copyright Steve Keen 2012
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
// Implementations of canvas items representing operations and variables.

/*
  @copyright Steve Keen 2014
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
#include "SVGItem.h"
#include "SVGItem.rcd"
#include "cairo_base.h"
#include "minsky_epilogue.h"
#include <librsvg-2.0/librsvg/rsvg.h>

// if not #ifdef protected, you get a deprecated warning, which is
// made fatal by -Werror
#ifndef RSVG_CAIRO_H
#include <librsvg-2.0/librsvg/rsvg-cairo.h>
#endif

#include <stdexcept>
using namespace std;

namespace minsky
{
 
  void SVGRenderer::setResource(const std::string& resource)
  {
    if (svg) g_object_unref(svg);
    GError* err=nullptr;
    svg=rsvg_handle_new_from_file(resource.c_str(),&err);
    if (err)
      {
        std::string msg="SVGRenderer failed to initialise: ";
        msg+=err->message;
        g_error_free(err);
        throw runtime_error(msg);
      }
    rsvg_handle_get_intrinsic_size_in_pixels(svg, &m_width, &m_height);
  }
  

  SVGRenderer::~SVGRenderer()
  {
    if (svg)
      g_object_unref(svg);
  }

  void SVGRenderer::render(cairo_t* cairo) const
  {
    if (svg)
      {
        GError* err=nullptr;
        RsvgRectangle rect{0,0,m_width,m_height};
        rsvg_handle_render_document(svg,cairo,&rect,&err);
      }
  }

}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::SVGRenderer);
