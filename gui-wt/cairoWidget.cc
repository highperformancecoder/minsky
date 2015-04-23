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

#include <Wt/WPaintedWidget>
#include <Wt/WPainter>
#include <Wt/WRectArea>
#include "cairoWidget.h"
#include <ecolab_epilogue.h>
#include <algorithm>

namespace minsky { namespace gui {

using namespace Wt;
using namespace boost::filesystem;

CairoWidget::CairoWidget(WContainerWidget* parent)
  : WPaintedWidget(parent),
    buffer(new RasterBuffer(cairo_image_surface_create(CAIRO_FORMAT_RGB24, 200, 200))),
    entireArea(0),
    image(new WPainter::Image("cairo_widget", defaultWidth, defaultHeight)),
    clientWidth(defaultWidth),
    clientHeight(defaultHeight)
{
  setLayoutSizeAware(true);
}

CairoWidget::~CairoWidget()
{
}

void CairoWidget::paintEvent(WPaintDevice* paintDevice)
{
  //  Wt is so opaque we cannot reused this functionality directly.
  //  but instead have to save the png to file.
  //  it's not very pretty...
  WPainter painter(paintDevice);
  painter.fillRect(0, 0, clientWidth, clientHeight, WBrush(WColor("white")));

  if (!tmpFileName.empty())
  {
    //remove(tmpFileName);
  }
  else
  {
    tmpFileName = unique_path();
  }

  cairo_surface_t* port = \
      cairo_surface_create_similar_image(buffer->surface(), CAIRO_FORMAT_RGB24, viewport.width(), viewport.height());

  if (port)
  {
    cairo_t* context = cairo_create(port);
    if (context)
    {
      cairo_set_source_surface (context, buffer->surface(), viewport.x(), viewport.y());
      cairo_rectangle (context, 0, 0, viewport.width(), viewport.height());
      cairo_fill (context);

      image = ImagePtr(new WPainter::Image("/cairo_widget", viewport.width(), viewport.height()));

      if (!cairo_surface_write_to_png(port, WString(tmpFileName.native()).toUTF8().c_str()))
      {
        WRectF srcRect(0, 0, viewport.width(), viewport.height());
        WRectF dstRect(0, 0, clientWidth, clientHeight);
        painter.drawImage(dstRect, *(image.get()), srcRect);
        return;
      }
      cairo_destroy(context);
    }
    cairo_surface_destroy(port);
  }

  std::cout << "\n\nPAINT\n\n" << std::endl;
}

void CairoWidget::setCursor(Wt::Cursor cursor)
{
  if (entireArea)
  {
    delete entireArea;
  }
  entireArea = new WRectArea;
  addArea(entireArea);
  entireArea->setCursor(cursor);
}

void CairoWidget::layoutSizeChanged(int width, int height)
{
  if (width != clientWidth || height != clientHeight)
  {
    // recalculate the viewport.
    float ratioX, ratioY;

    ratioX = (clientWidth) ? (float(width) / float(clientWidth)) : 1;
    ratioY = (clientHeight) ? (float(height) / float(clientHeight)) : 1;

    float newWidth, newHeight;

    newWidth = viewport.width() * ratioX;
    newHeight = viewport.height() * ratioY;

    viewport.setX((newWidth - viewport.width()) / 2);
    viewport.setY((newHeight - viewport.height()) / 2);

    viewport.setWidth(newWidth);
    viewport.setHeight(newHeight);

    clientWidth = width;
    clientHeight = height;

    // this call will cause recursion, hence the test on entry.
    resize(WLength(width, WLength::Pixel), WLength(height, WLength::Pixel));

    update();
  }
}

}}  // namespace minsky::gui
