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

#ifndef CAIROIMAGERESOURCE_H_
#define CAIROIMAGERESOURCE_H_

#include <cairo_base.h>
#undef None
#include <Wt/WResource>
#include <Wt/WSignal>
#include <Wt/WPaintDevice>
#include <strstream>

namespace minsky { namespace gui {

class ::Wt::Http::Request;
class ::Wt::Http::Response;

/**
 * Web image resource for cairo raster surfaces. Surface has to
 * be a 24 bit RGB bitmap.
 */
class CairoImageResource: public Wt::WResource
{
public:
  /**
   * Exception thrown when HTTP request do not match held data.
   */
  struct httpDownloadRequestException
    : virtual boost::exception, virtual std::exception {};

private:
  typedef std::vector<char> Buffer;

private:
  Buffer  buffer;                 ///< transport buffer
  Wt::Signal<> transferComplete_; ///< Event signals end of transfer.

public:
  /**
   * Constructor.
   */
  CairoImageResource(WObject* parent);

  /**
   * Destructor.
   */
  virtual ~CairoImageResource();

  /**
   * Client should call this to start the streaming process.
   */
  void updateData(cairo_surface_t* surface);

  /**
   * Handles clients requests for streaming update.
   */
  virtual void handleRequest (const Wt::Http::Request &request, Wt::Http::Response &response);

  /**
   * Signal accessor for the client.
   */
  inline Wt::Signal<>& tranferComplete()
  {
    return transferComplete_;
  }

private:
  static cairo_status_t writeToStream (void* closure, const unsigned char* data, unsigned int length);
};

}}  // namespace minsky::gui

#endif /* CAIROIMAGERESOURCE_H_ */
