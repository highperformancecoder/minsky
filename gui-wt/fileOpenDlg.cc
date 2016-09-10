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
#include "fileOpenDlg.h"
#include <ecolab_epilogue.h>
#include <Wt/WFileUpload>
#include <Wt/Ext/Button>
#include <Wt/WContainerWidget>

namespace minsky { namespace gui {

using namespace Wt;

FileOpenDlg::FileOpenDlg(const WString& title)
  : Ext::Dialog(title)
{
  fileUpload = new WFileUpload;
  contents()->addWidget(fileUpload);

  Ext::Button* button = new Ext::Button(WString::tr("dialog.ok"));
  addButton(button);
  button->clicked().connect(this, &FileOpenDlg::onOk);

  button = new Ext::Button(WString::tr("dialog.cancel"));
  addButton(button);
  button->clicked().connect(this, &FileOpenDlg::onCancel);
}

FileOpenDlg::~FileOpenDlg()
{
}

void FileOpenDlg::onOk()
{
  if (fileUpload && fileUpload->canUpload())
  {
    MinskyApp::getApp()->log("info")
        << "Starting upload of file "
        << fileUpload->clientFileName();
    fileUpload->upload();
    done(Accepted);
  }
  else
  {
    done(Rejected);
  }
}

void FileOpenDlg::onCancel()
{
  done(Rejected);
}

}}  // namespace minsky::gui

