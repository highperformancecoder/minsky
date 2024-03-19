/*
  @copyright Steve Keen 2018
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

#ifndef BOOKMARK_H
#define BOOKMARK_H
#include <string>

namespace minsky
{
  struct Bookmark
  {
    float x=0, y=0, zoom=1;
    std::string name;
    Bookmark() {}
    Bookmark(const std::string& n): name(n) {}
    Bookmark(float x, float y, float z,const std::string& n):
      x(x), y(y), zoom(z), name(n) {}
    bool operator<(const Bookmark& x) const
    {return name<x.name;}
  };
}

#include "bookmark.cd"
#include "bookmark.xcd"
#include "bookmark.rcd"
#endif
