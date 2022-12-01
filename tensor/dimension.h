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
#include <boost/date_time.hpp>
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

  template <class T> Dimension::Type dimensionTypeOf();
  template <> inline Dimension::Type dimensionTypeOf<std::string>() {return Dimension::string;}
  template <> inline Dimension::Type dimensionTypeOf<boost::posix_time::ptime>() {return Dimension::time;} 
  template <> inline Dimension::Type dimensionTypeOf<double>() {return Dimension::value;}

  /// a variant type representing a value of a dimension
  // TODO - when we move to c++17, consider using std::variant
  struct any
  {
    Dimension::Type type=Dimension::string;
    boost::posix_time::ptime time;
    double value;
    std::string string;
    size_t hash() const;
    any()=default;
    any(const boost::posix_time::ptime& x): type(Dimension::time), time(x) {}
    any(const std::string& x): type(Dimension::string), string(x) {}
    any(const double& x): type(Dimension::value), value(x) {}
    template <class T> any& operator=(const T&x) {return *this=any(x);}
  };

  inline size_t any::hash() const {
    switch (type) {
    case Dimension::string: return std::hash<std::string>()(string);
    case Dimension::time: return std::hash<size_t>()((time-boost::posix_time::ptime()).ticks());
    case Dimension::value: return std::hash<double>()(value);
    }
  }

  inline bool operator<(const any& x, const any& y) {
    if (x.type==y.type)
      switch (x.type)   {
      case Dimension::string: return x.string<y.string;
      case Dimension::time: return x.time<y.time;
      case Dimension::value: return x.value<y.value;
      }
    return x.type<y.type;
  }
  
  inline bool operator==(const any& x, const any& y) {
    if (x.type!=y.type) return false;
      switch (x.type)   {
      case Dimension::string: return x.string==y.string;
      case Dimension::time: return x.time==y.time;
        //case Dimension::time: return x.time-y.time==0;
      case Dimension::value: return x.value==y.value;
      }
      assert(false);
      return false; // shouldn't be here.
  }

#ifdef STRINGKEYMAP_H
  using classdesc::StringKeyMap;
#else
  template <class T> using StringKeyMap=std::map<std::string, T>;
#endif

  typedef StringKeyMap<Dimension> Dimensions;

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

#ifdef CLASSDESC
#pragma omit json_pack civita::any
#pragma omit json_unpack civita::any
#pragma omit RESTProcess civita::any
#endif

#endif
