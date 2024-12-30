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
  constexpr float sliderHandleRadius=3;
  
  /// data structure supporting sliders - shared between constant
  /// operations and variables
  struct Slider
  {
    mutable bool sliderStepRel=false;   /**< sliderStep is relative to the range
                          [sliderMin,sliderMax] */

    mutable double sliderMin=std::numeric_limits<double>::max(),
      sliderMax=-sliderMin, sliderStep=0;
    bool enableSlider=true;

    /// ensure there are at most 10000 steps between sliderMin and Max. see ticket 1255. 	
    double maxSliderSteps() const {
      if (!isfinite(sliderStep) || sliderMax-sliderMin > 1.0e04*sliderStep) return (sliderMax-sliderMin)/1.0e04;    
      return sliderStep;
    }
  };
}

#include "slider.cd"
#include "slider.xcd"
#endif
