/*
  @copyright Russell Standish 2019
  @author Russell Standish
  This file is part of Civita.

  Civita is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Civita is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Civita.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CIVITA_DIMENSION_H
#define CIVITA_DIMENSION_H
#include <string>
#include <map>
#include <stdexcept>
namespace civita
{
  struct Dimension
  {
    enum Type {string, time, value};
    Type type=string;
    std::string units; // for values, or parser string for time conversion
    Dimension() {}
    Dimension(Type t,const std::string& s): type(t), units(s) {}
  };

  typedef std::map<std::string, Dimension> Dimensions;

  typedef std::map<std::string, double> ConversionsMap;
  struct Conversions: public ConversionsMap
  {
    double convert(double val, const std::string& from, const std::string& to)
    {
      if (from==to) return val;
      auto i=find(from+":"+to);
      if (i!=end())
        return i->second*val;
      i=find(to+":"+from);
      if (i!=end())
        return val/i->second;
      throw std::runtime_error("inconvertible types "+from+" and "+to);
    }
    Conversions& operator=(const ConversionsMap& x)
    {ConversionsMap::operator=(x); return *this;}
  };
}

#endif
