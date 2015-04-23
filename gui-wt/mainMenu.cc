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

#include "minskyApp.h"
#include <boost/exception/all.hpp>
#include <algorithm>
#include <Wt/Ext/Button>
#include <Wt/Ext/Menu>
#include <Wt/WMenuItem>
#include <Wt/WPopupMenu>
#include "mainMenu.h"
#include "minskyDoc.h"
#include "abstractView.h"
#include "canvasView.h"
#include <ecolab_epilogue.h>

namespace minsky { namespace gui {

using namespace Wt;
using namespace boost;
using namespace std;

template <class T, class V>
Ext::MenuItem* MainMenu::addMenuItem(Ext::Menu* menu, const string& id, T* target, void(V::*method)())
{
  string iconName;
  Ext::MenuItem* item;

  if (MinskyApp::getIconName(id, iconName))
  {
    item = menu->addItem(iconName, WString::tr(id), target, method);
  }
  else
  {
    item = menu->addItem(WString::tr(id), target, method);
  }
  return item;
}

MainMenu::MainMenu(WContainerWidget* parent)
  :   _base_class(parent)
{
  setSelectable(false);
  createFileMenu();
  createEditMenu();
  createViewMenu();
  createHelpMenu();
}

void MainMenu::createFileMenu()
{
  Ext::Menu* menu = addMenu("menu.file");

  addMenuItem(menu, "menu.file.new",    MinskyApp::getApp(), &MinskyApp::fileNew);
  addMenuItem(menu, "menu.file.open",   MinskyApp::getApp(), &MinskyApp::fileOpen);
  addMenuItem(menu, "menu.file.saveas", MinskyApp::getApp(), &MinskyApp::fileSaveAs);
  addMenuItem(menu, "menu.file.close",  MinskyApp::getApp(), &MinskyApp::fileClose);
  menu->addSeparator();
  addMenuItem(menu, "menu.file.properties", MinskyApp::getApp(), &MinskyApp::fileProperties);
  menu->addSeparator();
  addMenuItem(menu, "menu.file.exit",   MinskyApp::getApp(), &MinskyApp::exit);
}

void MainMenu::createEditMenu()
{
  Ext::Menu* menu = addMenu("menu.edit");

  addMenuItem(menu, "menu.edit.cut",      this, &MainMenu::editCut);
  addMenuItem(menu, "menu.edit.copy",     this, &MainMenu::editCopy);
  addMenuItem(menu, "menu.edit.paste",    this, &MainMenu::editPaste);
  menu->addSeparator();
  addMenuItem(menu, "menu.edit.group",    this, &MainMenu::editGroup);
  addMenuItem(menu, "menu.edit.ungroup",  this, &MainMenu::editUngroup);
  menu->addSeparator();
  addMenuItem(menu, "menu.edit.preferences", MinskyApp::getApp(), &MinskyApp::editPreferences);
}

void MainMenu::createViewMenu()
{
  Ext::Menu* menu = addMenu("menu.view");
  addMenuItem(menu, "TODO:");
}

void MainMenu::createHelpMenu()
{
  Ext::Menu* menu = addMenu("menu.help");

  addMenuItem(menu, "menu.help.help",   MinskyApp::getApp(), &MinskyApp::help);
  menu->addSeparator();
  addMenuItem(menu, "menu.help.about",  MinskyApp::getApp(), &MinskyApp::about);
}

Ext::Menu* MainMenu::addMenu(const string& id)
{
  string iconName;
  Ext::Menu* menu = new Ext::Menu;

  if (MinskyApp::getIconName(id, iconName))
  {
    addButton(iconName, WString::tr(id), menu);
  }
  else
  {
    addButton(WString::tr(id), menu);
  }
  return menu;
}

Ext::MenuItem* MainMenu::addMenuItem(Ext::Menu* menu, const string& id)
{
  string iconName;
  Ext::MenuItem* item;

  if (MinskyApp::getIconName(id, iconName))
  {
    item = menu->addItem(iconName, WString::tr(id));
  }
  else
  {
      item = menu->addItem(WString::tr(id));
  }
  return item;
}

void MainMenu::editCut()
{
  AbstractView* view = MinskyApp::getApp()->getActiveView();
  if (view)
  {
//    view->editCut();
  }
}

void MainMenu::editCopy()
{
  AbstractView* view = MinskyApp::getApp()->getActiveView();
  if (view)
  {
//    view->editCopy();
  }
}

void MainMenu::editPaste()
{
  AbstractView* view = MinskyApp::getApp()->getActiveView();
  if (view)
  {
//    view->editPaste();
  }
}

void MainMenu::editGroup()
{
  CanvasView* view = dynamic_cast<CanvasView*>(MinskyApp::getApp()->getActiveView());
  if (view)
  {
    //TODO:
    //view->editGroup();
  }
}

void MainMenu::editUngroup()
{
  CanvasView* view = dynamic_cast<CanvasView*>(MinskyApp::getApp()->getActiveView());
  if (view)
  {
    //TODO:
    //view->editUngroup();
  }
}

}}  // namespace minsky::gui

