/*
  @copyright Steve Keen 2019
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

#include "itemRS.h"
#include "assetClass.h"
#include "assetClass.rcd"
#include "bookmark.h"
#include "bookmark.rcd"
#include "cairoSurfaceImage.h"
#include "cairoSurfaceImage.rcd"
#include "cairoSurfaceImage.xcd"
#include "callableFunction.h"
#include "callableFunction.rcd"
#include "callableFunction.xcd"
#include "capiRenderer.h"
#include "capiRenderer.cd"
#include "capiRenderer.xcd"
#include "CSVDialog.h"
#include "CSVDialog.rcd"
#include "CSVParser.rcd"
#include "dataSpecSchema.rcd"
#include "dimension.rcd"
#include "dynamicRavelCAPI.h"
#include "dynamicRavelCAPI.rcd"
#include "dynamicRavelCAPI.xcd"
#include "engNotation.h"
#include "engNotation.rcd"
#include "engNotation.xcd"
#include "eventInterface.h"
#include "eventInterface.rcd"
#include "eventInterface.xcd"
#include "godleyIcon.h"
#include "godleyIcon.rcd"
#include "godleyTable.rcd"
#include "godleyTableWindow.h"
#include "godleyTableWindow.rcd"
#include "godleyTableWindow.xcd"
#include "handleLockInfo.h"
#include "handleLockInfo.rcd"
#include "handleLockInfo.xcd"
#include "hypercube.rcd"
#include "hypercube.xcd"
#include "index.rcd"
#include "index.xcd"
#include "lock.h"
#include "lock.rcd"
#include "lock.xcd"
#include "plotWidget.h"
#include "plotWidget.rcd"
#include "ravelState.rcd"
#include "ravelState.xcd"
#include "ravelWrap.h"
#include "ravelWrap.rcd"
#include "ravelWrap.xcd"
#include "renderNativeWindow.rcd"
#include "renderNativeWindow.xcd"
#include "selection.h"
#include "selection.rcd"
#include "sheet.h"
#include "sheet.rcd"
#include "showSlice.xcd"
#include "slider.rcd"
#include "SVGItem.rcd"
#include "switchIcon.h"
#include "switchIcon.rcd"
#include "tensorInterface.rcd"
#include "tensorInterface.xcd"
#include "tensorVal.rcd"
#include "tensorVal.xcd"
#include "variable.rcd"
#include "variableType.rcd"
#include "variableValue.rcd"
#include "xvector.rcd"
#include "minsky_epilogue.h"

namespace minsky
{
  DEF(GodleyIcon,Item)
  DEF(Group,Item)
  DEF(Lock,Item)
  DEF(PlotWidget,Item)
  DEF(Sheet,Item)
  DEF(SwitchIcon,BottomRightResizerItem)
}
