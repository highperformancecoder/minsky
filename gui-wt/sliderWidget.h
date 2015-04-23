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

#ifndef SLIDERWIDGET_H_
#define SLIDERWIDGET_H_

#include <Wt/WContainerWidget>
#include <Wt/WPaintedWidget>

namespace minsky { namespace gui {

class OperationCanvasItem;
class CanvasView;
class CanvasPainter;

/**
 * The small slider companion to constant operations.
 */
class SliderWidget: public Wt::WContainerWidget
{
private:
  typedef Wt::WContainerWidget _base_class;

public:
  static const int width  = 40;
  static const int height = 20;

protected:
  /**
   * The bar the slider thumb slides on.
   */
  class SliderBar : public Wt::WPaintedWidget
  {
  private:
    typedef Wt::WPaintedWidget _base_class;

  public:
    static const int width = SliderWidget::width;
    static const int height = 5;
    static const int left = 0;
    static const int top = SliderWidget::height - height;

  public:
    SliderBar(SliderWidget* parent);  ///<  Constructor
    virtual ~SliderBar();             ///<  Destructor

    /**
     * Returns the Wt parent cast as a SliderWidget*
     */
    inline SliderWidget* parent()
    {
      return dynamic_cast<SliderWidget*>(_base_class::parent());
    }

  protected:
    /**
     * This function is called by the framework when the widget needs redraw.
     */
    virtual void paintEvent(Wt::WPaintDevice *paintDevice);
  };

  /**
   * The interactive slider thumb.
   */
  class SliderThumb : public Wt::WPaintedWidget
  {
  private:
    typedef Wt::WPaintedWidget _base_class;

  public:
    static const int width  = 21;
    static const int height = 5;
    static const int leftLimit = 0;
    static const int rightLimit = SliderBar::width - width;
    static const int left = (leftLimit + rightLimit) / 2;
    static const int top = SliderWidget::height - height;

  private:
    double stepSize =  0.1;
    double min      = -1.;
    double max      =  1.;

    double mouseX = 0;
    double savedPos = 0;
    bool   moving = false;

  public:
    SliderThumb(SliderWidget* parent);  ///<  Constructor
    virtual ~SliderThumb();             ///<  Destructor

    /**
     * Returns the Wt parent cast as a SliderWidget*
     */
    inline SliderWidget* parent()
    {
      return dynamic_cast<SliderWidget*>(_base_class::parent());
    }

    /**
     * Sets the value.
     */
    void setValue(const double& value);

    /**
     * Sets the range and step size.  Assumes values are valid.
     */
    void setRangeAndStepSize(const double& stepSize,
                             const double& min,
                             const double& max);

    /**
     * returns the current slider value.
     */
    double value() const;

  private:
    /**
     * Converts a position to value between min and max
     */
    double posToValue(const double& pos) const;

    /**
     * Converts a value to a slider position
     */
    double valueToPos(const double& value) const;

    /**
     * Sets the position of the thumb and updates model.
     */
    void setPos(const double& newPos);

  protected:
    /**
     * This function is called by the framework when the widget needs redraw.
     */
    virtual void paintEvent(Wt::WPaintDevice *paintDevice);
  };

  /**
   * Small label indicating the value of the slider widget.
   */
  class SliderLabel : public Wt::WPaintedWidget
  {
  private:
    typedef Wt::WPaintedWidget _base_class;

  public:
    static const int fontSize = 8;  ///<  Font size for rendering
    static const int precision = 2;  ///<  Number of decimals to print
    static const int width = SliderWidget::width;
    static const int height = SliderWidget::height - SliderThumb::height;

  public:
    SliderLabel(SliderWidget* parent);  ///<  Constructor
    virtual ~SliderLabel();             ///<  Destructor

    /**
     * Returns the Wt parent cast as a SliderWidget*
     */
    inline SliderWidget* parent()
    {
      return dynamic_cast<SliderWidget*>(_base_class::parent());
    }

    /**
     * returns the value to print.
     */
    double value();

  protected:
    /**
     * This function is called by the framework when the widget needs redraw.
     */
    virtual void paintEvent(Wt::WPaintDevice *paintDevice);
  };

  friend class SliderLabel;
  friend class SliderThumb;
  friend class CanvasPainter;

private:
  OperationCanvasItem& opItem;    ///< The operation this slider is attached to.
  SliderThumb* sliderThumb;       ///< The Thumb widget.
  SliderLabel* sliderLabel;       ///< The label widget.

public:
  /**
   * Constructor.
   *
   * @param item  The operation (constant) this widget is attached to.
   * @param parent  The CanvasView onto which this object is drawn.
   */
  SliderWidget(OperationCanvasItem& item, CanvasView* parent);

  /**
   * Destructor.
   */
  virtual ~SliderWidget();

  /**
   * Returns the Wt parent cast as a CanvasView*
   */
  inline CanvasView* parent();

  /**
   * Accesses the operation this slider is attached to.
   */
  inline OperationCanvasItem& getOp()
  {
    return opItem;
  }

  /**
   * Sets the range
   */
  inline void setRangeAndStepSize(const double& stepSize,
                                  const double& min,
                                  const double& max)
  {
    sliderThumb->setRangeAndStepSize(stepSize, min, max);
  }

  /**
   * Forces a redraw of childer widgets.
   */
  void update();

  /**
   * returns the current slider value.
   */
  double value();

protected:
  /**
   * Accesses the label.
   */
  SliderLabel* label()
  {
    return sliderLabel;
  }

  /**
   * Called by the slider thumb to indicate a value change.
   */
  void thumbSetValue(const double& value);
};

}}  // namespace minsky::gui

#endif /* SLIDERWIDGET_H_ */
