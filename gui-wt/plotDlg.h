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

#ifndef PLOTDLG_H_
#define PLOTDLG_H_

#include "abstractView.h"
#include "tempCairoImageFile.h"
#include <Wt/WDialog>
#include <Wt/Ext/Dialog>
#include <Wt/WPaintedWidget>
#include <string>

namespace minsky { namespace gui {

typedef std::shared_ptr<TempCairoImageFile> TempCairoImageFilePtr;

class LargePlotWidget : public Wt::WPaintedWidget
{
private:
  typedef Wt::WPaintedWidget _base_class;


private:
  int plotId;       ///< Model plot ID.
  std::string surfaceName;  ///< Name of the cairo back buffer associated with this view.
  MinskyDoc&  doc;          ///< The minsky model.
  TempCairoImageFilePtr image;  ///< The image rendered by the minsky model.

public:
  LargePlotWidget(int plotId, MinskyDoc& doc, Wt::WContainerWidget* parent = NULL);
  virtual ~LargePlotWidget();

  /**
   * Accesses the plot ID
   * @return A reference to the plot string ID.
   */
  inline int getPlotId() const
  {
    return plotId;
  }

  /**
   * Accesses the backbuffer ID
   * @return A reference to the backbuffer string ID.
   */
  inline const std::string& getSurfaceName() const
  {
    return surfaceName;
  }

  /**
   * Sets the image resource file with the plot
   * @param img
   */
  inline void setImage(TempCairoImageFilePtr img)
  {
    image = img;
  }
  /**
   * Accesses the image representing the plot.
   * @return
   */
  inline TempCairoImageFilePtr getImage() const
  {
    return image;
  }

  /**
   * Resets the plot by destroying its backbuffer
   */
  void reset();

protected:
  /**
   * Called by Wt framework when the controlling layout changes size.
   * @param newWidth  new width assigned by the layout in pixels.
   * @param newHeight new height assigned by the layout in pixels.
   */
  virtual void layoutSizeChanged(int newWidth,int newHeight);

  /**
   * Draws the widget.
   * @param paintDevice device to draw with.
   */
  virtual void paintEvent(WPaintDevice *paintDevice);
};

class PlotDlg: public AbstractView, public Wt::WDialog
{
private:
  int plotId;             ///< Plot ID.
  LargePlotWidget* plotWidget;    ///< Weak pointer to the plot widget,
                                  ///  pointer belongs to Wt

public:
  /**
   * Constructor
   */
  PlotDlg(int plotId, MinskyDoc& doc);

  /**
   * Destructor
   */
  virtual ~PlotDlg();

protected:

  /**
   * Called by the framework when dialog closes.
   */
  virtual void done(Wt::WDialog::DialogCode r);

  /**
   *  Called when the document has changed and needs refresh
   *  @param The View that caused the modification, or  0 if not applicable.
   */
  virtual void docHasChanged(AbstractView* source, MinskyDoc::Hint hint, const AnyParams& data);
  /**
   * Called by Wt framework when the controlling layout changes size.
   * @param newWidth  new width assigned by the layout in pixels.
   * @param newHeight new height assigned by the layout in pixels.
   */
  virtual void layoutSizeChanged(int newWidth,int newHeight);
};

}}  // namespace minsky::gui

#endif /* PLOTDLG_H_ */
