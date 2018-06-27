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

#ifndef XVECTOR_H
#define XVECTOR_H
#include "dimension.h"
#include <boost/any.hpp>
#include <vector>

namespace minsky
{
  std::string str(const boost::any&);

  /// labels describing the points along dimensions. These can be strings (text type), time values (boost::posix_time type) or numerical values (double)
  struct XVector: public std::vector<boost::any>
  {
    typedef std::vector<boost::any> V;
    std::string name;
    Dimension dimension;
    XVector() {}
    XVector(const std::string& name, const V& v=V()): V(v), name(name) {}
    //bool operator==(const XVector& x) const {return name==x.name && V(*this)==(x);}
    void push_back(const std::string&);
    /// convert a value (from this xvector into a string representation
  };

}

#include "xvector.cd"
#endif
