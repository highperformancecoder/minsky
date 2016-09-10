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

#ifndef TEMPCAIROIMAGEFILE_H_
#define TEMPCAIROIMAGEFILE_H_

#include "guiDefs.h"
#include <Wt/WPainter>
#include <string>
#include <cairo.h>

namespace minsky { namespace gui {

/**
 * Creates a temporary image from a cairo surface for display on
 * the client browser. Image conversion, storage and removal
 * are handled automatically.
 */
class TempCairoImageFile
{
public:
  struct pngWriteException
    : virtual boost::exception,
      virtual std::exception { };

private:
  std::string id;   ///< Unique ID.
  Wt::WPainter::Image* image;   ///< Wt image.

public:
  /**
   * Constructor.
   */
  TempCairoImageFile(cairo_surface_t* surface);

  /**
   * Destructor.
   */
  virtual ~TempCairoImageFile();

  /**
   * Returns the local file system filename.
   */
  std::string fileName() const;

  /**
   * Returns the Web server URL
   */
  std::string url() const;

  /**
   * Deletes the stored data.
   */
  void deleteFile();

  /**
   *  Accessor.
   */
  inline Wt::WPainter::Image* getImage()
  {
    return image;
  }

protected:
  /**
   * Generates a unique ID
   */
  static std::string generateUniqueName();
};

}}  // namespace minsky::gui

#endif /* TEMPCAIROIMAGEFILE_H_ */
