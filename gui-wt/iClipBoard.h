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

#ifndef ICLIPBOARD_H_
#define ICLIPBOARD_H_

namespace minsky { namespace gui {

using std::string;
using std::wstring;

/**
 *  Unified access to the OS clipboard interface.  This clipboard is
 *  only used for Minsky data.  All other usual stuff, like text will
 *  be best handled by the OS.
 *
 *  This interface also provides a way for OS-independant support of Minsky
 *  data for the conventional clipboard, in case of local operation.
 *
 *  @todo: add interface for all types handled by the application.
 *
 */
class IClipBoard
{
public:
  /**
   *  Temporary placeholder
   *  @todo replace by Minsky data chunk type.
   */
  typedef void* DataChunk;

public:
  /**
   *  Destructor.
   */
  virtual ~IClipBoard() {}

  /**
   * Stores a copy of a data chunk to the clipboard.
   * @param data data to copy to the clipboard.
   */
  virtual void insert(const DataChunk& data) = 0;

  /**
   * Gets data from the clipboard.
   * @returns true if data was present on the clipboard.
   */
  virtual bool get(DataChunk& data) = 0;

  /**
   *  Indicates whether the clipboard contains data of a certain type
   */
  virtual bool contains(const DataChunk& data) = 0;
};

}} // namespace minsky::gui

#endif /* ICLIPBOARD_H_ */
