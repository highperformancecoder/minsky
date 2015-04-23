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

#ifndef OPERATIONPROPERTIESDLG_H_
#define OPERATIONPROPERTIESDLG_H_

#include "propertiesDlg.h"
#include "GUI/operation.h"

namespace minsky { namespace gui {

/**
 * Properties dialog for operations, serves as a base for editing
 * properties all misky::Operation-derived objects
 */
class OperationPropertiesDlg: public PropertiesDlg
{
private:
  typedef PropertiesDlg _base_class;

private:
  OperationPtr operation; ///<  Operation beign edited.
  double rotation  = 0;   ///<  Rotation on canvas
  bool  flipped = false;  ///<  true if mirrored on canvas.
                          ///   @TODO: not implemented in model
public:
  /**
   * Constructor.
   */
  OperationPropertiesDlg(OperationPtr op);

  /**
   * Destructor.
   */
  virtual ~OperationPropertiesDlg();

  /**
   * Saves edited values to model.
   */
  virtual void save();

protected:
  /**
   * Accesses, with cast in derived classes, the operation being edited.
   */
  inline OperationBase* getOp()
  {
    return operation.get();
  }

  /**
   * Creates dialog contents
   */
  virtual void initialize();
};

}}  // namespace minsky::gui

#endif /* OPERATIONPROPERTIESDLG_H_ */
