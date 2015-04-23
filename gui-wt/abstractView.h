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

#ifndef ABSTRACTVIEW_H_
#define ABSTRACTVIEW_H_

#include <Wt/WContainerWidget>
#include "minskyDoc.h"
#include "anyParams.h"

namespace minsky { namespace gui {

using namespace Wt;

class MinskyDoc;

/**
 * 	Interface for Minsky document views.
 */
class AbstractView
{
/*
 *	Place holders for actual types, need to be plugged to actual Minsky data.
 */
	typedef void* Selection;
/**
 *  The source document for this view.
 */
	MinskyDoc& document;

protected:
	/**
	 * Constructor.
	 */
	AbstractView(MinskyDoc& doc);

public:
	/**
	 * Destructor.
	 */
	virtual ~AbstractView();

	//
	//  Interface
	//

public:
	/**
	 *  Called when the document has changed and needs refresh
	 *  @param The View that caused the modification, or  0 if not applicable.
	 */
	virtual void docHasChanged(AbstractView* source, MinskyDoc::Hint hint, const AnyParams& data) = 0;

	/**
	 * Handler called when the view has gained user input focus.
	 */
	void onGetFocus();

  //
	//	Helpers
	//
public:
	/**
	 *	Accesses the document this view is attached to.
	 *	@returns A pointer to the application's active document.
	 */
	inline MinskyDoc& getDoc()
	{
		return document;
	}
};

}}  // namespace minsky::gui

#endif /* ABSTRACTVIEW_H_ */
