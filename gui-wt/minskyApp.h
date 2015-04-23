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

#ifndef _MINSKYAPP_H
#define _MINSKYAPP_H

#include "guiDefs.h"
#include "globalPreferences.h"
#include "displaySettings.h"
#include <string>
#include <exception>
#include <boost/exception/all.hpp>
#include <TCL_obj_base.h>
#include "iClipBoard.h"
#include "viewManager.h"
#include "classdesc_access.h" //  use classdesc shared_ptr.
#undef None
#include <Wt/WApplication>

namespace minsky { namespace gui {

using namespace Wt;
using std::wstring;
using std::string;

//  Forward declarations.
class MinskyDoc;
class AbstractView;
class MainWindow;

class FileOpenDlg;

/**
 *  Minsky Application class.
 *
 *  The main Wt application object.
 *
 */
class MinskyApp : public WApplication, public ViewManager
{
public:
  /***
   *  Exception thrown when the pointer to the application is accessed after
   *  the session was terminated and
   */
  struct nullAppPointerException
    : virtual std::exception,
      virtual boost::exception {};

  /***
   *  Exception thrown when the pointer to the document is accessed and
   *  is NULL
   */
  struct nullDocPointerException
    : virtual std::exception,
      virtual boost::exception {};

  /***
   *  Exception thrown when the document was not destroyed before the session
   *  is closed.  This error usually indicates an unsaved document and an
   *  abruptly terminated session.
   */
  struct docNotDestroyedOnExit
    : virtual std::exception,
      virtual boost::exception {};

private:
  IClipBoard* clipboard;    ///< The clipboard.
  MinskyDoc* activeDoc;     ///< The active document. Currently MinskyApp handles a single document.
  WTimer*    simTimer;      ///< The simulation timer. Pointer belongs to Wt.

  classdesc::shared_ptr<FileOpenDlg> fileOpenDlgPtr;

  DisplaySettings displayOptions; ///< Application-wide display and drawing options.
  GlobalPreferences preferences;  ///< Global preferences.

  /**
   *  Weak pointer to the active view.
   *
   *  @note: Currently MinskyApp handles a single view.
   *         since this likely will change, do not access
   *         directly, use the method getActiveView to access.
   */
  AbstractView* activeView;

private:
/**
 *  Creates the Minsky application object.
 *
 *  @param env		The environment, including information on the client.
 *
 *	@todo	Add security checks/ modify environment maybe?
 */
  MinskyApp(const WEnvironment& env);

public:
  virtual ~MinskyApp(); ///< Destructor.

public:
  /**
   *  Creates an application object.
   *
   *  @returns a pointer to the unique application object held
   *  in MinskyApp::theApp.
   */
  static WApplication *createApplication(const WEnvironment& env);

  /**
   *  Accesses the application pointer.
   *
   *	There is only one application object for the application.
   *
   *  @returns The application pointer for the current session.
   *
   *  @throws nullAppPointerException
   */
  static inline MinskyApp* getApp()
  {
    MinskyApp* app = dynamic_cast<MinskyApp*>(WApplication::instance());
    if (!app)
    {
      BOOST_THROW_EXCEPTION(nullAppPointerException());
    }
    return app;
  }

  /**
   * Accesses the display options.
   */
  static inline DisplaySettings& displaySettings()
  {
    return getApp()->displayOptions;
  }

  /**
   *  Accesses the active document
   *
   *  MinskyApp does not support multiple documents at this time.
   *
   *  @returns A pointer to the active document.
   *  @throws nullAppPointerException, nullDocPointerException
   */
  static inline MinskyDoc* getDoc()
  {
    return getApp()->getActiveDocument();
  }

  /**
   * Close the current file, lets the user pick a name, eventually.
   */
  StandardButton closeActiveDocument();

  /**
   *  Indicates whether the application pointer is valid.  This should
   *  normally always return true.
   *
   *	There is only one application object for the application.
   *
   *  @returns The pointer value held in MinslyApp::theApp.
   */
  static inline bool isValid()
  {
    MinskyApp* app = dynamic_cast<MinskyApp*>(WApplication::instance());
    return (app != NULL);
  }

