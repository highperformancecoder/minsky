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

#ifndef _MAINMENU_H
#define _MAINMENU_H

#include "guiDefs.h"
#include <Wt/Ext/ToolBar>
#include <Wt/WContainerWidget>
#include <Wt/Ext/Menu>

namespace minsky { namespace gui {

using namespace Wt;
using namespace std;

/**
 *	Application main frame menu.  Handles creation, display and command
 *	dispatch.
 *
 *	Commands are dispatched directly to the MinskyApp object if possible.  The
 *	ones that require dynamic handling are dispatched to MainMenu handlers
 *	where the target will be dynamically defined.
 *
 *	Commands that need this runtime refinement are usually targeted at the view
 *	that has the focus.
 *
 *  @todo At present the menu items are always enabled.
 *        Logic needs added.
 */
class MainMenu : public Ext::ToolBar
{
public:
  /**
   *  This exception is thrown whenever a call to WWidget::findWidget fails.
   */
  struct cantLocateWidgetException
    : virtual boost::exception,
      virtual std::exception {};

private:
  /**
   *  Shorthand to this class name.
   */
  typedef MainMenu _this_class;

  /**
   *  Shorthand to the base class.
   */
  typedef Ext::ToolBar _base_class;

public:
  /**
   *  Constructor.
   *  @param parent The parent container for the menu bar.
   */
  MainMenu(WContainerWidget* parent = 0);

private:
  /**
   *  Creates the file menu
   */
  void createFileMenu();

  /**
   *  Creates the file menu
   */
  void createEditMenu();

  /**
   *  Creates the file menu
   */
  void createViewMenu();

  /**
   *  Creates the file menu
   */
  void createHelpMenu();

  /**
   *  Handler for the 'File' main menu header.
   */
  void itemFileSelected();

  /**
   *  Handler for the 'Edit' main menu header.
   */
  void itemEditSelected();

  /**
   *  Handler for the 'View' main menu header.
   */
  void itemViewSelected();

  /**
   *  Handler for the 'Help' main menu header.
   */
  void itemHelpSelected();

  /**
   *  Creates a menu item header on the menu bar.
   */
  Ext::Menu* addMenu(const string& id);

  /**
   *  Creates a popup sub menu item from an ID.
   */
  static WMenuItem* addPopupItem(WPopupMenu* popup, const string& id);

  /**
   *  Adds a menu item from an ID.
   *  @param menu the parent for the newly created item.
   *  @param id a unique Id for the item to be created.
   *  @param target the target object for the event handler \pmethod
   */
  //  implemented in the .cc since it's private.
  template <class T, class V>
  static Ext::MenuItem* addMenuItem(Ext::Menu* menu, const string& id, T* target, void(V::*method)());

  /**
   *  Adds a menu item to one of the menus.
   *  @param menu the menu to add the item to.
   *  @param id   string ID for the item.  If a corresponding icon exists, it
   *              will be displayed next to the label.
   */
  static Ext::MenuItem* addMenuItem(Ext::Menu* menu, const string& id);

  //
  //  Handlers
  //
private:
  /**
   * Handler for Edit / Cut.
   */
  void editCut();

  /**
   * Handler for Edit / Copy.
   */
  void editCopy();

  /**
   * Handler for Edit / Paste.
   */
  void editPaste();

  /**
   * Handler for Edit / Group.
   */
  void editGroup();

  /**
   * Handler for Edit / Ungroup.
   */
  void editUngroup();
};

}}  // namespace minsky::gui

#endif
