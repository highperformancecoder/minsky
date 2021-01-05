/*
  @copyright Steve Keen 2012
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

/** Utilities for walking an exprtk expression object.

To use, define the actions you want to happen for each type of expression
*/

#ifndef UNITEXPRESSIONWALKER_H
#define UNITEXPRESSIONWALKER_H
#include <variableType.h>

namespace minsky
{
  struct UnitsExpressionWalker
  {
    Units units;
    static bool check;
    double value=std::nan("");
    UnitsExpressionWalker() {}
    UnitsExpressionWalker(double x): value(x) {}
    UnitsExpressionWalker(const std::string& units): units(units) {}

    void checkSameDims(const UnitsExpressionWalker& x) const
    {if (check && x.units!=units) throw std::runtime_error("Incommensurate units");}
    void checkDimensionless() const
    {if (check && !units.empty()) throw std::runtime_error("Should be dimensionless");}
    UnitsExpressionWalker operator+=(const UnitsExpressionWalker& x)
    {checkSameDims(x); return *this;}
    UnitsExpressionWalker operator+(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return *this;}
    UnitsExpressionWalker operator-=(const UnitsExpressionWalker& x)
    {checkSameDims(x); return *this;}
    UnitsExpressionWalker operator-(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return *this;}

    UnitsExpressionWalker operator*=(const UnitsExpressionWalker& x)
    {
      for (auto& i: x.units)
        units[i.first]+=i.second;
      units.normalise();
      return *this;
    }
    UnitsExpressionWalker operator*(const UnitsExpressionWalker& x) const
    {auto tmp=*this; return tmp*=x;}
    UnitsExpressionWalker operator/=(const UnitsExpressionWalker& x)
    {
      for (auto& i: x.units)
        units[i.first]-=i.second;
      units.normalise();
      return *this;
    }
    UnitsExpressionWalker operator/(const UnitsExpressionWalker& x) const
    {auto tmp=*this; return tmp/=x;}
    UnitsExpressionWalker operator%(const UnitsExpressionWalker& x) const
    {return x;}
    UnitsExpressionWalker operator&&(const UnitsExpressionWalker& x) const
    {checkSameDims(x);return x;}
    UnitsExpressionWalker operator^(const UnitsExpressionWalker& x) const
    {checkSameDims(x);return x;}
    UnitsExpressionWalker operator||(const UnitsExpressionWalker& x) const
    {checkSameDims(x);return x;}
    bool operator<(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return true;}
    bool operator>(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return true;}
    bool operator<=(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return true;}
    bool operator>=(const UnitsExpressionWalker& x) const
    {checkSameDims(x); return true;}
    bool operator==(const UnitsExpressionWalker& x) const
    {return x.value==value;}
    bool operator!=(const UnitsExpressionWalker& x) const
    {return !operator==(x);}
  };

}
#endif
