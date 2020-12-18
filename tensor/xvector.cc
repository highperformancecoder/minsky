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

#include "xvector.h"
#include <error.h>
#include <regex>
#include "minsky_epilogue.h"
using ecolab::error;

using namespace std;

#include <boost/date_time.hpp>
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

#include <time.h>

namespace civita
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
      {
        if (i->type()!=j->type())
          {
            cout << i->type().name() << " "<<j->type().name() << endl;
            return false;
          }
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
          static regex screwyDates{R"(%([mdyY])[^%]%([mdyY])[^%]%([mdyY]))"};
          smatch m;
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
          else if (regex_match(dim.units, m, screwyDates)) // handle dates with 1 or 2 digits see Ravel ticket #35
            {
              static regex valParser{R"((\d+)\D(\d+)\D(\d+))"};
              smatch val;
              if (regex_match(s, val, valParser))
                {
                  int day=0, month=0, year=0;
                  for (size_t i=1; i<val.size(); ++i)
                    {
                      
                      int v;
                      try
                        {v=stoi(val[i]);}
                      catch (...)
                        {throw runtime_error(val[i].str()+" is not an integer");}
                      switch (m.str(i)[0])
                        {
                        case 'd': day=v; break;
                        case 'm': month=v; break;
                        case 'y':
                          if (v>99) throw runtime_error(val[i].str()+" is out of range for %y");
                          year=v>68? v+1900: v+2000;
                          break;
                        case 'Y': year=v; break;
                        }
                    }
                  return ptime(date(year,month,day));
                }
              else
                throw runtime_error(s+" doesn't match "+dim.units);
            }
          else if (!dim.units.empty())
            {
              istringstream is(s);
              is.imbue(locale(is.getloc(), new time_input_facet(dim.units.c_str())));
              ptime pt;
              is>>pt;
              if (pt.is_special())
                throw error("invalid date/time: %s",s.c_str());
              return pt;
              // note: boost time_input_facet too restrictive, so this was a strptime attempt. See Ravel ticket #35
              // strptime is not available on Windows alas
              //              struct tm tm;
              //              memset(&tm,0,sizeof(tm));
              //              if (char* next=strptime(s.c_str(), dim.units.c_str(), &tm))
              //                try
              //                  {
              //                    return ptime(date(tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday), time_duration(tm.tm_hour,tm.tm_min,tm.tm_sec));
              //                  }
              //                catch (...)
              //                  {
              //                    cout << s << " " << tm.tm_year<<tm.tm_mon << " " << tm.tm_mday << endl;
              //                    throw;
              //                  }
              //              else
              //                throw error("invalid date/time: %s",s.c_str());
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
      throw runtime_error(string("incompatible types ")+x.type().name()+" and "+y.type().name()+" in diff");
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

  // format a string with two integer arguments
  string formatString(const std::string& format, int i, int j)
  {
    char r[512];
    snprintf(r,sizeof(r),format.c_str(),i,j);
    return r;
  }
  
  string str(const boost::any& v, const string& format)
  {
    string::size_type pq;
    if (auto s=any_cast<std::string>(&v))
      return *s;
    else if (auto s=any_cast<const char*>(&v))
      return *s;
    else if (auto s=any_cast<double>(&v))
      return to_string(*s);
    else if (auto s=any_cast<ptime>(&v))
      if (format.empty())
        return to_iso_extended_string(*s);
      else if ((pq=format.find("%Q"))!=string::npos)
        {
          auto pY=format.find("%Y");
          if (pY==string::npos)
            throw error("year not specified in format string");
            
          // replace %Q and %Y with %d
          string sformat=format;
          for (size_t i=1; i<sformat.size(); ++i)
            if (sformat[i-1]=='%' && (sformat[i]=='Q' || sformat[i]=='Y'))
              sformat[i]='d';
          char result[100];
          auto tm=to_tm(s->date());
          if (pq<pY)
            return formatString(sformat,tm.tm_mon/3+1, tm.tm_year+1900);
          else
            return formatString(sformat, tm.tm_year+1900, tm.tm_mon/3+1);
        }
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
    auto f=any_cast<ptime>(front()), b=any_cast<ptime>(back());
    if (f>b) std::swap(f,b);
    auto dt=b-f;
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
  
  void XVector::imposeDimension()
  {
    // check if anything to be done
    switch (dimension.type)
      {
      case Dimension::string:
        if (checkType<string>()) return;
        break;
      case Dimension::value:
        if (checkType<double>()) return;
        break;
      case Dimension::time:
        if (checkType<ptime>()) return;
        break;
      }

    for (auto& i: *this)
      i=anyVal(dimension, str(i));
    assert(checkThisType());
  }

}
