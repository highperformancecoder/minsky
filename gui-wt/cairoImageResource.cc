/*
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

#include "cairoImageResource.h"
#include <Wt/Http/Request>
#include <Wt/Http/Response>

namespace minsky { namespace gui {

using namespace Wt;

CairoImageResource::CairoImageResource(WObject* parent)
  : WResource(parent),
    WPaintDevice(),
    transferComplete_(this)
{
}

CairoImageResource::~CairoImageResource()
{
}

void CairoImageResource::updateData(cairo_surface_t* surface)
{
  cairo_surface_write_to_png_stream(&surface, &CairoImageResource::writeToStream, this);
}


void CairoImageResource::handleRequest (const Http::Request &request, Http::Response &response)
{
  if (request.method() == "GET")
  {
    size_t transferOffset = 0;
    Http::ResponseContinuation* continuation = request.continuation();

    if (continuation)
    {
      transferOffset = continuation->data();
    }

    if (transferOffset > buffer.size())
    {
      BOOST_THROW_EXCEPTION(httpDownloadRequestException());
    }

    size_t bytesToGo = buffer.size() - transferOffset;
    transferOffset += bytesToGo;  //  next

    while (bytesToGo)
    {
      const size_t BufferSize = 4096;
      char buffer[BufferSize];

      size_t n = std::max(bytesToGo, BufferSize);

      response.out().write(&(buffer[0]) + transferOffset, n);
      bytesToGo -= n;
    }

    if (transferOffset < buffer.size())
    {
        continuation = response.createContinuation();
        continuation->setData(transferOffset);
    }
    else
    {
      transferComplete_();
    }

    response.setMimeType("image/png");
    response.setContentLength(buffer.size());
  }
  else
  {
    //  reject others
    response.setStatus(403);
  }

}

cairo_status_t CairoImageResource::writeToStream(void* closure, const unsigned char* data, unsigned int length)
{
  CairoImageResource* obj = static_cast<CairoImageResource>(closure);

  if (obj)
  {
    obj->buffer.insert(obj->buffer.end(), data, data + length);

    return CAIRO_STATUS_SUCCESS;
  }
  return CAIRO_STATUS_NO_MEMORY;
}

}}  // namespace minsky::gui
