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

#include "abstractView.h"
#include "minskyApp.h"
#include <ecolab_epilogue.h>

namespace minsky { namespace gui {

AbstractView::AbstractView(MinskyDoc& doc)
   : document(doc)
{
  MinskyApp::getApp()->registerView(*this, document);
}

AbstractView::~AbstractView()
{
  //  avoid throwing exceptions in destructor.
  //  Note that if the application was destroyed, the view/model association
  //  was destroyed anyway...
  if (MinskyApp::isValid())
  {
    MinskyApp::getApp()->unregisterView(*this);
  }
}

void AbstractView::onGetFocus()
{
  MinskyApp::getApp()->setActiveView(*this);
}

}}  // namespace minsky::gui
