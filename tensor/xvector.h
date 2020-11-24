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

#ifndef CIVITA_XVECTOR_H
#define CIVITA_XVECTOR_H
#include "dimension.h"
#include <boost/any.hpp>
#include <boost/date_time.hpp>
#include <vector>
#include <initializer_list>

namespace civita
{
  /// convert string rep to an any rep
  boost::any anyVal(const Dimension&, const std::string&);

  /// \a format - can be any format string suitable for a
  /// boost::date_time time_facet. eg "%Y-%m-%d %H:%M:%S"
  std::string str(const boost::any&, const std::string& format="");

  /// return absolute difference between any elements
  /// for strings, returns hamming distance
  /// for time, returns seconds
  /// @throw if any is an incompatible type with dimension
  double diff(const boost::any& x, const boost::any& y);
  struct AnyLess
  {
    bool operator()(const boost::any& x, const boost::any& y) const
    {return diff(x,y)<0;}
  };

  /// default parsing of a time string
  boost::posix_time::ptime sToPtime(const std::string& s);

  struct NamedDimension
  {
    std::string name;
    Dimension dimension;
    NamedDimension(const std::string& name={}, const Dimension& dimension={}): name(name), dimension(dimension) {}
  };
  
  /// labels describing the points along dimensions. These can be strings (text type), time values (boost::posix_time type) or numerical values (double)
  struct XVector: public NamedDimension, public std::vector<boost::any>
  {
    typedef std::vector<boost::any> V;
    XVector() {}
    XVector(const std::string& name, const Dimension& dimension={}, const V& v={}): NamedDimension(name), V(v) {}
    XVector(const std::string& name, const Dimension& dimension, const std::initializer_list<const char*>& v): NamedDimension(name, dimension)
    {for (auto i: v) push_back(i);}
    bool operator==(const XVector& x) const;
    void push_back(const std::string&);
    void push_back(const char* x) {push_back(std::string(x));}
    using V::push_back;
    /// best time format given range of data for plot xticks and spreadsheet labels
    std::string timeFormat() const;
    /// rewrites the labels according to dimension
    void imposeDimension();
    /// @return true if all elements of this are of type T
    template <class T>
    bool checkType() const {
      for (auto& i:*this)
        if (!boost::any_cast<T>(&i))
          return false;
      return true;
    }
    /// @return true if all elements have type described by @a dimension
    bool checkThisType() const {
      switch (dimension.type)
        {
        case Dimension::string:
          return checkType<std::string>();
        case Dimension::value:
          return checkType<double>();
        case Dimension::time:
          return checkType<boost::posix_time::ptime>();
        }
      return false;
    }

  };

}

#endif
