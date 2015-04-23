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

#include "globalPreferencesPropDlg.h"
#include <Wt/WComboBox>

namespace minsky { namespace gui {

class GodleyOutputStylePropertyComboBox
    : public PropertyControl<GlobalPreferences::GodleyOutputStyle>
{
public:
  virtual void create(const Wt::WString& label, Wt::WLayout* parentLayout)
  {
    setMinimumSize(250, 35);
    setSelectable(false);

    Wt::WText* labelWidget  = new Wt::WText(label);


    Wt::WComboBox* comboBox = new Wt::WComboBox;

    comboBox->insertItem(enumToIndex(GlobalPreferences::godleyStyleDrCr),
                         Wt::WString::tr("dialog.prefs.godleyDrCr"));

    comboBox->insertItem(enumToIndex(GlobalPreferences::godleyStylePlusMinus),
                         Wt::WString::tr("dialog.prefs.godleyPlusMinus"));

    valueWidget = comboBox;

    labelWidget->setVerticalAlignment(Wt::AlignMiddle);
    comboBox->setVerticalAlignment(Wt::AlignMiddle);

    Wt::WHBoxLayout* layout = new Wt::WHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->addWidget(labelWidget);
    layout->addStretch(1);
    layout->addWidget(valueWidget);

    parentLayout->addWidget(this);

    comboBox->resize(80, Wt::WLength::Auto);
  }

  virtual bool setValue(const GlobalPreferences::GodleyOutputStyle& val)
  {
    Wt::WComboBox* comboBox = dynamic_cast<Wt::WComboBox*>(valueWidget);

    if (comboBox)
    {
      comboBox->setCurrentIndex(enumToIndex(val));
      return true;
    }
    return false;
  }

  virtual bool getValue(GlobalPreferences::GodleyOutputStyle& val)
  {
    Wt::WComboBox* comboBox = dynamic_cast<Wt::WComboBox*>(valueWidget);

    if (comboBox)
    {
      val = indexToEnum(comboBox->currentIndex());
      return true;
    }
    return false;
  }

private:
  static int enumToIndex(GlobalPreferences::GodleyOutputStyle val)
  {
    switch(val)
    {
    default:
    case GlobalPreferences::godleyStylePlusMinus:
      return 1;

    case GlobalPreferences::godleyStyleDrCr:
      return 0;
    }
  }

  static GlobalPreferences::GodleyOutputStyle indexToEnum(int index)
  {
    switch(index)
    {
    default:
    case 1:
      return GlobalPreferences::godleyStylePlusMinus;

    case 0:
      return GlobalPreferences::godleyStyleDrCr;
    }
  }
};

GlobalPreferencesPropDlg::GlobalPreferencesPropDlg(GlobalPreferences& _prefs, WObject* _parent)
  : _base_class(_parent),
    prefs(_prefs)
{ }

GlobalPreferencesPropDlg::~GlobalPreferencesPropDlg()
{ }

void GlobalPreferencesPropDlg::initialize()
{
  setTitle(Wt::WString::tr("dialog.prefs.title"));

  addProperty(Wt::WString::tr("dialog.prefs.godleyTableDoubleEntry"), prefs.godleyTableDoubleEntry);
  addProperty(Wt::WString::tr("dialog.prefs.godleyTableShowValues"), prefs.godleyTableShowValues);
  addProperty(Wt::WString::tr("dialog.prefs.godleyTableOutputStyle"), prefs.godleyTableOutputStyle, new GodleyOutputStylePropertyComboBox);

  _base_class::initialize();
}

}}  // namespace minsky::gui
