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

#include "minskyApp.h"
#include "minskyDoc.h"
#include <sstream>
#include <boost/filesystem.hpp>
#include "GUI/minsky.h"
#include <ecolab_epilogue.h>

#include <Wt/WRectF>
#include <Wt/WTimer>

namespace minsky { namespace gui {

//
//  Default file name and extension.  Could live in text.xml, but if that file
//  does not exist, or the tags are not defined, Wt includes '??' in the its
//  result string, which is not a valid character for file name.
//
#define MINSKY_DEF_NAME   "NewDocument{1}"
#define MINSKY_EXT        ".msk"

using namespace Wt;

MinskyDoc::MinskyDoc()
  : autoFileName(false)
{
}

MinskyDoc::~MinskyDoc()
{
  close();
}

void MinskyDoc::generateFileName(std::string& name)
{
  unsigned int fileId = 0;
  string str;
  do
  {
    std::stringstream sstr;
    sstr << WString::tr(MINSKY_DEF_NAME)
         << fileId++
         << WString::tr(MINSKY_EXT);
    name = sstr.str();
  }
  while (boost::filesystem::exists(name));
}

void MinskyDoc::docHasChanged(AbstractView* caller, Hint hint, const AnyParams& data)
{
  setModified();
  MinskyApp::getApp()->docHasChanged(*this, caller, hint, data);
}

void MinskyDoc::loadFromFile(const std::string& name)
{
  if (!name.empty())
  {
    try
    {
      getModel().load(name.c_str());
      fileName = name;
      autoFileName = false;
      docHasChanged(NULL, hintDocumentLoad, AnyParams());
    }
    catch(...)
    {
      getModel().reset();
      MinskyApp::getApp()->MessageBox(WString::tr("messagebox.fileformaterror"), Ok);
    }
  }
}

void MinskyDoc::saveToFile(const std::string& name)
{
  if (name.empty())
  {
    //  should never reach.
    BOOST_THROW_EXCEPTION(emptyFileNameException());
  }

  getModel().save(name.c_str());

  fileName = name;
  autoFileName = false;
  docHasChanged(NULL, hintDocumentSaved, AnyParams());
}

void MinskyDoc::close()
{
  docHasChanged(NULL, hintDocumentClose, AnyParams());
  getModel().clearAllMaps();
  getModel().resetEdited();
  fileName.clear();
  autoFileName = false;
  simulationStop();
}

void MinskyDoc::getProperties(Properties& properties)
{
  properties.clear();

  properties[WString::tr("file.properties.filename").toUTF8()] = fileName;
  // ...
  //TODO: fill.
}


MinskyDoc::ObjectId MinskyDoc::addOperation(AbstractView* caller,
                                            minsky::OperationType::Type opType,
                                            const Wt::WPointF& pntInsert)
{
  ObjectId id = getModel().addOperation(minsky::OperationType::typeName(opType).c_str());
  if (id != invalidId)
  {
    getModel().operations[id]->moveTo(pntInsert.x(), pntInsert.y());
    docHasChanged(caller, hintAddOperation, AnyParams(id));
  }

  return id;
}

void MinskyDoc::removeOperation(AbstractView* caller, ObjectId id)
{
  docHasChanged(caller, hintRemoveOperation, AnyParams(id));
  getModel().deleteOperation(id);
}

void MinskyDoc::moveOperation(AbstractView* caller, ObjectId id, double x, double y)
{
  minsky::Operations::iterator it = getModel().operations.find(id);
  if (it != getModel().operations.end())
  {
    AnyParams data;
    data << id << (*it)->x() << (*it)->y() << x << y;
    (*it)->moveTo(x, y);
    setModified();
    docHasChanged(caller, hintMoveOperation, data);
  }
}

void MinskyDoc::setOperationSliderValue(AbstractView* caller, ObjectId id, double value)
{
  //TODO: Constant type has no slider value !?
}

MinskyDoc::ObjectId MinskyDoc::addWire(AbstractView* caller, ObjectId from, ObjectId to)
{
  if (getModel().ports.count(from) && getModel().ports.count(to))
  {
    ecolab::array<float> coords(4);
    coords[0] = getModel().ports[from].x();
    coords[1] = getModel().ports[from].y();
    coords[2] = getModel().ports[to].x();
    coords[3] = getModel().ports[to].y();
    ObjectId id = getModel().addWire(from, to, coords);

    if (id != invalidId)
    {
      docHasChanged(caller, hintAddWire, AnyParams(id));
    }
    return id;
  }
  return invalidId;
}

void MinskyDoc::removeWire(AbstractView* caller, ObjectId id)
{
  docHasChanged(caller, hintRemoveWire, AnyParams(id));
  getModel().deleteWire(id);
}

void MinskyDoc::addWirePoint(AbstractView* caller, ObjectId id, size_t pointIndex, float x, float y)
{
  if (getModel().wires.count(id) == 0)
  {
    return;
  }

  ecolab::array<float> coords;

  Wire& wire = getModel().wires[id];
  coords = wire.coords();

  ecolab::array<float> newCoords(coords.size() + 2);

  size_t i;
  for (i = 0; i < 2 * pointIndex; i += 2)
  {
    newCoords[i] = coords[i];
    newCoords[i + 1] = coords[i + 1];
  }

  newCoords[i] = x;
  newCoords[i + 1] = y;

  for ( ; i < coords.size(); i += 2)
  {
    newCoords[i + 2] = coords[i];
    newCoords[i + 3] = coords[i + 1];
  }
  wire.coords(newCoords);

  AnyParams params;
  params << id << coords << newCoords;
  docHasChanged(caller, hintEditWire, params);
}

void MinskyDoc::removeWirePoint(AbstractView* caller, ObjectId id, size_t pointIndex)
{
  if (getModel().wires.count(id) == 0)
  {
    return;
  }

  ecolab::array<float> coords;

  Wire& wire = getModel().wires[id];
  coords = wire.coords();

  ecolab::array<float> newCoords(coords.size() - 2);

  size_t i;
  for (i = 0; i < 2 * pointIndex; ++i)
  {
    newCoords[i] = coords[i];
  }

  for ( ; i < newCoords.size(); ++i)
  {
    newCoords[i] = coords[i + 2];
  }
  wire.coords(newCoords);

  AnyParams params;
  params << id << coords << newCoords;
  docHasChanged(caller, hintEditWire, params);
}

void MinskyDoc::setWirePoint(AbstractView* caller, ObjectId id, size_t pointIndex, float x, float y)
{
  if (getModel().wires.count(id) == 0)
  {
    return;
  }

  ecolab::array<float> coords;

  Wire& wire = getModel().wires[id];
  coords = wire.coords();

  ecolab::array<float> newCoords(coords);

  newCoords[pointIndex * 2] = x;
  newCoords[(pointIndex * 2) + 1] = y;

  wire.coords(newCoords);

  AnyParams params;
  params << id << coords << newCoords;
  docHasChanged(caller, hintEditWire, params);
}

MinskyDoc::ObjectId MinskyDoc::addVariable(AbstractView* caller,
                                           const std::string& name,
                                           const Wt::WPointF& pntInsert)
{
  ObjectId id = getModel().newVariable(name);
  if (id != invalidId)
  {
    getModel().variables[id]->moveTo(pntInsert.x(), pntInsert.y());
    docHasChanged(caller, hintAddVariable, AnyParams(id));
  }
  return id;
}

void MinskyDoc::moveVariable(AbstractView* caller, ObjectId id, double x, double y)
{
  VariableManager::iterator it = getModel().variables.find(id);
  if (it != getModel().variables.end())
  {
    AnyParams data;
    data << id << (*it)->x() << (*it)->y() << x << y;
    (*it)->moveTo(x, y);
    setModified();
    docHasChanged(caller, hintMoveVariable, data);
  }
}

void MinskyDoc::removeVariable(AbstractView* caller, ObjectId id)
{
  docHasChanged(caller, hintRemoveVariable, AnyParams(id));
  getModel().deleteVariable(id);
}

MinskyDoc::ObjectId MinskyDoc::addGodleyTable(AbstractView* caller, const Wt::WPointF& pntInsert)
{
 ObjectId id = getModel().addGodleyTable(pntInsert.x(), pntInsert.y());

  if (id != invalidId)
  {
    docHasChanged(caller, hintAddGodleyTable, AnyParams(id));
  }

  return id;
}

void MinskyDoc::moveGodleyTable(AbstractView* caller, ObjectId id, double x, double y)
{
  minsky::Minsky::GodleyItems::iterator it = getModel().godleyItems.find(id);
  if (it != getModel().godleyItems.end())
  {
    AnyParams data;
    data << id << it->x() << it->y() << x << y;
    it->moveTo(x, y);
    setModified();
    docHasChanged(caller, hintMoveGodleyTable, data);
  }
}

void MinskyDoc::removeGodleyTable(AbstractView* caller, ObjectId id)
{
  docHasChanged(caller, hintRemoveGodleyTable, AnyParams(id));
  getModel().deleteGodleyTable(id);
}

void MinskyDoc::setGodleyCell(AbstractView* caller, ObjectId idTable, int row, int col, const std::string& value)
{
  AnyParams data;
  data << idTable << row << col << value;
  docHasChanged(caller, hintGodleyCellChanged, data);
  getModel().setGodleyCell(idTable, row, col, value);
}

int MinskyDoc::addPlot(AbstractView* caller, const Wt::WPointF& pntInsert)
{
  int id = getModel().newPlot(pntInsert.x(), pntInsert.y());
  //  TODO: this should really be in Plot constructor.
  getModel().plots[id].assignPorts();
  docHasChanged(caller, hintAddPlot, AnyParams(id));
  return id;
}

void MinskyDoc::movePlot(AbstractView* caller, int id, double x, double y)
{
  Plots::iterator it = getModel().plots.find(id);
  if (it != getModel().plots.end())
  {
    AnyParams data;
    data << id << it->x() << it->y() << x << y;
    it->moveTo(x, y);
    setModified();
    docHasChanged(caller, hintMovePlot, data);
  }
}

void MinskyDoc::removePlot(AbstractView* caller, int id)
{
  Plots::iterator it = getModel().plots.find(id);
  if (it != getModel().plots.end())
  {
    docHasChanged(caller, hintRemovePlot, AnyParams(id));
    getModel().deletePlot(id);
    setModified();
  }
}

void MinskyDoc::moveGroup(AbstractView* caller, ObjectId id, double x, double y)
{
  GroupIcons::iterator it = getModel().groupItems.find(id);
  if (it != getModel().groupItems.end())
  {
    AnyParams data;
    data << id << it->x() << it->y() << x << y;
    it->moveTo(x, y);
    it->updatePortLocation();
    setModified();
    docHasChanged(caller, hintMoveGroup, data);
  }
}

void MinskyDoc::addVariableToGroup(AbstractView* caller, ObjectId idGroup, ObjectId idVariable)
{
  getModel().addVariableToGroup(idGroup, idVariable);
  AnyParams data;
  data << idGroup << idVariable;
  docHasChanged(caller, hintAddVariableToGroup, data);
}

void MinskyDoc::removeVariableFromGroup(AbstractView* caller, ObjectId idGroup, ObjectId idVar)
{
  AnyParams data;
  data << idGroup << idVar;
  docHasChanged(caller, hintRemoveVariableFromGroup, data);
  getModel().removeVariableFromGroup(idGroup, idVar);
}

void MinskyDoc::addOperationToGroup(AbstractView* caller, ObjectId idGroup, ObjectId idOp)
{
  getModel().addOperationToGroup(idGroup, idOp);
  AnyParams data;
  data << idGroup << idOp;
  docHasChanged(caller, hintAddOperationToGroup, data);
}

void MinskyDoc::removeOperationFromGroup(AbstractView* caller, ObjectId idGroup, ObjectId idOp)
{
  AnyParams data;
  data << idGroup << idOp;
  docHasChanged(caller, hintRemoveOperationFromGroup, data);

  /// RKS - this would have thrown previously, and now doesn't even compile.
//  TCL_args args;
//  getModel().removeOperationFromGroup(args);
}

bool MinskyDoc::addGroupToGroup(AbstractView* caller, ObjectId idGroupContainer, ObjectId idGroup)
{
  bool b = getModel().addGroupToGroup(idGroupContainer, idGroup);
  if (b)
  {
    AnyParams data;
    data << idGroupContainer << idGroup;
    docHasChanged(caller, hintAddGroupToGroup, data);
  }
  return b;
}

void MinskyDoc::removeGroupFromGroup(AbstractView* caller, ObjectId idGroupContainer, ObjectId idGroup)
{
  AnyParams data;
  data << idGroupContainer << idGroup;
  docHasChanged(caller, hintRemoveGroupFromGroup, data);
  getModel().removeGroupFromGroup(idGroupContainer, idGroup);
}

void MinskyDoc::groupRect(AbstractView* caller, const Wt::WRectF& rect)
{
  getModel().select(rect.left(), rect.top(), rect.right(), rect.bottom());
  int id = getModel().createGroup();
  if (id >= 0)
  {
    docHasChanged(caller, hintNewGroup, AnyParams(id));
  }
}

void MinskyDoc::simulationStart(int speed)
{
  Wt::WTimer* timer = MinskyApp::getApp()->simulationTimer();
  if (!timer)
  {
    timer = MinskyApp::getApp()->createSimulationTimer();
    if (timer)
    {
      setSimulationSpeed(speed);
      timer->timeout().connect(
          [this](const WMouseEvent&)
          {
            simulationStep();
          });
    }
  }

  if (timer)
  {
    timer->start();
  }
}

void MinskyDoc::simulationStep()
{
  try
  {
    getModel().step();
    docHasChanged(NULL, hintSimulationStep, AnyParams());
  }
  catch(...)
  {
    // TODO: user feedback?
    simulationPause();
    docHasChanged(NULL, hintSimulationError, AnyParams());
  }
}

void MinskyDoc::simulationStop()
{
  simulationPause();
  simulationReset();
}

void MinskyDoc::simulationPause()
{
  if(simulationRunning())
  {
    MinskyApp::getApp()->destroySimulationTimer();
    docHasChanged(NULL, hintSimulationHalted, AnyParams());
  }
}

bool MinskyDoc::simulationRunning()
{
  return (MinskyApp::getApp()->simulationTimer() != NULL);
}

void MinskyDoc::simulationReset()
{
  getModel().reset();
  docHasChanged(NULL, hintSimulationReset, AnyParams());
}

void MinskyDoc::setSimulationSpeed(int speed)
{
  if (simulationRunning())
  {
    int delay;
    delay = maxSimulationDelay - ((maxSimulationDelay - minSimulationDelay) * (speed / 100.));
    MinskyApp::getApp()->simulationTimer()->setInterval(delay);
  }
}

}}  // namespace minsky::gui
