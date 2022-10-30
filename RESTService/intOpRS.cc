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
#include "bookmark.h"
#include "bookmark.rcd"
#include "cairoSurfaceImage.h"
#include "cairoSurfaceImage.rcd"
#include "cairoSurfaceImage.xcd"
#include "callableFunction.h"
#include "callableFunction.rcd"
#include "callableFunction.xcd"
#include "CSVDialog.h"
#include "CSVDialog.rcd"
#include "CSVParser.rcd"
#include "dataSpecSchema.h"
#include "dataSpecSchema.rcd"
#include "dimension.rcd"
#include "eventInterface.rcd"
#include "eventInterface.xcd"
#include "group.h"
#include "engNotation.h"
#include "engNotation.rcd"
#include "engNotation.xcd"
#include "hypercube.h"
#include "hypercube.rcd"
#include "hypercube.xcd"
#include "index.h"
#include "index.rcd"
#include "index.xcd"
#include "intOp.h"
#include "intOp.rcd"
#include "plotWidget.h"
#include "plotWidget.rcd"
#include "renderNativeWindow.h"
#include "renderNativeWindow.rcd"
#include "renderNativeWindow.xcd"
#include "slider.rcd"
#include "SVGItem.rcd"
#include "tensorInterface.h"
#include "tensorInterface.rcd"
#include "tensorInterface.xcd"
#include "tensorVal.h"
#include "tensorVal.rcd"
#include "tensorVal.xcd"
#include "variable.h"
#include "variable.rcd"
#include "variableType.rcd"
#include "variableValue.h"
#include "variableValue.rcd"
#include "xvector.rcd"
#include "minsky_epilogue.h"

namespace minsky
{
  DEF(IntOp, Operation<OperationType::integrate>)
}
