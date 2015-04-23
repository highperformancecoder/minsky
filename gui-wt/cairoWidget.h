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

#ifndef CAIROWIDGET_H_
#define CAIROWIDGET_H_

#include <cairo_base.h>
#undef None
#include "classdesc_access.h"
#include <Wt/WPaintedWidget>
#include <Wt/WPainter>
#include <boost/filesystem.hpp>

namespace minsky { namespace gui {

class CairoWidget: public Wt::WPaintedWidget
{
private:
  /**
   * The Raster buffer class
   */
  typedef ecolab::cairo::Surface RasterBuffer;

  /**
   * Shared ptr is used to manage raster buffer scope.
   */
  typedef classdesc::shared_ptr<RasterBuffer> RasterBufferPtr;

  /**
   * Shared ptr is used to manage Wt image uri scope.
   */
  typedef classdesc::shared_ptr<Wt::WPainter::Image> ImagePtr;

  enum
  {
    defaultWidth = 200,
    defaultHeight = 200,
  };

private:
  RasterBufferPtr buffer;     ///<  The raster buffer on which drawing is done.
  Wt::WRectArea* entireArea;  ///<  A rectangle that spans the entire widget,
                              ///   used for setting the mouse cursor.
                              ///   Pointer is owned by the Wt framework, and
                              ///   unregisters automatically on delete.
  Wt::WRectF viewport;        ///<  The viewport rectangle in source image
                              ///   coordinates
  ImagePtr image;             ///<  Image object used for transfer to the
                              ///   client.
  int clientWidth;            ///<  Display window width in pixels
  int clientHeight;           ///<  Display window height in pixels
  boost::filesystem::path tmpFileName;  ///< temporary filename to transfer files
                                        ///  to Wt's painter.

public:
  /**
   * Constructor
   */
  CairoWidget(Wt::WContainerWidget* parent = 0);

  /**
   * Destructor
   */
  virtual ~CairoWidget();

  /**
   * Changes the mouse cursor
   */
  void setCursor(Wt::Cursor cursor);

  inline unsigned int bitmapWidth() const
  {
    return image->width();
  }

  inline unsigned int bitmapHeight() const
  {
    return image->height();
  }

  /**
   * Returns the viewport, in plot coordinates.
   */
  void getViewport(double& x, double& y, double& width, double& height)
  {
    x = viewport.x();
    y = viewport.y();
    width = viewport.width();
    height = viewport.height();
  }

  /**
   * sets the viewport offset and size.
   */
  void setViewport(double x, double y, double width, double height)
  {
    viewport.setX(x);
    viewport.setY(y);
    viewport.setWidth(width);
    viewport.setHeight(height);
    update();
  }

  /**
   * Called by Wt framework when the browser window size has changed.
   */
  virtual void layoutSizeChanged(int width, int height);

protected:
  /**
   * Called by Wt framework to refresh the display.
   */
  void paintEvent(Wt::WPaintDevice* paintDevice);
};

}}  // namespace minsky::gui

#endif /* CAIROWIDGET_H_ */
