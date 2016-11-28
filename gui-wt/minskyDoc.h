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

#ifndef _MINSKYDOC_H
#define _MINSKYDOC_H

#include "guiDefs.h"
#include "anyParams.h"
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <boost/exception/all.hpp>
#include <Wt/WPointF>
#include "GUI/minsky.h"
#undef None

namespace minsky { namespace gui {

class AbstractView;
class AnyParams;

/**
 *  The document object that links Minsky model data with the GUI.
 *
 *  This class is really a facade. To access the actual document, use getModel().
 *
 *  @private
 *
 *  @note   The actual GUI/minsky::Minksy models makes use of a global minsky
 *          model.  This means that the can only be one MinskyDoc objet for the
 *          application also.
 *
 *          This also means that this class is stateless, except for this UI
 *          specific data, such as file name.
 *
 *          This breaks the classic view/model paradigm, but is safe
 *          to incorporate in a <b><u>single</u></b> document environment.
 *
 *          Also, the model is inextricably linked to its present view.  That
 *          is not a problem per se, but that must be kept in mind.
 *          Fortunately, this does not affect other views inside the model,
 *          such as tables, etc...
 *
 *  @note   The filenames are narrow characters.  No support for UNICODE file
 *          names.
 */
class MinskyDoc
{
public:
  /**
   *  Exception thrown when a file operation was attempted on an file with no
   *  name
   */
  struct emptyFileNameException :
      public virtual std::exception,
      public virtual boost::exception {};

    typedef int ObjectId;

    /**
     *  ID constants.
     */
    enum
    {
      invalidId = -1, ///<  Invalid ID.
    };

    /**
     * Simulation constants
     */
    enum
    {
      maxSimulationDelay = 500,
      minSimulationDelay = 20,
    };

