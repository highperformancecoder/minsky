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

#ifndef DISPLAYSETTINGS_H
#define DISPLAYSETTINGS_H

#include "guiDefs.h"
#include <Wt/WColor>
#include <Wt/WFont>
#include <Wt/WLength>
#include <vector>

namespace minsky { namespace gui {
/**
 * Utility class defining display constants.
 */
class DisplaySettings
{
public:
  static const Wt::WColor canvasViewBackColor;
  static const Wt::WColor bezierSummitColor;
  static const Wt::WColor bezierPassThroughColor;
  static const Wt::WColor wireColor;
  static const Wt::WColor bevelInNorthColor;
  static const Wt::WColor bevelInSouthColor;
  static const Wt::WColor bevelInEastColor;
  static const Wt::WColor bevelInWestColor;
  static const Wt::WColor& bevelOutNorthColor;
  static const Wt::WColor& bevelOutSouthColor;
  static const Wt::WColor& bevelOutEastColor;
  static const Wt::WColor& bevelOutWestColor;

  static const Wt::WColor                sliderTrackColor;
  static const Wt::WColor                sliderTickMarkColor;
  static const Wt::WColor                sliderThumbColor;
  static const Wt::WFont::GenericFamily  sliderLabelFontFamily;
  static const Wt::WFont::Weight         sliderLabelFontWeight;
  static const Wt::WLength               sliderLabelFontSize;

  static const double defaultZoom;          ///<  default zoom.
  static const double minZoomFactor;        ///<  minimum zoom factor for canvas
  static const double maxZoomFactor;        ///<  maximum zoom factor for canvas
  static const double zoomFactorIncrement;  ///<  increment to the zoom factor via the zoom buttons

  /**
   * Z index for canvas widgets, all values are negative because
   * of the rather poor handling of z order by the Wt library.
   */
  enum zIndex
  {
    widgetZIndex = -140,              ///<  z-index for widgets
    wireZIndex = -145,                //<  z-index for wires
    wireEventAreaZIndex = wireZIndex + 1, ///<  z-index for Wire event area.
    dialogZIndex = -50,              ///<  z-index for dialogs
    bezierSummitZIndex = -100,        ///<  z-index for bezier summit handles.
    bezierPassThroughZIndex = -110,   ///<  z-index for bezier passthrough point handles.
    sliderZIndex = -135,              ///<  z-index for slider widgets
    selectionOverlayZIndex = 0,   ///<  z-indedx for the selection overlay
                                  ///   @see RectSelectionOverlay.
    wireCreateZIndex = 0,         ///<  Wire creation overlay z-index
                                  ///   @see WireCreateOverlay
    deleteItemsOverlayZIndex = 0, ///<  Items delete overlay. @see DeleteItemsOverlay.
    disableItemsOverlay = 0,      ///<  Items disable overlay z index. @see DisablingOverlay.
  };
};

}}  // namespace minsky::gui

#endif // DISPLAYSETTINGS_H
