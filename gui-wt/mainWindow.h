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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <Wt/WContainerWidget>

namespace minsky { namespace gui {

using namespace Wt;

/**
 *  The Minsky application main interface.  Contains the layout
 *  definition and creates the menu, toolbars, and
 *  the cannevas.
 *
 *  Derives drom Ext::Container to manage the entire viewport.
 */
class MainWindow : public WContainerWidget
{
public:
  MainWindow(Wt::WContainerWidget* parent = 0);
};

}}  // namespace minsky::gui

#endif