    /**
     *  Hints are used when notifying views of model changes.
     *  As a side effect this enum, also lists all available
     *  operations modifying the model.
     *
     *  As a rule, hints about object deletion or removal are
     *  called before the operation is applied to the model,
     *  this gives a chance for listening views to get context
     *  information from the model while the object is still
     *  in place.
     *
     *  @see docHasChanged()
     */
    enum Hint
    {
      hintDocumentLoad,       ///< A new Document has been loaded.
      hintDocumentClose,      ///< The document is about to unload, all views
                              ///  should clear their internal data.
      hintDocumentSaved,      ///< Document has been saved.
      hintDocumentModified,   ///< The document has been modified.  Sent only once on
                              ///  first modification.
      hintAddOperation,       ///< An operation was inserted in the model.
                              ///  @param id Id of the new operation object.
      hintEditOperation,      ///< An operation's attributes are about to change.
                              ///  @param id Id of the operation object.
      hintRemoveOperation,    ///< An operation is about to be removed from the model.
                              ///  @param id Id of the affected object.
      hintMoveOperation,      ///< An operation has been moved.
                              /// @param id Id of the affected object.
                              /// @param oldX [float] X position before the move
                              /// @param oldY [float] Y position before the move
                              /// @param newX [double] X position after the move
                              /// @param newX [double] Y position after the move
      hintAddWire,            ///< A wire was added to the model.
                              ///  @param id Wire ID.
      hintRemoveWire,         ///< A wire is about to be deleted.
                              ///  @param id Wire ID.
      hintEditWire,           ///< A wire was edited.
                              ///  @param id Wire ID.
                              ///  @param ecolab::array<float> old wire data.
                              ///  @param ecolab::array<float> new wire data.
      hintAddVariable,        ///< A variable was added to the model.
                              ///  @param id Variable ID.
      hintMoveVariable,       /// A variable was moved on the canvas.
                              /// @param id Id of the affected object.
                              /// @param oldX [float] X position before the move
                              /// @param oldY [float] Y position before the move
                              /// @param newX [double] X position after the move
                              /// @param newX [double] Y position after the move
      hintEditVariable,       ///< An variables's attributes are about to change.
                              ///  @param id Id of the operation object.
      hintRemoveVariable,     ///< A variable os about tyo be removed from the model.
                              ///  @param id Variable ID.
      hintAddGodleyTable,     ///< A Godley table was added to the model.
                              ///  @param id Table ID.
      hintMoveGodleyTable,    ///< A godley table (icon) has been moved.
                              /// @param id Id of the affected object.
                              /// @param oldX [float] X position before the move
                              /// @param oldY [float] Y position before the move
                              /// @param newX [double] X position after the move
                              /// @param newX [double] Y position after the move
      hintEditGodleyTable,    ///< A godley table's attributes are about to change.
                              ///  @param id Id of the table icon.
      hintRemoveGodleyTable,  ///< A Godley table is about to be deleted.
                              ///  @param id Table ID.
      hintGodleyCellChanged,  ///< A Godley table cell value has changed.
                              ///  @param id Table ID.
                              ///  @param row Row index.
                              ///  @param col Column index.
                              ///  @param value The new cell value as an std::string.
      hintAddPlot,            ///<  A PlotWidget was added to the model.
                              ///   @param id [std::string] plot ID.
      hintMovePlot,           ///<  A plot widget was moved.
                              /// @param id Id of the affected object.
                              /// @param oldX [float] X position before the move
                              /// @param oldY [float] Y position before the move
                              /// @param newX [double] X position after the move
                              /// @param newX [double] Y position after the move
      hintRemovePlot,         /// A Plot is about to be removed from the model.
                              /// @param id [std::string] plot ID.
      hintMoveGroup,          ///<  A group icon was moved.
                              /// @param id Id of the affected object.
                              /// @param oldX [float] X position before the move
                              /// @param oldY [float] Y position before the move
                              /// @param newX [double] X position after the move
                              /// @param newX [double] Y position after the move
      hintAddVariableToGroup, ///< A variable reference was inserted in a group.
                              ///  @param idGroup Group ID.
                              ///  @param idVar Variable ID.
      hintRemoveVariableFromGroup,  ///< A variable reference is about to be removed from a group.
                                    ///  @param idGroup Group ID.
                                    ///  @param idVar Variable ID.
      hintAddOperationToGroup,      ///< An operation is being inserted in a group.
                                    ///  @param idGroup Group ID.
                                    ///  @param idOp Operation ID.
      hintRemoveOperationFromGroup, ///< An operation is about to be removed from a group.
                                    ///  @param idGroup Group ID.
                                    ///  @param idOp Operation ID.
      hintAddGroupToGroup,      ///< A group is being nested into another.
                                ///  @param idContainer Group ID for the outer group.
                                ///  @param idGroup Group ID for the inner group.
      hintRemoveGroupFromGroup, ///< A Group is being removed from another group.
                                ///  @param idContainer Group ID for the outer group.
                                ///  @param idGroup Group ID for the inner group.
      hintNewGroup,             ///<  A Group was created.
                                ///   @param idGroup ID of the new group.
      hintSimulationStep,       ///< A simulation step has been generated.
      hintSimulationReset,      ///< Simulation reset.
      hintSimulationHalted,     ///< Simulation was halted.
      hintSimulationError,      ///< An error has occured during simulation
      /* ... */
    };

public:
  /**
   *  A set of document properties.
   */
  typedef std::map<std::string, std::string> Properties;

private:
  bool        autoFileName;     ///<  Indicates whether the filename was set by
                                ///   the user, or is assigned automatically.
  std::string fileName;         ///<  The filename for this document.


public:
  /**
   *  Constructor.
   */
  MinskyDoc();

  /**
   *  Destructor.
   */
  ~MinskyDoc();

public:
  /**
   *  Returns the file name.  If none has been assigned yet, a name is
   *  generated automatically.
   */
  const string& getFileName() const;

  /**
   *  MinskyDoc is a stateless facade. This method gives access to the real
   *  document MinskyDoc provides an interface for. Used throughout this class,
   *  as it refers to a unique object.
   */
  /** @{ */
  inline minsky::Minsky& getModel()
  {
    return minsky();
  }

  inline const minsky::Minsky& getModel() const
  {
    return minsky();
  }
  /** @} */

protected:
  /**
   * Generates an automatic filename.
   */
  static void generateFileName(std::string& name);

