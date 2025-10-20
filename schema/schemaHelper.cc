/*
  @copyright Steve Keen 2020
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

#include "cairoItems.h"
#include "schemaHelper.h"
#include "selection.h"
#include "lasso.h"
#include "xml_common.h"
#include "minsky_epilogue.h"
#include "a85.h"
#include "zStream.h"
#include <zlib.h>
using namespace std;
using namespace classdesc;

namespace minsky
{
  classdesc::pack_t decode(const classdesc::CDATA& data)
  {
    if (data.empty()) return {};
    string trimmed; //trim whitespace
    for (auto c: data)
      if (!isspace(c)) trimmed+=c;
    
    vector<unsigned char> zbuf(a85::size_for_bin(trimmed.size()));
    // reverse transformation required to avoid the escape sequence ']]>'
    replace(trimmed.begin(),trimmed.end(),'~',']'); 
    a85::from_a85(trimmed.data(), trimmed.size(),zbuf.data());

    InflateZStream zs(zbuf);
    zs.inflate();
    return std::move(zs.output);
  }


  CDATA encode(const pack_t& buf)
  {
    if (!buf.size()) return CDATA();
    vector<unsigned char> zbuf(buf.size());
    DeflateZStream zs(buf, zbuf);
    zs.deflate();
    
    vector<char> cbuf(a85::size_for_a85(zs.total_out,false));
    a85::to_a85(zbuf.data(),zs.total_out, cbuf.data(), false);
    // this ensures that the escape sequence ']]>' never appears in the data
    replace(cbuf.begin(),cbuf.end(),']','~');
    return CDATA(cbuf.begin(),cbuf.end());
  }
}
