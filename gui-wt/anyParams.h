/*
  @copyright Steve Keen 2012
  @author Michael Roy
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

#ifndef ANYPARAMS_H_
#define ANYPARAMS_H_

#include <vector>
#include <boost/any.hpp>

namespace minsky { namespace gui {

class AnyParams : public std::vector<boost::any>
{
public:
  AnyParams(const boost::any& var)
  {
    push_back(var);
  }

  AnyParams()
    : std::vector<boost::any>()
  { }

  inline AnyParams(const AnyParams& other)
   : std::vector<boost::any>(other)
  { }

  inline AnyParams&  operator << (const boost::any& var)
  {
    push_back(var);
    return *this;
  }
};

}}  // namespace minsky::gui

#endif /* ANYPARAMS_H_ */
