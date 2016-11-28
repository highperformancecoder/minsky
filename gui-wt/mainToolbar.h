/*
 * mainToolbar.h
 *
 *  Created on: 9 mai 2013
 *      Author: tika
 */

#ifndef MAINTOOLBAR_H
#define MAINTOOLBAR_H

#include "toolbarBase.h"
#include "abstractView.h"
#include <Wt/Ext/Button>
#include <Wt/WPushButton>

namespace minsky { namespace gui {

using namespace Wt;

class CanvasView;

/**
 * This toolbar holds buttons to insert operations and tables on the canvas.
 *
 * The toolbar created is unique for the application and its ID is
 * "minsky-main-toolbar".
 *
 *  To add new items:
 *  @- add a handler for the button in canvasView.
 *  @- call addOperation() from initialize()
 *  @- connect the button clicked signal to the handler in view.
 *
 *  @note There is no way to add/change style for Ext::Button here.  all
 *  customization is done in css.
 */
class MainToolbar: public ToolbarBase, public AbstractView
{
private:
  typedef ToolbarBase _base_class;  ///< Alias for Maintoolbar's base class
  typedef MainToolbar _this_class;  ///< Alias for this class.
  typedef WPushButton Button;       ///< Alias for buttons.


private:
  Button* toolDefaultButton;  ///<  weak pointer to default button.  Pointer belongs to Wt.
  Button* toolLassoButton;    ///<  weak pointer to lasso button.  Pointer belongs to Wt.
  Button* toolLinkButton;     ///<  weak pointer to link button.  Pointer belongs to Wt.
  Button* toolDeleteButton;   ///<  weak pointer to delete button.  Pointer belongs to Wt.
  Wt::WSlider* simSpeedSlider;///<  weak pointer to the simulation speed slider.  Pointer belongs to Wt.

public:
  MainToolbar(WContainerWidget* parent = NULL);  ///< Constructor.
  virtual ~MainToolbar(); ///< Destructor.

  void initialize(CanvasView& canvas);

protected:

  void checkedChangeToolDefault();
  void checkedChangeToolLasso();
  void checkedChangeToolLink();
  void checkedChangeToolDelete();

  /**
   * Adds a button to the toolbar
   * @param id a unique string id for the button.  An icon is
   *           selected from that id.
   * @returns A pointer to the newly created button.  It is onwed by the Wt
   * framework.
   */
  Button* addOperationButton(const std::string& id);

  /**
   *  Called when the document has changed and needs refresh
   *  @param The View that caused the modification, or  0 if not applicable.
   */
  virtual void docHasChanged(AbstractView* source, MinskyDoc::Hint hint, const AnyParams& data);
};

}}  //  namespace minsky::gui

#endif /* MAINTOOLBAR_H */