  /**
   *  Called internally to propagate change to all views, including the undo view.
   *  @param caller  The view that initiated the command.
   *  @param hint    The command.
   *  @param args    Command data.
   *  @see Hint.
   */
  void docHasChanged(AbstractView* caller, Hint hint, const AnyParams& data);

public:
  /**
   *  Loads data from a file.
   *  @param name A valid path name to the file to load data from.
   *              Typically called from MinksyApp, which holds
   *              the logic for displaying the file open dialog
   */
  void loadFromFile(const std::string& name);

  /**
   *  Saves to file, optionally with a different file name.
   *  @param  name  An optional, valid new name for the file to save to, or
   *                an empty string to save for the file default name.
   *  @see fileName
   */
  void saveToFile(const std::string& name = std::string());

  /**
   *  Destroys the object data.
   *  @see destroy()
   */
  void close();

  /**
   *  Accessor for the file name.
   *  @see MinskyDoc::fileName
   */
  const std::string& getFileName()
  {
    if (fileName.empty())
    {
      generateFileName(fileName);
      autoFileName = true;
    }
    return fileName;
  }

  /**
   *  Indicates wether the document has been modified since
   *  it was loaded.
   */
  inline bool isModified() const
  {
    return getModel().edited();
  }

  /**
   *  Sets the state of the modified flag.
   *  @param [opt] hasChanged  New state for the modified flag.
   */
  inline void setModified(bool hasChanged = true)
  {
    if (hasChanged)
    {
      if (!getModel().edited())
      {
        getModel().markEdited();
        docHasChanged(NULL, hintDocumentModified, AnyParams());
      }
    }
    else
    {
      getModel().resetEdited();
    }
  }

  /**
   *  Displays the document's properties.
   */
  void getProperties(Properties& properties);

  //
  //  Accessors
  //

  minsky::OperationPtr getOperation(ObjectId id)
  {
    return getModel().operations[id];
  }

  minsky::VariablePtr getVariable(ObjectId id)
  {
    return getModel().variables[id];
  }

  minsky::Wire& getWire(ObjectId id)
  {
    if (getModel().wires.find(id) == getModel().wires.end())
    {
      throw 1;
    }
    return getModel().wires[id];
  }

//  minsky::Plot* getPlot(ObjectId id)
//  {
//    return getModel().plots[id];
//  }
//
  minsky::GodleyIcon& getGodleyTable(ObjectId id)
  {
    return getModel().godleyItems[id];
  }

  //
  //  Operations affecting the model
  //

  /**
   *  Inserts an operation into the model
   *  @param caller View requesting the insertion.
   *  @param opType Type of new operation.
   */
  ObjectId addOperation(AbstractView* caller,
                        minsky::OperationType::Type opType,
                         const Wt::WPointF& pntInsert);

  /**
   * Remove an operation from the model.
   * @param caller  The view that initiated the command.
   * @param id      The object ID of the operation.
   */
  void removeOperation(AbstractView* caller, ObjectId id);

  /**
   * Changes the position of an operation
   *
   */
  void moveOperation(AbstractView* caller, ObjectId id, double x, double y);

  /**
   * Changes the value of an operation's slider.
   */
  void setOperationSliderValue(AbstractView* caller, ObjectId id, double value);

  /**
   * Adds a wire to the model.
   * @param caller  The view that initiated the command.
   * @param from    Origin of the link the wire creates.
   * @param to      Destination of the link.
   * @returns The object Id of the newly created link.
   */
  ObjectId addWire(AbstractView* caller, ObjectId from, ObjectId to);

  /**
   * Removes a wire from the model.
   * @param caller  The view that initiated the command.
   * @param id      The object ID of the wire.
   */
  void removeWire(AbstractView* caller, ObjectId id);

  /**
   * Adds a point to a wire
   */
  void addWirePoint(AbstractView* caller, ObjectId id, size_t pointIndex, float x, float y);

  /**
   * Removes a point from a wire
   */
  void removeWirePoint(AbstractView* caller, ObjectId id, size_t pointIndex);

  /**
   * Sets a wire constrol point
   */
  void setWirePoint(AbstractView* caller, ObjectId id, size_t pointIndex, float x, float y);

  /**
   * Adds a variable to the model.
   * @param caller  The view that initiated the command.
   */
  ObjectId addVariable(AbstractView* caller, const std::string& name,
                      const Wt::WPointF& pntInsert);

