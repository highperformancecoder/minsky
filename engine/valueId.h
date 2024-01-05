/*
  @copyright Steve Keen 2021
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

#ifndef VALUEID_H
#define VALUEID_H
#include <memory>
#include <string>

namespace minsky
{
  class Group;
  typedef std::shared_ptr<Group> GroupPtr;

  /// check that name is a valid valueId (useful for assertions)
  bool isValueId(const std::string& name);

  /// construct a valueId \@ name should be canonicalised
  std::string valueIdCanonical(size_t scope, const std::string& name);
  /// construct a valueId from fully qualified name \@ name should not be canonicalised
  std::string valueId(const std::string& name);

  /// construct a valueId. \@ name should not be canonicalised
  /// if name has leading :, then heirarchy is searched for matching local name
  std::string valueId(const GroupPtr& ref, const std::string& name);

  /// convert a raw name into a canonical name - this is not idempotent.
  std::string canonicalName(const std::string& name);
  
  /// starting from reference group ref, applying scoping rules to determine the actual scope of \a name
  /// If name prefixed by :, then search up group heirarchy for locally scoped var, otherwise return ref
  GroupPtr scope(GroupPtr ref, const std::string& name);

  /// value Id from scope and canonical name \a name
  std::string valueIdFromScope(const GroupPtr& scope, const std::string& name);
    
  /// extract scope from a qualified variable name
  /// @throw if name is unqualified
  size_t scope(const std::string& name);
  /// extract unqualified portion of name
  std::string uqName(const std::string& name);
}
#endif