  /**
   *  Returns the view that has the focus.  There is only one view defined for now.
   *
   *  @returns A pointer to the current canvas.
   */
  inline AbstractView* getActiveView()
  {
    return activeView;
  }

  /**
   *  Indicates whether the active document pointer is valid.
   *  The active document is the document which has user focus.
   *  @returns  true if the pointer can be safely accessed.
   *  @see  MinskyApp::getActiveDocument(), \dminskyDoc
   */
  inline bool hasActiveDocument() const
  {
    return (activeDoc != NULL);
  }

  /**
   *  Accesses the active document.  The active document is
   *  the document which has user focus.
   *  @returns  A pointer top the active document.
   *  @throws A nullDocPointerException if the Application has
   *          no open document, to prevent caller from using the
   *          returned pointer.
   *  @see  MinskyApp::hasActiveDocument()
   */
  inline MinskyDoc* getActiveDocument()
  {
    if (!activeDoc)
    {
      BOOST_THROW_EXCEPTION(nullDocPointerException());
    }
    return activeDoc;
  }

  /**
   *  Sets the active document.  The active document is
   *  the document which has user focus.
   */
  inline void setActiveDocument(MinskyDoc* doc)
  {
    activeDoc = doc;
  }

  //  Menu commands.
  //
public:
  /**
   *  Creates a new File.
   */
  void fileNew();

  /**
   *  Opens an existing file, after prompting the user for a path.
   */
  void fileOpen();

  /**
   *  Saves the current file.
   */
  void fileSave();

  /**
   *  Saves the current file, but let the user pick a name.
   */
  void fileSaveAs();

  /**
   * Close the current file, lets the user pick a name, eventually.
   */
  void fileClose();

  /**
   * Displays file properties, lets the user pick a name, eventually.
   */
  void fileProperties();

  /**
   *  Inserts a file as a group.
   */
  void insertFile(const wstring& pathName/*, ...*/);

  /**
   *  Exits the application.
   */
  void exit();

  /**
   *  Copies the selection to the clipboard.
   */
  void editCopy();

  /**
   * Pastes the Selection.
   */
  void editPaste();

  /**
   *  Display help.
   */
  void help();

  /**
   *  Displays the About dialog.
   */
  void about();

  //
  //  Utilities.
  //
public:
  /**
   *  Displays a message box.  Will block until the user has supplied a response.
   *  @param strPrompt  The prompt to display
   *  @param buttons    A combinaison of standard Wt buttons to display.
   *  @param strIconName  The file name for an optional icon to illustrate the
   *                      message.
   *  @param strTitle   The title for the message box.  Uses the application
   *                    name if none is supplied.
   *  @returns  The button pressed by the user.
   *  @too    Currently no icon is displayed.
   *
   */
  StandardButton MessageBox(const WString& strPrompt, StandardButton buttons, \
                            const string& strIconName = string(), const WString& strTitle = WString());

  /**
   *  Check if an image exists for a named item.
   *
   *  Items are named in a dot form ("group.id"), which can be a problem for
   *  some OSes. this function generate a name using the format "group-id.png"
   *  and check if the file exists in the resources directory.
   *  @param id         The item ID.
   *  @param imageName  The file name of the image resource, or a null string if
   *                    the file could not be found.
   *  @returns true if the image exists.
   *
   *  @note Should eventually be upgraded to look in a list of directories and
   *        different file extensions.
   */
    static bool getIconName(const string& id, string& imageName);

   /**
    * Creates the simulation timer.
    */
    WTimer* createSimulationTimer();

    /**
     * destroys the simulation timer.
     */
    void destroySimulationTimer();

    /**
     * Returns a pointer to the simulation timer.
     */
    WTimer* simulationTimer();

    /**
     * Opens the preferences dialog.
     */
    void editPreferences();

private:
    /**
     *  Handles received data blocks for loading files.
     */
    void onDataReceived(uint64_t bytesDone, uint64_t totalBytes);

    /**
     *  Handles completion of incoming model file transfer.
     */
    void onTransferComplete();

    /**
     * Handles error when the size of the file transfer is larger than what the
     * server can handle.
     */
    void onFileTooLarge(int64_t fileSize);
};

}}  // namespace minsky::gui

#endif
