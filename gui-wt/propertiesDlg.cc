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

#include "minskyApp.h"
#include "propertiesDlg.h"
#include <ecolab_epilogue.h>
#include <Wt/WVBoxLayout>
#include <Wt/Ext/Button>
#include <algorithm>

namespace minsky { namespace gui {

using namespace Wt;

template<>
bool LineEditPropertyControl<Wt::WString>::setValue(const Wt::WString& val)
{
  dynamic_cast<Wt::WLineEdit*>(valueWidget)->setText(val);
  setSelectable(false);
  return true;
}

template<>
bool LineEditPropertyControl<Wt::WString>::getValue(Wt::WString& val)
{
  val = dynamic_cast<Wt::WLineEdit*>(valueWidget)->text();
  return true;
}

PropertiesDlg::PropertiesDlg(WObject* _parent /* = 0 */)
{
  setSelectable(false);
  setWindowTitle(WString::tr("propertiesdlg.title"));
}

void PropertiesDlg::show()
{
  initialize();
  finished().connect(this, &PropertiesDlg::onFinished);

  _base_class::show();
}

Ext::Dialog::DialogCode PropertiesDlg::exec()
{
  initialize();
  return _base_class::exec();
}

void PropertiesDlg::initialize()
{
  WVBoxLayout* layout = new WVBoxLayout(contents());

  std::for_each(properties.begin(), properties.end(),
      [layout](PropertyPtr prop)
      {
        prop->createControl(layout);
      });

  std::unique_ptr<Ext::Button> buttonOk(new Ext::Button(WString::tr("dialog.ok")));
  std::unique_ptr<Ext::Button> buttonCancel(new Ext::Button(WString::tr("dialog.cancel")));

  buttonOk->clicked().connect(this, &PropertiesDlg::onOkStub);

  buttonCancel->clicked().connect(this, &PropertiesDlg::onCancelStub);

  Ext::Button* defaultButton = buttonOk.get();
  addButton(buttonOk.release());
  addButton(buttonCancel.release());

  setDefaultButton(defaultButton);
}

bool PropertiesDlg::validate()
{
  bool result = true;

  Properties::iterator it;
  for (it = properties.begin(); result && it != properties.end(); ++it)
  {
    result &= (*it)->validate();

    //TODO: feedback in case of error
  }

  return result;
}

void PropertiesDlg::onOk()
{
  if (validate())
  {
    std::for_each(properties.begin(), properties.end(),
        [](PropertyPtr prop)
        {
          prop->save();
        });
    accept();
  }
}

/**
 * Handler for Cancel button
 */
void PropertiesDlg::onCancel()
{
  reject();
}

/**
 * Handler for Ok button
 */
void PropertiesDlg::onOkStub()
{
  onOk();
}

/**
 * Handler for Cancel button
 */
void PropertiesDlg::onCancelStub()
{
  onCancel();
}

void PropertiesDlg::onFinished(DialogCode exitCode)
{
  // only relevant to modeless dialogs.
  delete this;
}

}}  // namespace minsky::gui
