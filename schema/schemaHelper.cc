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

#include "schemaHelper.h"
#include "selection.h"
#include "minsky_epilogue.h"
#include "a85.h"
#include <zlib.h>
using namespace std;

namespace
{
  // nice RAII wrappers around zlib's data structures
  struct ZStream: public z_stream
  {
    ZStream(Bytef* input, size_t inputSize, Bytef* output, size_t outputSize)
    {
      next_in=input;
      avail_in=inputSize;
      next_out=output;
      avail_out=outputSize;
      zfree=Z_NULL;
      zalloc=Z_NULL;
    }
    void throwError() const {
      throw runtime_error(string("compression failure: ")+(msg? msg:""));
    }
  };
    
  struct DeflateZStream: public ZStream
  {
    template <class I, class O>
    DeflateZStream(const I& input, O& output):
      ZStream((Bytef*)input.data(), input.size(),
              (Bytef*)output.data(), output.size())
    {
      if (deflateInit(this,9)!=Z_OK) throwError();
    }
    ~DeflateZStream() {deflateEnd(this);}
    void deflate() {
      if (::deflate(this,Z_FINISH)!=Z_STREAM_END) throwError();
    }
  };
    
  struct InflateZStream: public ZStream
  {
    classdesc::pack_t output{256};
    Bytef* inputData;
    size_t inputSize;
      
    template <class I>
    InflateZStream(const I& input):
      ZStream((Bytef*)input.data(), input.size(), 0,0),
      inputData((Bytef*)input.data()),inputSize(input.size())
    {
      next_out=(Bytef*)output.data();
      avail_out=output.size();
      if (inflateInit(this)!=Z_OK) throwError();
    }
    ~InflateZStream() {inflateEnd(this);}

    void inflate() {
      int err;
      while ((err=::inflate(this,Z_SYNC_FLUSH))==Z_OK)
        {
          // try doubling size
          output.resize(2*output.size());
          next_out=(Bytef*)(output.data())+total_out;
          avail_out=output.size()-total_out;
          next_in=inputData+total_in;
          avail_in=inputSize-total_in;
        }
      if (err!=Z_STREAM_END) throwError();
    }
    void throwError() {
      throw runtime_error(string("compression failure: ")+(msg? msg:""));
    }
  };
}

namespace minsky
{
  classdesc::pack_t decode(const classdesc::CDATA& data)
  {
    string trimmed; //trim whitespace
    for (auto c: data)
      if (!isspace(c)) trimmed+=c;
    
    vector<unsigned char> zbuf(a85::size_for_bin(trimmed.size()));
    // reverse transformation required to avoid the escape sequence ']]>'
    replace(trimmed.begin(),trimmed.end(),'~',']'); 
    a85::from_a85(trimmed.data(), trimmed.size(),zbuf.data());

    InflateZStream zs(zbuf);
    zs.inflate();
    return move(zs.output);
  }


  classdesc::CDATA encode(const classdesc::pack_t& buf)
  {
    vector<unsigned char> zbuf(buf.size());
    DeflateZStream zs(buf, zbuf);
    zs.deflate();
    
    vector<char> cbuf(a85::size_for_a85(zs.total_out,false));
    a85::to_a85(&zbuf[0],zs.total_out, &cbuf[0], false);
    // this ensures that the escape sequence ']]>' never appears in the data
    replace(cbuf.begin(),cbuf.end(),']','~');
    return CDATA(cbuf.begin(),cbuf.end());
  }
}
