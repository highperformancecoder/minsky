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

#include "clipboard.h"
#include "libclipboard.h"

using namespace std;

namespace minsky
{
  struct Clipboard::Impl
  {
    clipboard_c* clipboard;
    Impl(): clipboard(clipboard_new(nullptr)) {}
    ~Impl() {clipboard_free(clipboard);}
  };

  Clipboard::Clipboard(): pimpl(make_shared<Impl>()) {}
  
  std::string Clipboard::getClipboard() const
  {
    if (pimpl->clipboard)
      {
        auto s=clipboard_text(pimpl->clipboard);
        return s? s: "";
      }
    return {};
  }
  
  void Clipboard::putClipboard(const std::string& text) const
  {
    if (pimpl->clipboard)
      if (text.empty())
        clipboard_clear(pimpl->clipboard, LCB_CLIPBOARD);
      else
        clipboard_set_text(pimpl->clipboard, text.c_str());
  }

  
}
