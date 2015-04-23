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
#include "mainMenu.h"
#include "mainWindow.h"
#include "canvasView.h"
#include "mainToolbar.h"
#include <ecolab_epilogue.h>
#include <Wt/WContainerWidget>
#include <Wt/WToolBar>
#include <Wt/WDefaultLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WBorderLayout>
#include <Wt/WFitLayout>
#include <Wt/Ext/Container>
#include <Wt/WScrollArea>
#include <memory>

namespace minsky { namespace gui {

using namespace Wt;

MainWindow::MainWindow(WContainerWidget* parent)
  : WContainerWidget(parent)
{
  //
  //  It is a basic layout for a frame, with a menu, and an edit area, and
  //  an optional status bar.
  //
  //  Pointers are owned by layout.  Not by this.

  setLayoutSizeAware(true);

  std::unique_ptr<MainMenu>   menu(new MainMenu);
  std::unique_ptr<MainToolbar> toolbar(new MainToolbar);
  //std::unique_ptr<WFitLayout> canvasHolder(new WFitLayout);
  std::unique_ptr<CanvasView> canvas(new CanvasView(*(MinskyApp::getApp()->getActiveDocument())));

  toolbar->initialize(*canvas);
  canvas->initialize();

  //  Layouts should have a parent before inserting items.
  WVBoxLayout* layout       = new WVBoxLayout(this);

  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);
  layout->addWidget(menu.release());
  layout->addWidget(toolbar.release());
  layout->addWidget((canvas.release())->parent(), 1);
}

}}  // namespace minsky::gui
