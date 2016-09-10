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

#ifndef VARIABLEPROPERTIESDLG_H_
#define VARIABLEPROPERTIESDLG_H_

#include "propertiesDlg.h"
#include "minskyDoc.h"

namespace minsky { namespace gui {

class VariablePropertiesDlg: public PropertiesDlg
{
private:
  typedef PropertiesDlg _base_class;

private:
  VariablePtr var;
  Wt::WString varName;
  double      varValue    = 0;
  double      varRotation = 0;
  bool        flipped     = false;

public:
  VariablePropertiesDlg(VariablePtr var);
  virtual ~VariablePropertiesDlg();

  /**
   * Creates dialog contents
   */
  virtual void initialize();

  /**
   * Saves dialog values to the model variable
   */
  void save();
};

}}  // namespace minsky::gui

#endif /* VARIABLEPROPERTIESDLG_H_ */
