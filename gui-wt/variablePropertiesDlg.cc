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

#include "variablePropertiesDlg.h"
#include <Wt/WText>
#include <Wt/WLineEdit>
#include "minskyApp.h"
#include <ecolab_epilogue.h>

namespace minsky { namespace gui {

using namespace Wt;

class VariableNameValidator : public Validator<WString>
{
public:
  virtual bool validate(WString& value)
  {
    MinskyDoc* doc = MinskyApp::getDoc();

    // remove whitespace
    value = value.trim();

//    OperationPtr op = doc->findVar(value);
//
//    if (op.get())
//    {
//      return false;
//    }
    return true;
  }
};

VariablePropertiesDlg::VariablePropertiesDlg(VariablePtr _var)
  : var(_var)
{
  varName = WString::fromUTF8(var->name());
  varValue = var->value();
  varRotation  = var->rotation;
  //TODO: flipped??
  //flipped = var->flipped();
}

VariablePropertiesDlg::~VariablePropertiesDlg()
{
}

void VariablePropertiesDlg::initialize()
{
  setWindowTitle(WString::tr("dialog.var.title"));
  addProperty(WString::tr("dialog.name"), varName, new VariableNameValidator);
  addProperty(WString::tr("dialog.value"), varValue);
  addProperty(WString::tr("dialog.rotation"), varRotation, -359., 359.);

  _base_class::initialize();
}

void VariablePropertiesDlg::save()
{
  var->rotation = varRotation;
  var->value(varValue);
  var->name(varName.toUTF8());
}

}}  // namespace minsky::gui
