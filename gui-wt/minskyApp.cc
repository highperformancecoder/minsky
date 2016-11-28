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

#include "mainWindow.h"
#include "mainMenu.h"
#include "minskyDoc.h"
#include "minskyApp.h"
#include "fileOpenDlg.h"
#include "globalPreferencesPropDlg.h"
#include "remoteClipBoard.h"
#include "abstractView.h"
#include <ecolab_epilogue.h>
#include <Wt/Ext/MessageBox>
#include <Wt/WText>
#include <Wt/Ext/Container>
#include <Wt/WFitLayout>
#include <Wt/WProgressBar>
#include <Wt/WResource>
#include <Wt/WTimer>
#include <Wt/WFileResource>
#include <Wt/WAnchor>
#include <stdio.h>      //  wfopen

namespace minsky { namespace gui {

using namespace Wt;
using std::string;
using classdesc::shared_ptr;

WApplication* MinskyApp::createApplication(const WEnvironment& env)
{
  return new MinskyApp(env);
}

MinskyApp::MinskyApp(const WEnvironment& env)
  : WApplication(env),
    clipboard(NULL),
    activeDoc(NULL),
    simTimer(NULL),
    activeView(NULL)
{
  //  resources first
  messageResourceBundle().use("./app-res/xml/text");

  //  TODO: create OS-specialized clipboard here.
  clipboard = new RemoteClipBoard;

  //  Create an empty document.
  activeDoc = new MinskyDoc;

  setTitle(WString::tr("app.title"));
  useStyleSheet("resources/themes/polished/wt.css");

  root()->setSelectable(false);
  root()->setOverflow(WContainerWidget::OverflowHidden);

  Ext::Container* container = new Ext::Container(root());
  WFitLayout* layout = new WFitLayout(container);
  layout->setContentsMargins(0,0,0,0);
  layout->addWidget(new MainWindow);

  //  load css overloads.
  useStyleSheet("/app-res/css/minsky.css");
}

MinskyApp::~MinskyApp()
{
  if (activeDoc)
  {
    delete activeDoc;
  }
  if (clipboard)
  {
    delete clipboard;
  }
}

StandardButton MinskyApp::closeActiveDocument()
{
  MinskyDoc* doc = getActiveDocument();
  if (doc)
  {
    StandardButton result = Yes;
    if (doc->isModified())
    {
      result = MessageBox(WString::tr("messagebox.savefilebeforeclose"),
                          Yes | No | Cancel);
      switch (result)
      {
      case Yes:
        try
        {
          fileSaveAs();
        }
        catch(...)
        {
          //  for some reason, the file was not saved.
          return Cancel;
        }
        break;

      case No:
        //  do not save file.
        break;

      default:
      case Cancel:
        //  do nothing...
        return Cancel;
      }
    }
    //  should be safe to close the file now.
    doc->close();
    return result;
  }
  return No;
}

void MinskyApp::fileNew()
{
  try
  {
    activeDoc->close();
  }
  GUI_CATCH_ALL()
}

void MinskyApp::fileOpen()
{
  //  Upload file...
  if (getActiveDocument() && !fileOpenDlgPtr)
  {
    if (Cancel != closeActiveDocument())
    {
      //
      //  File load is asynchronous because of browser security.
      //

      fileOpenDlgPtr = classdesc::shared_ptr<FileOpenDlg>(new FileOpenDlg("dialog.fileopen.title"));

      fileOpenDlgPtr->uploaded().connect(this, &MinskyApp::onTransferComplete);
      fileOpenDlgPtr->fileTooLarge().connect(this, &MinskyApp::onFileTooLarge);
      fileOpenDlgPtr->dataReceived().connect(this, &MinskyApp::onDataReceived);

      if (FileOpenDlg::Accepted == fileOpenDlgPtr->exec())
      {
        //  Disable user input while the file is being loaded
        root()->setDisabled(true);
      }
      else
      {
        fileOpenDlgPtr.reset();
      }
    }
  }
}

void MinskyApp::fileSave()
{
}

void MinskyApp::fileSaveAs()
{
  std::string fileName = "minsky.mky";
//  getDoc()->generateFileName(fileName);
  getDoc()->saveToFile(fileName);

  WFileResource* res = dynamic_cast<WFileResource*>(root()->find("minsky-save"));
  if (!res)
  {
    res = new WFileResource("application/octet-stream", fileName, root());
    res->setObjectName("minsky-save");
    res->suggestFileName(fileName);
    res->setDispositionType(WResource::Attachment); // should force open Save As, but doesn't
  }

  std::stringstream str;
//  str << "window.location=\"" << res->url() << "\"";
//  str << "document.body.appendChild(\"" << res->url() << "\")";
  str << "var a = document.createElement(\'a\');"
      << "a.href = \""     << res->url() << "\";"
      << "a.download = \'" << fileName << "\';"  // set the file name
      << "a.style.display = \'none\';"
      << "document.body.appendChild(a);"
      << "a.click(); "      // this is probably the key - simulating a click on a download link
      << "delete a;";       // we don't need this anymore

  doJavaScript(str.str(), false);
}

void MinskyApp::fileClose()
{
  closeActiveDocument();
}

void MinskyApp::fileProperties()
{
  MinskyDoc* doc = getActiveDocument();
  if (doc)
  {
    MinskyDoc::Properties props;
    doc->getProperties(props);
    //TODO: generate dialog.
  }
}

void MinskyApp::exit()
{
  if (Cancel == closeActiveDocument())
  {
    return;
  }
  // TODO: Add a html goodbye poge.
  this->quit();
}

void MinskyApp::help()
{
  // TODO:
}

void MinskyApp::about()
{
  // TODO:
}

StandardButton MinskyApp::MessageBox(const WString& strPrompt,
                                     StandardButton buttons,
                                     const string& strIconName,
                                     const WString& strTitle)
{
  // TODO: customize message box with icon...
  WString strT = strTitle;
  if (strT.empty())
  {
    strT = WString::tr("app.title");
  }
  MinskyApp::getApp()->root()->setDisabled(false);

  return Ext::MessageBox(root()).show(strT, strPrompt, buttons);
}

bool MinskyApp::getIconName(const string& id, string& imageName)
{
  imageName = id;

  for (string::iterator it = imageName.begin(); it != imageName.end(); ++it)
  {
    if (*it == '.')
    {
      *it = '-';
    }
  }
  imageName += ".gif";

  //  in out case, the URL is simply a directory name.
  string prefix = ".";
  string url = "/app-res/";

  imageName = url + imageName;

  FILE* file = fopen((prefix + imageName).c_str(), "r");
  if (file)
  {
    fclose(file);
  }
  else
  {
    imageName.clear();
  }

  return (file != NULL);
}

WTimer* MinskyApp::createSimulationTimer()
{
  if (!simTimer)
  {
    simTimer = new WTimer(this);
  }
  return simTimer;
}

void MinskyApp::destroySimulationTimer()
{
  if (simTimer)
  {
    delete simTimer;
    simTimer = NULL;
  }
}

WTimer* MinskyApp::simulationTimer()
{
  return simTimer;
}

void MinskyApp::editPreferences()
{
  GlobalPreferencesPropDlg dlg(preferences, root());
  dlg.exec();
}

void MinskyApp::onDataReceived(uint64_t bytesDone, uint64_t totalBytes)
{
  if (!fileOpenDlgPtr->progressBar())
  {
    fileOpenDlgPtr->setProgressBar(new WProgressBar);
  }

  if (fileOpenDlgPtr->progressBar())
  {
    fileOpenDlgPtr->progressBar()->setState(0, totalBytes, bytesDone);
  }
}

void MinskyApp::onTransferComplete()
{
  MinskyApp::getApp()->log("info")
      << "Transfer of file "
      << fileOpenDlgPtr->getClientFileName()
      << " complete.";

  getActiveDocument()->loadFromFile(fileOpenDlgPtr->getServerFileName());
  fileOpenDlgPtr.reset();
  root()->setDisabled(false);
}

void MinskyApp::onFileTooLarge(int64_t fileSize)
{
  MinskyApp::getApp()->log("info")
      << "Transfer of file "
      << fileOpenDlgPtr->getClientFileName()
      << " interrupted, file size (" << (fileSize >> 10) << " kB) too large, "
      << "consider increasing max-memory-request-size in wt_config.xml";

  //  TODO: stays stuck in destructor here...  But error should noit happen if
  //  is ok.

  fileOpenDlgPtr.reset();
  root()->setDisabled(false);
  MessageBox(WString::tr("messagebox.filetoobig"), Ok);
}

}}  // namespace minsky::gui
