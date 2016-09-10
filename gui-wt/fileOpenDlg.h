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

#ifndef FILEOPENDLG_H_
#define FILEOPENDLG_H_

#include <Wt/Ext/Dialog>
#include <Wt/WSignal>
#include <Wt/WFileUpload>

namespace minsky { namespace gui {

/**
 * File selection dialog for loading files.
 *
 * For now, we have to deal with browser security restrictions, so the loading
 * is done in 2 steps.
 *
 * File loading is done asynchronously.  Event handlers should be plugged into
 * the signal slots uploaded(), fileTooLarge(), and dataReceived().
 */

class FileOpenDlg: public Wt::Ext::Dialog
{
private:
  Wt::WFileUpload* fileUpload;      ///< Weak pinter to the uploader,
                                    ///   pointer owned by Wt library.

public:
  /**
   * Constructor.
   */
  FileOpenDlg(const Wt::WString& title = Wt::WString());

  /**
   * Destructor.
   */
  virtual ~FileOpenDlg();

  /**
   * Returns the file name selected by the user.  The file name is only valid
   * if user has selected 'OK' button.  Check must be done with return code
   * from exec()
   */
  inline const std::string getClientFileName() const
  {
    return fileUpload->clientFileName().toUTF8();
  }

  /**
   * Returns the name of the file saved server side.
   */
  inline const std::string getServerFileName() const
  {
    return fileUpload->spoolFileName();
  }

  /**
   * Signal is raised when file transfer is complete.
   */
  inline Wt::EventSignal<>& uploaded()
  {
    return fileUpload->uploaded();
  }

  /**
   *  Signal raised when a block of data has been received.  It is mainly used
   *  to update the progress bar, if applicable.
   */
  inline Wt::Signal<uint64_t, uint64_t>& dataReceived()
  {
    return fileUpload->dataReceived();
  }

  /**
   * This signal is raised when the file is larger than the maximum size the
   * http server allows.
   *
   * For the built-in http server, this is controlled by parameter
   * max-request-size in wt_config.xml.
   */
  inline Wt::Signal<int64_t>& fileTooLarge()
  {
    return fileUpload->fileTooLarge();
  }

  inline Wt::WProgressBar* progressBar()
  {
    return fileUpload->progressBar();
  }

  inline void setProgressBar(Wt::WProgressBar* progressBar)
  {
    fileUpload->setProgressBar(progressBar);
  }

private:
  /**
   * OK button handler.
   */
  void onOk();

  /**
   * Cancel button handler.
   */
  void onCancel();
};

}}  // namespace minsky::gui

#endif /* FILEOPENDLG_H_ */
