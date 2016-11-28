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

#include "viewManager.h"
#include "abstractView.h"
#include <ecolab_epilogue.h>

namespace minsky { namespace gui {

void ViewManager::registerView(AbstractView& view, MinskyDoc& doc)
{
  //  make sure the view is not already registered.
  ViewsMap::iterator it = viewsMap.find(&view);
  if (it == viewsMap.end())
  {
    viewsMap[&view] = &doc;
  }
  else
  {
    /**
     * for debugging, really.
     */
    BOOST_THROW_EXCEPTION(alreadyRegisteredViewException());
  }
}

bool ViewManager::unregisterView(AbstractView& view)
{
  //  make sure the view is not already registered.
  ViewsMap::iterator it = viewsMap.find(&view);
  if (it != viewsMap.end())
  {
    if (activeView == &view)
    {
      activeView = NULL;
    }
    viewsMap.erase(it);
    return true;
  }
  return false;
}

void ViewManager::getViews(const MinskyDoc& doc, ViewsVector& viewsVector)
{
  viewsVector.clear();

  for(ViewsMap::iterator it = viewsMap.begin(); it != viewsMap.end(); ++it)
  {
    if (it->second == &doc)
    {
      viewsVector.push_back(it->first);
    }
  }
}

MinskyDoc& ViewManager::getDoc(AbstractView& view)
{
  ViewsMap::iterator it = viewsMap.find(&view);
  if (it == viewsMap.end())
  {
    BOOST_THROW_EXCEPTION(unknownViewException());
  }
  return *(it->second);
}

void ViewManager::setActiveView(AbstractView& view)
{
  ViewsMap::iterator it = viewsMap.find(&view);
  if (it == viewsMap.end())
  {
    BOOST_THROW_EXCEPTION(unknownViewException());
  }
  activeView = &view;
}


void ViewManager::docHasChanged(MinskyDoc& doc, AbstractView* view,
                              MinskyDoc::Hint hint, const AnyParams& data)
{
  // Since some views may be destroyed while enumerating, using a copy of the list  
  // avoids invalid iterators problems.
  ViewsVector views;
  getViews(doc, views);

  for (ViewsVector::iterator it = views.begin(); it != views.end(); ++it)
  {
    if (viewsMap.count(*it))
    {
      (*it)->docHasChanged(view, hint, data);
    }
  }
}

}}  // namespace minsky::gui
