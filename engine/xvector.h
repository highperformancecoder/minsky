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
#include <boost/date_time.hpp>
#include <vector>
#include <initializer_list>

namespace minsky
{
  /// \a format - can be any format string suitable for a
  /// boost::date_time time_facet. eg "%Y-%m-%d %H:%M:%S"
  std::string str(const boost::any&, const std::string& format="");
  /// convert string rep to an any rep
  boost::any anyVal(const Dimension&, const std::string&);

  /// return absolute difference between any elements
  /// for strings, returns hamming distance
  /// for time, returns seconds
  /// @throw if any is an incompatible type with dimension
  double diff(const boost::any& x, const boost::any& y);

  /// default parsing of a time string
  boost::posix_time::ptime sToPtime(const std::string& s);

  /// labels describing the points along dimensions. These can be strings (text type), time values (boost::posix_time type) or numerical values (double)
  struct XVector: public std::vector<boost::any>
  {
    typedef std::vector<boost::any> V;
    std::string name;
    Dimension dimension;
    XVector() {}
    XVector(const std::string& name, const V& v=V()): V(v), name(name) {}
    XVector(const std::string& name, const std::initializer_list<const char*>& v): name(name)
    {for (auto i: v) push_back(i);}
    bool operator==(const XVector& x) const;
    void push_back(const std::string&);
    void push_back(const char* x) {push_back(std::string(x));}
    using V::push_back;

  };

}

// nobble these, as they're not needed, and boost::any has rather nontrivial serialisers
#ifdef _CLASSDESC
#pragma omit pack minsky::XVector
#pragma omit unpack minsky::XVector
#endif
#include <classdesc.h>
namespace classdesc_access
{
  template<> struct access_pack<minsky::XVector>:
    public classdesc::NullDescriptor<classdesc::pack_t> {};
  template<> struct access_unpack<minsky::XVector>:
    public classdesc::NullDescriptor<classdesc::unpack_t> {};
}
#include "xvector.cd"
#endif
