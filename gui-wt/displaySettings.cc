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

#include "displaySettings.h"

namespace minsky { namespace gui {

const Wt::WColor DisplaySettings::canvasViewBackColor   = Wt::WColor(255, 255, 255);

const Wt::WColor DisplaySettings::bevelInNorthColor  = Wt::WColor(32, 32, 32);
const Wt::WColor DisplaySettings::bevelInSouthColor  = Wt::WColor(255, 255, 255);
const Wt::WColor DisplaySettings::bevelInEastColor   = Wt::WColor(240, 240, 240);
const Wt::WColor DisplaySettings::bevelInWestColor   = Wt::WColor(48, 48, 48);
const Wt::WColor& DisplaySettings::bevelOutNorthColor= DisplaySettings::bevelInSouthColor;
const Wt::WColor& DisplaySettings::bevelOutSouthColor= DisplaySettings::bevelInNorthColor;
const Wt::WColor& DisplaySettings::bevelOutEastColor = DisplaySettings::bevelInWestColor;
const Wt::WColor& DisplaySettings::bevelOutWestColor = DisplaySettings::bevelInEastColor;

const Wt::WColor                DisplaySettings::sliderTrackColor      = Wt::WColor(240, 240, 255);
const Wt::WColor                DisplaySettings::sliderTickMarkColor   = Wt::WColor(0, 0, 255);
const Wt::WColor                DisplaySettings::sliderThumbColor      = Wt::WColor(250, 250, 255);
const Wt::WFont::GenericFamily  DisplaySettings::sliderLabelFontFamily = Wt::WFont::SansSerif;
const Wt::WFont::Weight         DisplaySettings::sliderLabelFontWeight = Wt::WFont::Bold;
const Wt::WLength               DisplaySettings::sliderLabelFontSize   = Wt::WLength(10);

const Wt::WColor DisplaySettings::bezierSummitColor     = Wt::WColor(255, 0, 0);
const Wt::WColor DisplaySettings::bezierPassThroughColor= Wt::WColor(0, 0, 255);
const Wt::WColor DisplaySettings::wireColor             = Wt::WColor(0, 0, 0);

const double DisplaySettings::defaultZoom         = 1;
const double DisplaySettings::minZoomFactor       = .30;
const double DisplaySettings::maxZoomFactor       = 10.;
const double DisplaySettings::zoomFactorIncrement = .25;

}}  // namespace minsky::gui
