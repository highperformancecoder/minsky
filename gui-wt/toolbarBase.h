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

#include "guiDefs.h"
#include <Wt/Ext/ToolBar>

#ifndef _TOOLBARBASE_H
#define _TOOLBARBASE_H

namespace minsky { namespace gui {

using namespace Wt;

/**
 *  All toolbar in Minsky derive from this class.  Defines default behaviour
 *  commoon to all toolbars.
 */
class ToolbarBase : public Ext::ToolBar
{
private:
  typedef Ext::ToolBar _base_class;

public:
  /// Constructor
  ToolbarBase(WContainerWidget* parent);
};

}}  // namespace minsky::gui

#endif
