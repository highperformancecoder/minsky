/*
  @copyright Steve Keen 2021
  @author Russell Standish
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

#ifndef CLIPBOARD_H
#define CLIPBOARD_H
#include <memory>
#include <string>

namespace minsky
{
  /// An RAII wrapper around jtanx's libclipboard (https://github.com/jtanx/libclipboard)
  class ClipBoard
  {
    struct Impl;
    std::shared_ptr<Impl> pimpl;
  public:
    ClipBoard();
    /// return clipboard contents as UTF8 text
    std::string getClipboard() const;
    /// puts UTF8 text string on clipboard
    void putClipboard(const std::string&) const;
  };
}

#endif
