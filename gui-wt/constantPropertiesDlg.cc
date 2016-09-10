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

#include "constantPropertiesDlg.h"
#include "minskyApp.h"
#include <ecolab_epilogue.h>

using namespace Wt;

namespace minsky { namespace gui {

namespace {
  class NameValidator : public Validator<WString>
  {
  public:
    Constant* constant;   ///< Weak pointer to edited object

    NameValidator(Constant* _constant)
      : constant(_constant)
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

ConstantPropertiesDlg::ConstantPropertiesDlg(OperationPtr op)
    : _base_class(op)
{
  if (!getOp())
  {
    BOOST_THROW_EXCEPTION(wrongOperationType());
  }

  setWindowTitle(WString::tr("dialog.constant.properties"));

  Constant* constant = getOp();
  name = WString::fromUTF8(constant->description);
  value = constant->value;
  if (constant->sliderBoundsSet)
  {
    sliderMin = constant->sliderMin;
    sliderMax = constant->sliderMax;
    sliderStepSize = constant->sliderStep;
  }
  sliderIsRelative = constant->sliderStepRel;
  showSlider = constant->sliderVisible;
}

ConstantPropertiesDlg::~ConstantPropertiesDlg()
{
}

void ConstantPropertiesDlg::initialize()
{
  addProperty(WString::tr("dialog.name"), name, new NameValidator(getOp()));
  addProperty(WString::tr("dialog.value"), value);
  addProperty(WString::tr("dialog.slider.min"), sliderMin);
  addProperty(WString::tr("dialog.slider.max"), sliderMax);
  addProperty(WString::tr("dialog.slider.step"), sliderStepSize);
  addProperty(WString::tr("dialog.slider.relative"), sliderIsRelative);
  addProperty(WString::tr("dialog.slider.show"), showSlider);

  _base_class::initialize();
}

bool ConstantPropertiesDlg::validate()
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

void ConstantPropertiesDlg::save()
{
  _base_class::save();

  Constant* constant = getOp();
  constant->description = name.toUTF8();
  constant->value = value;
  constant->sliderMin = sliderMin;
  constant->sliderMax = sliderMax;
  constant->sliderStep = sliderStepSize;
  constant->sliderStepRel = sliderIsRelative;
  constant->sliderVisible = showSlider;
}

}}  // namespace minsky::gui
