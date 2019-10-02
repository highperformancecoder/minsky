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

#include "xvector.h"
#include <error.h>
#include "minsky_epilogue.h"
using ecolab::error;

using namespace std;

#include <boost/regex.hpp>
#include <boost/date_time.hpp>
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace minsky
{

  namespace
  {
    void extract(const string& fmt, const string& data, int pos1, const char* re1, int& var1,
                 int pos2, const char* re2, int& var2)
    {
      string rePat=fmt.substr(0,pos1)+re1+
                    fmt.substr(pos1+2,pos2-pos1-2)+re2+
        fmt.substr(pos2+2);
      regex pattern(rePat);
      smatch match;
      regex_match(data,match,pattern);
      var1=stoi(match[1]);
      var2=stoi(match[2]);
    }
  }

  bool XVector::operator==(const XVector& x) const
  {
    if (dimension.type!=x.dimension.type || name!=x.name ||
        size()!=x.size())
      return false;
    for (auto i=begin(), j=x.begin(); i!=end(); ++i, ++j)
      switch (dimension.type)
        {
        case Dimension::string:
          try
            {
              if (any_cast<string>(*i)!=any_cast<string>(*j))
                return false;
            }
          catch (const bad_any_cast&)
            {
              if (strcmp(any_cast<const char*>(*i), any_cast<const char*>(*j))!=0)
                return false;
            }
          break;
        case Dimension::value:
          if (any_cast<double>(*i)!=any_cast<double>(*j))
            return false;
          break;
        case Dimension::time:
          if (any_cast<ptime>(*i)!=any_cast<ptime>(*j))
            return false;
          break;
        default:
          throw error("unknown dimension type");
        }
    return true;
  }
  
  void XVector::push_back(const std::string& s)
  {
    V::push_back(anyVal(dimension, s));
  }

  boost::any anyVal(const Dimension& dim, const std::string& s)
  {
    switch (dim.type)
      {
      case Dimension::string:
        return s;
        break;
      case Dimension::value:
        return stod(s);
        break;
      case Dimension::time:
        {
          string::size_type pq;
          if ((pq=dim.units.find("%Q"))!=string::npos)
            {
              // year quarter format expected. Takes the first %Y (or
              // %y) and first %Q for year and quarter
              // respectively. Everything else is passed to regex, which
              // can be used to match complicated patterns.
              string pattern;
              greg_month quarterMonth[]={Jan,Apr,Jul,Oct};
              int year, quarter;
              auto pY=dim.units.find("%Y");
              if (pY>=0)
                if (pq<pY)
                  extract(dim.units,s,pq,"(\\d)",quarter,pY,"(\\d{4})",year);
                else
                  extract(dim.units,s,pY,"(\\d{4})",year,pq,"(\\d)",quarter);
              else
                throw error("year not specified in format string");
              if (quarter<1 || quarter>4)
                throw error("invalid quarter %d",quarter);
              return ptime(date(year, quarterMonth[quarter-1], 1));
            }
          else if (!dim.units.empty())
            {
              unique_ptr<time_input_facet> facet
                (new time_input_facet(dim.units.c_str()));
              istringstream is(s);
              is.imbue(locale(is.getloc(), facet.release()));
              ptime pt;
              is>>pt;
              if (pt.is_special())
                throw error("invalid date/time: %s",s.c_str());
              return pt;
            }
          else
            return sToPtime(s);
          break;
        }
      }
    assert(false);
    return any(); // shut up compiler warning
  }

  ptime sToPtime(const string& s)
  {
    const char* p=s.c_str();
    char *lp;
    unsigned long d[]={1,1,1,0,0,0}; // Y,M,D,h,m,s
    size_t i=0;
    for (; i<6 && *p; p=lp, ++i)
      {
        d[i]=strtoul(p,&lp,10);
        if (lp==p)
          break;
        while (*lp && !isdigit(*lp)) lp++;
      }
    if (i==0)
      throw error("invalid date/time: %s",s.c_str());
    return ptime(date(d[0],d[1],d[2]), time_duration(d[3],d[4],d[5]));
  }
  
  double diff(const boost::any& x, const boost::any& y)
  {
    if (x.type()!=y.type())
      throw error("incompatible types in diff");
    if (auto vx=any_cast<string>(&x))
      {
        // Hamming distance
        auto vy=any_cast<string>(y);
        double r=abs(double(vx->length())-double(vy.length()));
        for (size_t i=0; i<vx->length() && i<vy.length(); ++i)
          r += (*vx)[i]!=vy[i];
        return (*vx<vy)? -r: r;
      }
    if (auto vx=any_cast<double>(&x))
      return *vx-any_cast<double>(y);
    if (auto vx=any_cast<ptime>(&x))
      return 1e-9*(*vx-any_cast<ptime>(y)).total_nanoseconds();
    throw error("unsupported type");
  }
  
  string str(const boost::any& v, const string& format)
  {
    if (auto s=any_cast<std::string>(&v))
      return *s;
    else if (auto s=any_cast<const char*>(&v))
      return *s;
    else if (auto s=any_cast<double>(&v))
      return to_string(*s);
    else if (auto s=any_cast<ptime>(&v))
      if (format.empty())
        return to_iso_extended_string(*s);
      else
        {
          unique_ptr<time_facet> facet(new time_facet(format.c_str()));
          ostringstream os;
          os.imbue(locale(os.getloc(), facet.release()));
          os<<*s;
          return os.str();
        }
    else
      return "";
  }

  string XVector::timeFormat() const
  {
    if (dimension.type!=Dimension::time || empty()) return "";
    static const auto day=hours(24);
    static const auto month=day*30;
    static const auto year=day*365;
    auto dt=any_cast<ptime>(back())-any_cast<ptime>(front());
    if (dt > year*5)
      return "%Y";
    else if (dt > year)
      return "%b %Y";
    else if (dt > month*6)
      return "%b";
    else if (dt > month)
      return "%d %b";
    else if (dt > day)
      return "%d %H:%M";
    else if (dt > hours(1))
      return "%H:%M";
    else if (dt > minutes(1))
      return "%M:%S";
    else
      return "%s";
  }
  

}
