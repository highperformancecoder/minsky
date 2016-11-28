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
#ifndef CLASSDESC_ACCESS_H
#define CLASSDESC_ACCESS_H

#include <xml_pack_base.h>
#include <xml_unpack_base.h>
#include <xsd_generate_base.h>

// redefine this to include xml_* descriptors
#undef CLASSDESC_ACCESS
#define CLASSDESC_ACCESS(type)                              \
  friend struct classdesc_access::access_pack<type>;        \
  friend struct classdesc_access::access_unpack<type>;      \
  friend struct classdesc_access::access_xml_pack<type>;    \
  friend struct classdesc_access::access_xml_unpack<type>;  \
  friend struct classdesc_access::access_xsd_generate<type>   

#endif
