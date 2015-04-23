/*
 * mainToolbar.cc
 *
 *  Created on: 9 mai 2013
 *      Author: tika
 */

#include <Wt/WSlider>
#include "mainToolbar.h"
//#include "operationType.h"
#include "canvasView.h"
#include "minskyApp.h"
#include <ecolab_epilogue.h>

namespace minsky { namespace gui {

using namespace Wt;

MainToolbar::MainToolbar(WContainerWidget* parent)
  : _base_class(parent),
    AbstractView(*MinskyApp::getDoc())
{
  setId("minsky-main-toolbar");
  setSelectable(false);
}

MainToolbar::~MainToolbar()
{
}

void MainToolbar::initialize(CanvasView& canvas)
{
  addOperationButton("toolbar.tool.default")
    ->clicked().connect(boost::bind(&CanvasView::setActivity, &canvas, CanvasView::actDefault));
  addOperationButton("toolbar.tool.link")
    ->clicked().connect(boost::bind(&CanvasView::setActivity, &canvas, CanvasView::actAddWire));
  addOperationButton("toolbar.tool.lasso")
    ->clicked().connect(boost::bind(&CanvasView::setActivity, &canvas, CanvasView::actSelectGroup));
  addOperationButton("toolbar.tool.delete")
    ->clicked().connect(boost::bind(&CanvasView::setActivity, &canvas, CanvasView::actDelete));
  addSeparator();
  addOperationButton("toolbar.zoom.in")
    ->clicked().connect(&canvas, &CanvasView::clickedZoomIn);
  addOperationButton("toolbar.zoom.out")
    ->clicked().connect(&canvas, &CanvasView::clickedZoomOut);
  addOperationButton("toolbar.zoom.reset")
    ->clicked().connect(&canvas, &CanvasView::clickedZoomReset);
  addSeparator();
  addOperationButton("toolbar.operation.constant")
    ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::constant));
  addOperationButton("toolbar.operation.variable")
    ->clicked().connect(&canvas, &CanvasView::addVariable);
  addOperationButton("toolbar.operation.add")
    ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::add));
  addOperationButton("toolbar.operation.subtract")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::subtract));
   addOperationButton("toolbar.operation.multiply")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::multiply));
   addOperationButton("toolbar.operation.divide")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::divide));
  addOperationButton("toolbar.operation.log")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::log));
  addOperationButton("toolbar.operation.pow")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::pow));
  addOperationButton("toolbar.operation.time")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::time));
  addOperationButton("toolbar.operation.integrate")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::integrate));
  addOperationButton("toolbar.operation.sqrt")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::sqrt));
  addOperationButton("toolbar.operation.exp")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::exp));
  addOperationButton("toolbar.operation.ln")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::ln));
  addOperationButton("toolbar.operation.sin")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::sin));
  addOperationButton("toolbar.operation.cos")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::cos));
  addOperationButton("toolbar.operation.tan")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::tan));
  addOperationButton("toolbar.operation.asin")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::asin));
  addOperationButton("toolbar.operation.acos")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::acos));
  addOperationButton("toolbar.operation.atan")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::atan));
  addOperationButton("toolbar.operation.sinh")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::sinh));
  addOperationButton("toolbar.operation.cosh")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::cosh));
  addOperationButton("toolbar.operation.tanh")
   ->clicked().connect(boost::bind(&CanvasView::addOperation, &canvas, minsky::OperationType::tanh));
  addSeparator();
  addOperationButton("toolbar.table.godley")
    ->clicked().connect(&canvas, &CanvasView::addGodleyTable);
  addOperationButton("toolbar.plot")
    ->clicked().connect(&canvas, &CanvasView::addPlotWidget);
  addSeparator();
  addOperationButton("toolbar.simulation.stop")
    ->clicked().connect(
        [](const WMouseEvent&)
        {
          MinskyApp::getDoc()->simulationStop();
        });

  addOperationButton("toolbar.simulation.step")
    ->clicked().connect(
        [](const Wt::WMouseEvent&)
        {
          MinskyApp::getDoc()->simulationStep();
        });

  addOperationButton("toolbar.simulation.run")
    ->clicked().connect(
        [this](const WMouseEvent&)
        {
          std::string iconID;
          if (!MinskyApp::getDoc()->simulationRunning())
          {
            if (simSpeedSlider)
            {
              int speed = simSpeedSlider->value();
              MinskyApp::getDoc()->simulationStart(speed);
            }
            iconID = "toolbar.simulation.pause";
          }
          else
          {
            MinskyApp::getDoc()->simulationPause();
            iconID = "toolbar.simulation.run";
          }

          // toggle icon on button
          Button* button = dynamic_cast<Button*>(sender());
          if (button)
          {
            std::string iconURL;
            if (MinskyApp::getApp()->getIconName(iconID, iconURL))
            {
              button->setIcon(iconURL);
            }
          }
        });

  simSpeedSlider = new WSlider;
  add(simSpeedSlider);
  simSpeedSlider->setWidth(WLength(64, WLength::Pixel));
  simSpeedSlider->setHeight(WLength(32, WLength::Pixel));
  simSpeedSlider->setObjectName("toolbar.simulation.speed");
  simSpeedSlider->setToolTip(WString::tr("toolbar.simulation.speed"), PlainText);
  simSpeedSlider->setMinimum(1);
  simSpeedSlider->setMaximum(100);
  simSpeedSlider->setValue(50);
  simSpeedSlider->valueChanged().connect(
      [](int value, NoClass, NoClass, NoClass, NoClass, NoClass)
      {
        if (MinskyApp::getDoc()->simulationRunning())
        {
          MinskyApp::getDoc()->setSimulationSpeed(value);
        }
      });
}

MainToolbar::Button* MainToolbar::addOperationButton(const string& id)
{
  std::string iconURL;
  Button* button = new Button();
  add(button);

  button->setObjectName(id);
  button->setToolTip(WString::tr(id));
  button->resize(32, 32);
  button->addStyleClass("minsky-tb32");

  if (!MinskyApp::getApp()->getIconName(id, iconURL))
  {
    MinskyApp::getApp()->log("Warning") << "Main toolbar button \'"
        << id << "\' has no corresponding icon.";
  }
  else
  {
    button->setIcon(iconURL);
  }
  return button;
}

void MainToolbar::docHasChanged(AbstractView* source, MinskyDoc::Hint hint, const AnyParams& data)
{
  switch(hint)
  {
  case MinskyDoc::hintSimulationError:
  case MinskyDoc::hintDocumentClose:
  case MinskyDoc::hintSimulationHalted:
  case MinskyDoc::hintSimulationReset:
    {
      // reset "play" icon when simulation stops
      WPushButton* button = dynamic_cast<WPushButton*>(find("toolbar.simulation.run"));
      if (button)
      {
        std::string iconURL;
        if (MinskyApp::getApp()->getIconName("toolbar.simulation.run", iconURL))
        {
          button->setIcon(iconURL);
        }
      }
    }
    break;
  default: break;
  }
}

}}  //  namespace minsky::gui
