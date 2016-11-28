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

#include "integralPropertiesDlg.h"
#include "minskyApp.h"
#include <ecolab_epilogue.h>

namespace minsky { namespace gui {

using namespace Wt;

namespace {
  class NameValidator : public Validator<WString>
  {
  public:
    minsky::IntOp* integrate;   ///< Weak pointer to edited object

    NameValidator(IntOp* _integrate)
      : integrate(_integrate)
    { }

    virtual bool validate(WString& value)
    {
      MinskyDoc* doc = MinskyApp::getDoc();

      // remove whitespace
      value = value.trim();

      if (value.empty())
      {
        return false;
      }

  //    OperationPtr op = doc->findVar(value);
  //
  //    if (op.get())
  //    {
  //      return false;
  //    }
      return true;
    }
  };
}

IntegralPropertiesDlg::IntegralPropertiesDlg(OperationPtr op)
  : _base_class(op)
{
  if (!getOp())
  {
    BOOST_THROW_EXCEPTION(wrongOperationType());
  }

  setWindowTitle(WString::tr("dialog.integrate.properties"));

  IntOp* integrate = getOp();
  varName = WString::fromUTF8(integrate->description());
  //??initialValue = integrate->value;
}

IntegralPropertiesDlg::~IntegralPropertiesDlg()
{ }

void IntegralPropertiesDlg::initialize()
{

  addProperty(WString::tr("dialog.name"), varName, new NameValidator(getOp()));
  addProperty(WString::tr("dialog.value"), initialValue);
//  addProperty(WString::tr("dialog.slider.min"), sliderMin);
//  addProperty(WString::tr("dialog.slider.max"), sliderMax);
//  addProperty(WString::tr("dialog.slider.step"), sliderStepSize);
//  addProperty(WString::tr("dialog.slider.relative"), sliderIsRelative);
//  addProperty(WString::tr("dialog.slider.show"), showSlider);

  _base_class::initialize();
}

bool IntegralPropertiesDlg::validate()
{
//  if (_base_class::validate())
//  {
//    if (getPropertyValue(sliderMin) >= getPropertyValue(sliderMax))
//    {
//      return false;
//    }
//    return true;
//  }
//  return false;
  return _base_class::validate();
}

void IntegralPropertiesDlg::save()
{
  _base_class::save();

  IntOp* integrate = getOp();
  integrate->description(varName.toUTF8());
  //TODO: ?? integrate->setInitialvalue(initialValue);
}

}}  // namespace minsky::gui
