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

#include "tempCairoImageFile.h"
#include "minskyApp.h"
#include <ecolab_epilogue.h>
#include <boost/filesystem.hpp>

namespace minsky { namespace gui {

using namespace boost::filesystem;

TempCairoImageFile::TempCairoImageFile(cairo_surface_t* surface)
  : id(generateUniqueName()),
    image(NULL)
{
  if (CAIRO_STATUS_SUCCESS == cairo_surface_write_to_png(surface, fileName().c_str()))
  {
    image = new Wt::WPainter::Image(url(), fileName());
  }
  else
  {
    BOOST_THROW_EXCEPTION(pngWriteException());
  }
}

TempCairoImageFile::~TempCairoImageFile()
{
  deleteFile();
}

std::string TempCairoImageFile::fileName() const
{
  std::string name = MinskyApp::getApp()->relativeResourcesUrl();

  if (*(name.rbegin()) != '/' && *(name.rbegin()) != '\\')
  {
    name += '/';
  }
  name += id;
  return name;
}

std::string TempCairoImageFile::url() const
{
  std::string uri = MinskyApp::getApp()->resourcesUrl();

  if (*(uri.rbegin()) != '/')
  {
    uri += '/';
  }
  uri += id;
  return uri;
}

void TempCairoImageFile::deleteFile()
{
  if (!id.empty())
  {
    remove(fileName());
    id.clear();
  }
  if (image)
  {
    delete image;
  }
}

std::string TempCairoImageFile::generateUniqueName()
{
  return boost::filesystem::unique_path().string();
}

}}  // namespace minsky::gui
