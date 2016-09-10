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

#include "operationPropertiesDlg.h"
#include <ecolab_epilogue.h>
#include <Wt/WString>

namespace minsky { namespace gui {

OperationPropertiesDlg::OperationPropertiesDlg(OperationPtr op)
    : operation(op)
{
  setWindowTitle(Wt::WString::tr("dialog.operation.properties"));
  rotation = operation->rotation;
}

OperationPropertiesDlg::~OperationPropertiesDlg()
{ }

void OperationPropertiesDlg::save()
{
  operation->rotation = rotation;
}

void OperationPropertiesDlg::initialize()
{
  addProperty(Wt::WString::tr("dialog.rotation"), rotation, -359., 359.);
//  addProperty(WString::tr("dialog.fipped"), flipped);

  _base_class::initialize();
}

}}  // namespace minsky::gui
