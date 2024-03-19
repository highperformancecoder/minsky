/*
  @copyright Steve Keen 2013
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
#include "cairo_base.h"
#include "group.h"
#include "variableType.h"
#include "lasso.h"
#include "tensorVal.h"
#include "tensorVal.xcd"
#include "tensorVal.rcd"
#include "minsky_epilogue.h"
using namespace classdesc;

#include <string>
#include <sstream>
using namespace std;

namespace minsky
{
  string VariableType::typeName(int t)
  {return enumKey<Type>(t);}

  string Units::str() const
  {
    ostringstream s;
    s<<*this;
    return s.str();
  }

  string Units::latexStr() const
  {
    string s;
    for (auto& i: *this)
      if (!i.first.empty() && i.second!=0)
        {
          s+=i.first;
          if (i.second!=1)
            s+="^{"+to_string(i.second)+"}";
          else
            s+=" ";
        }
    return s;
  }
        
  
  Units::Units(const string& x)
  {
    if (x.empty()) return;
    // split into numerator and denominator components if / found
    auto divPos=x.find('/');
    vector<string> components;
    components.push_back(x.substr(0,divPos));
    if (divPos!=string::npos)
      components.push_back(x.substr(divPos+1));
    
    if (components.back().find('/')!=string::npos)
      throw runtime_error("too many division signs: "+x);
    
    int coef=1;
    for (auto& i: components)
      {
        if (i.empty())
          throw runtime_error("empty numerator or denominator: "+x);
        const char* b=i.c_str();
        for (const char*j=b;;)
          {
            if (*j=='^'||*j=='\0'||isspace(*j))
              {
                const string name(b,j); // stash end of unit name
                if (name.empty())
                  throw runtime_error("empty unit name: "+x);
                while (isspace(*j)) ++j;
                if (*j=='^')
                  {
                    auto k=j+1;
                    const int v=strtol(k,const_cast<char**>(&j),10);
                    if (j==k)
                      throw runtime_error("invalid exponent: "+x);
                    if (name!="1")
                      (*this)[name]+=coef*v;
                  }
                else if (name!="1")
                  (*this)[name]+=coef;
                if ((*this)[name]==0)
                  erase(name);
                while (isspace(*j)) ++j;
                b=j; // update to next unit name
                if (*j=='\0') break; //loop exits here
              }
            else
              ++j;
          }
        if (b==i.c_str()) // we haven't found any exponents
          emplace(i,coef);
        coef*=-1;
      }
  }


}
//CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::VariableType);
