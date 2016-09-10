/*
  @copyright Steve Keen 2012
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

#ifndef SLIDER_H
#define SLIDER_H
namespace minsky
{
  /// data structure supporting sliders - shared between constant
  /// operations and variables
  struct Slider
  {
    // constants have sliders
    bool sliderVisible, ///< slider is visible on canvas
      sliderBoundsSet, ///< slider bounds have been initialised at some point
      sliderStepRel;   /**< sliderStep is relative to the range
                          [sliderMin,sliderMax] */

    double sliderMin, sliderMax, sliderStep;
    Slider(): sliderVisible(false), sliderBoundsSet(false), 
              sliderStepRel(false) {}
  };
}

#include "slider.cd"
#endif