  /**
   * Changes the position of a variable
   *
   */
  void moveVariable(AbstractView* caller, ObjectId id, double x, double y);

  /**
   * Removes a variable from the model.
   * @param caller  The view that initiated the command.
   * @param id      The object ID of the variable.
   */
  void removeVariable(AbstractView* caller, ObjectId id);

  /**
   * Adds a Godley table.
   * @param caller  The view that initiated the command.
   */
  ObjectId addGodleyTable(AbstractView* caller, const Wt::WPointF& pntInsert);

  /**
   * Changes the position of an godley table
   *
   */
  void moveGodleyTable(AbstractView* caller, ObjectId id, double x, double y);

  /**
   * Removes a Godley table.
   * @param caller  The view that initiated the command.
   * @param id      The object ID of the table.
   */
  void removeGodleyTable(AbstractView* caller, ObjectId id);

  /**
   * Sets a goldey cell value.
   * @param caller  The view that initiated the command.
   */
  void setGodleyCell(AbstractView* caller, ObjectId idTable, int row, int col, const std::string& value);

  /**
   * Adds a plot widget in the model.
   */
  int addPlot(AbstractView* caller, const Wt::WPointF& pntInsert);

  /**
   * Changes the position of a plot widget
   */
  void movePlot(AbstractView* caller, int id, double x, double y);

  /**
   * Removes a Plot from the model.
   * @param caller  The view that initiated the command.
   * @param id      The object ID of the plot widget.
   */
  void removePlot(AbstractView* caller, int id);

  /**
   * Changes the position of a group icon
   */
  void moveGroup(AbstractView* caller, ObjectId id, double x, double y);

  /**
   * Adds a variable to a group.
   * @param caller  The view that initiated the command.
   */
  void addVariableToGroup(AbstractView* caller, ObjectId idGroup, ObjectId idVar);

  /**
   * Removes a variable from a group.
   * @param caller  The view that initiated the command.
   */
  void removeVariableFromGroup(AbstractView* caller, ObjectId idGroup, ObjectId idVar);

  /**
   * Adds an operation to a group
   * @param caller  The view that initiated the command.
   */
  void addOperationToGroup(AbstractView* caller, ObjectId idGroup, ObjectId idOp);

  /**
   * Removes an operation from a group
   * @param caller  The view that initiated the command.
   */
  void removeOperationFromGroup(AbstractView* caller, ObjectId idGroup, ObjectId idOp);

  /**
   * Nests a group inside another.
   * @param caller  The view that initiated the command.
   */
  bool addGroupToGroup(AbstractView* caller, ObjectId idGroupContainer, ObjectId idGroup);

  /**
   * Removes a nested group.
   * @param caller  The view that initiated the command.
   */
  void removeGroupFromGroup(AbstractView* caller, ObjectId idGroupContainer, ObjectId idGroup);

  /**
   * Groups items laying within a rectangle
   * @param rect Area in model space.
   */
  void groupRect(AbstractView* caller, const Wt::WRectF& rect);

  /**
   * Generates a single simulation step.
   */
  void simulationStep();

  /**
   * Starts the simulation.
   * @param speed Simulation speed, value between 1 and 100.
   */
  void simulationStart(int speed);

  /**
   * Stops the simulation.
   */
  void simulationStop();

  /**
   * pauses the simulation
   */
  void simulationPause();

  /**
   * Returns whether the simulation is running or not.
   */
  bool simulationRunning();

  /**
   * Resets simulation data.
   */
  void simulationReset();

  /**
   * Sets the simulation speed.
   * @param speed a value between 1 and 100.
   */
  void setSimulationSpeed(int speed);

  /**
   * Indicates whether a variable exists.
   * @param name Variable name.
   * @return true if a variable with that name exists
   */
  inline bool variableExists(const std::string& name) const
  {
    return getModel().variables.exists(name);
  }

  /**
   * Returns a variable's value
   * @param varName Name of the variable
   * @return The value of the variable \p varName
   */
  inline double getVariableValue(const std::string& varName) const
  {
    return getModel().variables.getVariableValue(varName).value();
  }
};

}}  // namespace minsky::gui

#endif
