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

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <map>
#include <vector>
#include <boost/exception/all.hpp>
#include "minskyDoc.h"
#include "anyParams.h"

namespace minsky { namespace gui {

class AbstractView;

/**
 *  View manager.
 *
 *  Maintains a list of association between documents and views.
 *  Does not own any pointers.
 */
class ViewManager
{
  /**
   * @group ViewManager_esxceptions
   *
   *  Exceptions thrown by ViewManager.
   */
  /** @{ */
public:
  /**
   * This exception is thrown when an attempt is made to register an already
   * registered view.
   */
  struct alreadyRegisteredViewException
      : virtual boost::exception,
        virtual std::exception  { };
  /**
   * This exception is thrown when an attempt is made to access information
   * for an unregistered view.
   */
  struct unknownViewException
      : virtual boost::exception,
        virtual std::exception  { };
  /** @} */
    /**
   * @group ViewManager_data
   *
   *  Data types and members.
   */
  /** @{ */
public:
  /**
   *  The links are implemented using a map of weak pointers. The view pointers
   *  in this table are owned by the Wt framework.  The documents are owned by
   *  MinksyApp.
   */
  typedef std::map<AbstractView*, MinskyDoc*> ViewsMap;
  /**
   * A views list.  Used for retrieving views associated with a specific
   * document.
   */
  typedef std::vector<AbstractView*> ViewsVector;

public:
  ViewsMap viewsMap;          ///< The views/document associations.

private:
/**
   *  The view that has focus.  The views pointers are owned by the Wt
   *  framework
   */
  AbstractView*  activeView;
  /** @} */
  /**
   * @group ViewManager_construction
   *
   *        Construction.
   */
  /** @{ */
public:
  /**
   * Constructor.
   */
  inline ViewManager()
    : activeView(NULL)
  { }
  /**
   * Destructor.
   */
  inline ~ViewManager()
  { }
  /** @} */
  /**
   * @group ViewManager_management
   */
  /** @{ */
  /**
   * Registers a view.  The view must be attached to a document.  It is the
   * responsibility of the view to call unregisterView() in its destructor.
   * @param view the View to register.
   * @param doc  the document it is attached to.
   * @see AbstractView::AbstractView()
   */
  void registerView(AbstractView& view, MinskyDoc& doc);
  /**
   * Unregisters a view.  The link View/document is destroyed, this method is
   * called from AbstractView::~AbstractView.
   * @param view the View to unregister.
   * @returns false if the view could not be found in the association map.
   * @throws alreadyRegisteredViewException if the view is already registered.
   */
  bool unregisterView(AbstractView& view);
  /**
   * Gets a list of views associated with a specific document.
   * @param doc the document of interest.
   * @param on return, contains a list of views into this document.
   */
  void getViews(const MinskyDoc& doc, ViewsVector& viewsVector);
  /**
   * Retrieves the document associated with a view.
   * @param view the View of interest.
   * @returns a reference to the document of interest.
   * @throws unknownViewException if the view is not registered.
   */
  MinskyDoc& getDoc(AbstractView& view);
  /**
   *  Returns active view.  The active view is the view that has user input
   *  focus.
   *  @returns A pointer to the active view. Can be NULL if user input is not
   *           on one of the managed views.
   */
  inline AbstractView* getActiveView()
  {
    return activeView;
  }
  /**
   *  Sets the active view.  The active view is the view that has user input
   *  focus.
   *  @param view a reference to the view that just gained input focus.
   *  @throws unkownViewException if the view is not registered with this
   *          manager.
   */
  void setActiveView(AbstractView& view);
  /**
   *  Releases focus. This method is called by the active view when it has lost
   *  user focus.
   */
  inline void resetActiveView()
  {
    activeView = NULL;
  }

  /**
   *  Called by the document to indicate changes.
   *  @param doc The affected document.
   *  @param source The view that modified the document, or NULL if not applicable.
   *  @param hint The operation that took place
   *  @param data A variable list of parameters, content depends on @p hint.
   *  @see AnyParam, MinskyDoc::Hint
   */
  void docHasChanged(MinskyDoc& doc, AbstractView* source,
                     MinskyDoc::Hint hint, const AnyParams& data);
  /** @} */
};

}}  // namespace minsky::gui

#endif /* VIEWMANAGER_H */
