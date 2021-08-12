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
#include "item.rcd"
#include "assetClass.h"
#include "assetClass.rcd"
#include "bookmark.h"
#include "bookmark.rcd"
#include "capiRenderer.h"
#include "capiRenderer.cd"
#include "capiRenderer.xcd"
#include "cairoSurfaceImage.h"
#include "cairoSurfaceImage.rcd"
#include "cairoSurfaceImage.xcd"
#include "callableFunction.h"
#include "callableFunction.rcd"
#include "callableFunction.xcd"
#include "constMap.h"
#include "constMap.rcd"
#include "dynamicRavelCAPI.h"
#include "dynamicRavelCAPI.rcd"
#include "dynamicRavelCAPI.xcd"
#include "dataSpecSchema.h"
#include "dataSpecSchema.rcd"
#include "godleyIcon.h"
#include "godleyIcon.rcd"
#include "godleyTable.rcd"
#include "godleyTableWindow.h"
#include "godleyTableWindow.rcd"
#include "godleyTableWindow.xcd"
#include "group.h"
#include "group.rcd"
#include "hypercube.rcd"
#include "hypercube.xcd"
#include "lock.h"
#include "lock.rcd"
#include "lock.xcd"
#include "index.rcd"
#include "index.xcd"
#include "noteBase.rcd"
#include "operation.h"
#include "operation.rcd"
#include "operationType.rcd"
#include "port.rcd"
#include "CSVDialog.rcd"
#include "CSVParser.rcd"
#include "dimension.rcd"
#include "parameterTab.h"
#include "parameterTab.rcd"
#include "plotWidget.h"
#include "plotWidget.rcd"
#include "plot.xcd"
#include "plot.rcd"
#include "polyRESTProcessBase.rcd"
#include "polyRESTProcessBase.xcd"
#include "ravelWrap.h"
#include "ravelWrap.rcd"
#include "ravelWrap.xcd"
#include "ravelState.rcd"
#include "ravelState.xcd"
#include "renderNativeWindow.rcd"
#include "renderNativeWindow.xcd"
#include "slider.rcd"
#include "selection.h"
#include "selection.rcd"
#include "sheet.h"
#include "sheet.rcd"
#include "switchIcon.h"
#include "switchIcon.rcd"
#include "SVGItem.rcd"
#include "tensorVal.rcd"
#include "tensorVal.xcd"
#include "tensorInterface.rcd"
#include "tensorInterface.xcd"
#include "userFunction.h"
#include "userFunction.rcd"
#include "variable.h"
#include "variable.rcd"
#include "variableType.rcd"
#include "variableValue.rcd"
#include "wire.rcd"
#include "units.rcd"
#include "xvector.rcd"

#include "nobble.h"
#include "engNotation.rcd"
#include "engNotation.xcd"

#include "minsky_epilogue.h"
#include "itemTemplateInstantiations.h"

namespace minsky
{
  DEFVAR(undefined)
  DEFVAR(constant)
  DEFVAR(parameter)
  DEFVAR(flow)
  DEFVAR(integral)
  DEFVAR(stock)
  DEFVAR(tempFlow)
  
  DEF(GodleyIcon,Item)
  DEF(Group,Item)
  DEF(Lock,Item)
  DEF(PlotWidget,Item)
  DEF(Sheet,Item)
  DEF(SwitchIcon,BottomRightResizerItem)
}
