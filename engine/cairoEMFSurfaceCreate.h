/*
  @copyright Steve Keen 2019
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

/**
  Create a cairo surface that renders to Microsoft's Enhanced Metafile
  Format. Only functional on Windows.
 */

#ifdef _WIN32
#include <cairo.h>
namespace minsky
{
  /// create a cairo surface for EMF files. width and height parameters ignored
  cairo_surface_t* createEMFSurface(const char* filename, double, double);
}

#endif
