/*
  @copyright Steve Keen 2020
  @author Russell Standish
  @author Wynand Dednam
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

#ifndef ZSTREAM_H
#define ZSTREAM_H
#include <zlib.h>
#include <cstring>

namespace minsky
{
  // nice RAII wrappers around zlib's data structures
  struct ZStream: public z_stream
  {
    ZStream(Bytef* input, std::size_t inputSize, Bytef* output, std::size_t outputSize)
    {
      // Zero-initialize the entire z_stream structure to avoid uninitialized fields
      memset(static_cast<z_stream*>(this), 0, sizeof(z_stream));
      next_in=input;
      avail_in=inputSize;
      next_out=output;
      avail_out=outputSize;
      zfree=Z_NULL;
      zalloc=Z_NULL;
    }
    void throwError() const {
      throw std::runtime_error(std::string("compression failure: ")+(msg? msg:""));
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
    std::size_t inputSize;
      
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
      throw std::runtime_error(std::string("compression failure: ")+(msg? msg:""));
    }
  };
  
  struct InflateFileZStream: public ZStream
  {
    std::string output;
    Bytef* inputData;
    std::size_t inputSize;
      
    template <class I>
    InflateFileZStream(const I& input):
      ZStream((Bytef*)input.data(), input.size(), 0,0),
      inputData((Bytef*)input.data()),inputSize(input.size())
    {
      next_out=(Bytef*)output.data();
      avail_out=output.size();
      if (inflateInit2(this,-MAX_WBITS)!=Z_OK) throwError();  //none of MAX_WBITS + 16, MAX_WBITS or -MAX_WBITS works. see https://stackoverflow.com/questions/1838699/how-can-i-decompress-a-gzip-stream-with-zlib
    }
    ~InflateFileZStream() {inflateEnd(this);}

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
      throw std::runtime_error(std::string("compression failure: ")+(msg? msg:""));
    }
  };  
}

#include "zStream.cd"
#endif
