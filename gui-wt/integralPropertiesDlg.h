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

#ifndef INTEGRALPROPERTIESDLG_H_
#define INTEGRALPROPERTIESDLG_H_

#include "operationPropertiesDlg.h"
#include "GUI/operation.h"

namespace minsky { namespace gui {

class IntegralPropertiesDlg: public OperationPropertiesDlg
{
private:
  typedef OperationPropertiesDlg _base_class;

public:
  struct wrongOperationType
    : virtual boost::exception,
      virtual std::exception { };

private:
  Wt::WString varName;
  double initialValue = 0;

public:
  IntegralPropertiesDlg(OperationPtr op);
  virtual ~IntegralPropertiesDlg();

  virtual void save();

protected:
  inline IntOp* getOp()
  {
    return dynamic_cast<IntOp*>(_base_class::getOp());
  }

  /**
   * Creates dialog contents
   */
  virtual void initialize();

  /**
   * Vslidates dialog contents
   */
  virtual bool validate();
};

}}  // namespace minsky::gui

#endif /* INTEGRALPROPERTIESDLG_H_ */
