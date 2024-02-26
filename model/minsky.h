/*
  @copyright Steve Keen 2012
  @author Russell Standish
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

#ifndef MINSKY_H
#define MINSKY_H

#include "stringKeyMap.h"
#include "intrusiveMap.h"
#include "bookmark.h"
#include "canvas.h"
#include "clipboard.h"
#include "dimension.h"
#include "evalOp.h"
#include "equationDisplay.h"
#include "equations.h"
#include "fontDisplay.h"
#include "godleyIcon.h"
#include "intrusiveMap.h"
#include "latexMarkup.h"
#include "variableValues.h"
#include "canvas.h"
#include "lock.h"
#include "operation.h"
#include "pannableTab.h"
#include "phillipsDiagram.h"
#include "plotWidget.h"
#include "progress.h"
#include "pubTab.h"
#include "ravelWrap.h"
#include "rungeKutta.h"
#include "saver.h"
#include "stringKeyMap.h"
#include "variablePane.h"
#include "version.h"

#include <vector>
#include <string>
#include <set>
#include <deque>
#include <cstdio>

#include <ecolab.h>
#include <xml_pack_base.h>
#include <xml_unpack_base.h>

namespace minsky
{
  using namespace std;
  using classdesc::shared_ptr;
  using namespace civita;
  using namespace boost::posix_time;
  
  struct CallableFunction;
  class VariableInstanceList;
  
  class SaveThread;
  
  // a place to put working variables of the Minsky class that needn't
  // be serialised.
  struct MinskyExclude
  {
    shared_ptr<ofstream> outputDataFile;
    unique_ptr<BackgroundSaver> autoSaver;

    enum StateFlags {is_edited=1, reset_needed=2, fullEqnDisplay_needed=4};
    int flags=reset_needed;

    std::chrono::time_point<std::chrono::system_clock> resetAt=std::chrono::time_point<std::chrono::system_clock>::max();
    std::chrono::milliseconds resetDuration;

    Progress progressState;
    int busyCursorStack=0;
    
    std::vector<int> flagStack;

    std::map<std::string, std::shared_ptr<CallableFunction>> userFunctions;

    Clipboard clipboard; ///< clipboard manager
    
    // make copy operations just dummies, as assignment of Minsky's
    // doesn't need to change this
    MinskyExclude(): historyPtr(0) {}
    MinskyExclude(const MinskyExclude&): historyPtr(0) {}
    MinskyExclude& operator=(const MinskyExclude&) {return *this;}

    /// record nativeWindows that have requested redrawing
    std::set<RenderNativeWindow*> nativeWindowsToRedraw;
    
  protected:
    /// save history of model for undo
    /* 
       TODO: it should be sufficient to add move semantics to pack_t,
       but for some reason copy semantics are required, requiring the
       use of shared_ptr
     */
    std::deque<classdesc::pack_t> history;
    std::size_t historyPtr;
    bool undone=false; //< flag indicating undo() was previous command 
  };

  enum ItemType {wire, op, var, group, godley, plot};

  struct Minsky_multipleEquities: public ecolab::TCLAccessor<Minsky,bool>
  {
    Minsky_multipleEquities(const std::string& name, ecolab::TCLAccessor<Minsky,bool>::Getter g,
                            ecolab::TCLAccessor<Minsky,bool>::Setter s):
      ecolab::TCLAccessor<Minsky,bool>(name,g,s) {}  
  };

  
  class Minsky: public Exclude<MinskyExclude>, public RungeKutta, public Minsky_multipleEquities
  {
    CLASSDESC_ACCESS(Minsky);

    /// returns a diagnostic about an item that is infinite or
    /// NaN. Either a variable name, or and operator type.
    std::string diagnoseNonFinite() const;

    /// write current state of all variables to the log file
    void logVariables() const;

    Exclude<ptime> lastRedraw;

    bool m_multipleEquities=false;    
    
    /// balance two Godley columns
    void balanceColumns(const GodleyIcon& srcGodley, int srcCol, GodleyIcon& destGodley, int destCol) const;

    /// remove the definition network from \a wire, up to, but not including attached variables.
    void removeItems(Wire& wire);

  public:
    PannableTab<EquationDisplay> equationDisplay;
    FontDisplay fontSampler;
    PhillipsDiagram phillipsDiagram;
    std::vector<PubTab> publicationTabs;

    void addNewPublicationTab(const std::string& name) {publicationTabs.emplace_back(name);}
    void addCanvasItemToPublicationTab(size_t i) {
      if (canvas.item && i<publicationTabs.size())
        publicationTabs[i].items.emplace_back(canvas.item);
    }
    
    // Allow multiple equity columns.
    bool multipleEquities() const {return m_multipleEquities;}
    bool multipleEquities(const bool& m);
    
    /// reflects whether the model has been changed since last save
    bool edited() const {return flags & is_edited;}
    /// true if reset needs to be called prior to numerical integration
    bool reset_flag() const {return flags & reset_needed;}
    /// indicate model has been changed since last saved
    void markEdited() {
      flags |= is_edited | fullEqnDisplay_needed;
      if (!running) flags|=reset_needed; // don't reset when running
      variablePane.update();
      canvas.requestRedraw();
      canvas.model.updateTimestamp();
    }
    void requestReset();
    /// requests a redraw of the curren active tab
    void requestRedraw();
    
    /// @{ push and pop state of the flags
    void pushFlags() {flagStack.push_back(flags);}
    void popFlags() {
      if (!flagStack.empty()) {
        flags=flagStack.back();
        flagStack.pop_back();
      }
    }
    /// @}

    bool resetIfFlagged() override {
      if (reset_flag())
        reset();
      return reset_flag();
    }
    
    VariableValues variableValues;
    Dimensions dimensions;
    Conversions conversions;
    /// stash the maximum absolute value obtained by a dimensioned quantity
    std::map<Units, double> maxValue;
    std::map<Units, double> maxFlowValue; // max flow values along wires
    /// fills in dimensions table with all loaded ravel axes
    void populateMissingDimensions();
    void populateMissingDimensionsFromVariable(const VariableValue&);
    void renameDimension(const std::string& oldName, const std::string& newName);
    
    void setGodleyIconResource(const string& s)
    {GodleyIcon::svgRenderer.setResource(s);}
    void setGroupIconResource(const string& s)
    {Group::svgRenderer.setResource(s);}
    void setLockIconResource(const string& locked, const string& unlocked) {
      Lock::lockedIcon.setResource(locked);
      Lock::unlockedIcon.setResource(unlocked);
    }
    void setRavelIconResource(const string& s)
    {Ravel::svgRenderer.setResource(s);}
    SVGRenderer histogramResource;
    
    /// @return available matching columns from other Godley tables
    /// @param currTable - this table, not included in the matching process
    //  @param ac type of column we wish matches for
    std::set<string> matchingTableColumns(const GodleyIcon& currTable, GodleyAssetClass::AssetClass ac);

    /// find any duplicate column, and use it as a source column for balanceDuplicateColumns
    void importDuplicateColumn(GodleyTable& srcTable, int srcCol);

    /// makes all duplicated columns consistent with \a srcTable, \a srcCol
    void balanceDuplicateColumns(const GodleyIcon& srcTable, int srcCol);

    std::vector<std::string> allGodleyFlowVars() const;
    
    // reset m_edited as the GodleyIcon constructor calls markEdited
    Minsky():
      ECOLAB_ACESSOR_INIT(Minsky, multipleEquities),
      equationDisplay(*this) {
      lastRedraw=boost::posix_time::microsec_clock::local_time();
      model->iHeight(std::numeric_limits<float>::max());
      model->iWidth(std::numeric_limits<float>::max());
      model->self=model;
      publicationTabs.emplace_back("Publication");
    }
    ~Minsky();
    
    GroupPtr model{new Group};
    Canvas canvas{model};

    void clearAllMaps(bool clearHistory);
    void clearAllMaps() {clearAllMaps(true);}
    // for TCL use
    void clearAllMapsTCL() {clearAllMaps(true);}
    
    /// returns reference to variable defining (ie input wired) for valueId
    VariablePtr definingVar(const std::string& valueId) const;

    static void saveGroupAsFile(const Group&, const string& fileName);
    void saveCanvasItemAsFile(const string& fileName) const
    {if (auto g=dynamic_cast<Group*>(canvas.item.get())) saveGroupAsFile(*g,fileName);}

    void initGodleys();

    /// erase items in current selection, put copy into clipboard
    void cut();
    /// copy items in current selection into clipboard
    void copy() const;
    /// paste clipboard as a new group or ungrouped items on the canvas. canvas.itemFocus is set to
    /// refer to the new group or items.
    void paste();
    /// return true if nothing available in clipboard
    bool clipboardEmpty() const {return clipboard.getClipboard().empty();}
    void saveSelectionAsFile(const string& fileName) const {saveGroupAsFile(canvas.selection,fileName);}
    
    void insertGroupFromFile(const string& file);

    void makeVariablesConsistent();

    void imposeDimensions();

    // runs over all ports and variables removing those not in use
    void garbageCollect();

    /// checks for presence of illegal cycles in network. Returns true
    /// if there are some
    bool cycleCheck() const;

    /// opens the log file, and writes out a header line describing
    /// names of all variables
    void openLogFile(const string&);
    /// closes log file
    void closeLogFile() {outputDataFile.reset();}
    /// set of variables (valueIds) to log
    std::set<string> logVarList;
    /// returns true if logging is in operation
    bool loggingEnabled() const {return outputDataFile.get();}
    
    /// construct the equations based on input data
    /// @throws ecolab::error if the data is inconsistent
    void constructEquations();
    /// performs dimension analysis, throws if there is a problem
    void dimensionalAnalysis() const;
    /// removes units markup from all variables in model
    void deleteAllUnits();
    
    /// consistency check of the equation order. Should return
    /// true. Outputs the operation number of the invalidly ordered
    /// operation.
    bool checkEquationOrder() const;

    
    double lastT{0}; ///<previous timestep
    double deltaT() const {return t-lastT;}
    void reset(); ///<resets the variables back to their initial values
    std::vector<double> step();  ///< step the equations (by n steps, default 1)

    int numBackups=1; ///< number of previous versions of saved files to keep
    /// save to a file
    void save(const std::string& filename);
    /// load from a file
    void load(const std::string& filename);

    /*static*/ void exportSchema(const std::string& filename, int schemaLevel=1);

    /// indicate operation item has error, if visible, otherwise contining group
    void displayErrorItem(const Item& op) const;

    /// return the AEGIS assigned version number
    static const std::string minskyVersion;
    /*static*/ std::string ecolabVersion() const {return VERSION;}
    /*static*/ std::string ravelVersion() const {
      if (ravel::Ravel::available())
        {
          int d=ravel::Ravel::daysUntilExpired();
          return ravel::Ravel::version() + ": "+((d>=0)?("Expires in "+std::to_string(d)+" day"+(d!=1?"s":"")): "Expired");
        }
      else return "unavailable";
    }
    static bool ravelExpired() {return  ravel::Ravel::available() && ravel::Ravel::daysUntilExpired()<0;}
    
    std::string fileVersion; ///< Minsky version file was saved under
    
    unsigned maxHistory{100}; ///< maximum no. of history states to save
    int maxWaitMS=100; ///< maximum  wait in millisecond between redrawing canvas during simulation

    /// name of an auto save file
    std::string autoSaveFile() const {return autoSaver? autoSaver->fileName: std::string();}
    /// initialises auto saving
    /// empty \a file to turn off autosave
    void setAutoSaveFile(const std::string& file);
    
    /// clear history
    void clearHistory() {history.clear(); historyPtr=0;}
    /// called periodically to ensure history up to date
    void checkPushHistory() {if (historyPtr==history.size()) pushHistory();}

    /// push current model state onto history if it differs from previous
    bool pushHistory();

    /// Executed after each interpreter command to maintain undo/redo stack, edited flag, autosaving etc.
    /// @param command '.' separated command
    /// @param nargs number of arguments
    /// @return whether to save the command when recording events
    bool commandHook(const std::string& command, unsigned nargs);

    enum CmdData {no_command, is_const, is_setterGetter, generic};

    /// return meta information on a given command
    virtual CmdData getCommandData(const std::string& command) const {return generic;}
    
    /// flag to indicate whether a TCL should be pushed onto the
    /// history stack, or logged in a recording. This is used to avoid
    /// movements being added to recordings and undo history
    bool doPushHistory=true;

    /// restore model to state \a changes ago
    /// @return index of current state in history
    long undo(int changes=1);

    /// set a Tk image to render equations to
    void renderEquationsToImage(const std::string& image);

    /// Converts variable(s) named by \a name into a variable of type \a type.
    /// @throw if conversion is disallowed
    void convertVarType(const std::string& name, VariableType::Type type);

    /// add integral to current canvas item (which must be variable
    /// convertible to an integral variable
    void addIntegral();
    
    /// returns true if any variable of name \a name has a wired input
    bool inputWired(const std::string& name) const {return definingVar(name).get();}

    /// render canvas to a postscript file
    void renderCanvasToPS(const std::string& filename) {canvas.renderToPS(filename);}
    /// render canvas to a PDF file
    void renderCanvasToPDF(const std::string& filename) {canvas.renderToPDF(filename);}
    /// render canvas to an SVG file
    void renderCanvasToSVG(const std::string& filename) {canvas.renderToSVG(filename);}
    /// render canvas to a PNG image file
    void renderCanvasToPNG(const std::string& filename) {canvas.renderToPNG(filename);}
    /// render canvas to a EMF image file (Windows only)
    void renderCanvasToEMF(const std::string& filename) {canvas.renderToEMF(filename);}
    
    /// render all plots 
    void renderAllPlotsAsSVG(const string& prefix) const;
    /// export all plots
    void exportAllPlotsAsCSV(const string& prefix) const;

    /// set DE mode on all godley tables
    void setAllDEmode(bool);
    /// set std library RNG seed
    void srand(int seed) {::srand(seed);}

    // godley table display values preferences
    bool displayValues=false;
    GodleyTable::DisplayStyle displayStyle=GodleyTable::sign;

    /// set display value mode on all godley table editor modes
    void setGodleyDisplayValue(bool displayValues, GodleyTable::DisplayStyle displayStyle);

    /// import a Vensim file
    void importVensim(const std::string&);
    
    /// request all Godley table windows to redraw
    void redrawAllGodleyTables();

    /// set/clear busy cursor in GUI
    virtual void setBusyCursor() {}
    virtual void clearBusyCursor() {}
    /// set progress bar, out of 100, labelling the progress bar with \a title
    virtual void progress(const std::string& title,int) {}

    /// refresh the bookmark menu after changes
    virtual void bookmarkRefresh() {}
    
    /// reset main window scroll bars after model has been panned
    virtual void resetScroll() {}
    
    /// display a message in a popup box on the GUI
    virtual void message(const std::string&) {}

    /// run callback attached to \a item
    virtual void runItemDeletedCallback(const Item&) {}
    
    /// check whether to proceed or abort, given a request to allocate
    /// \a bytes of memory. Implemented in MinskyTCL
    virtual bool checkMemAllocation(size_t bytes) const {return true;}

    /// returns amount of memory installed on system
    /*static*/ std::size_t physicalMem() const;
    
    vector<string> listFonts() const {
      vector<string> r;
#ifdef PANGO
      PangoFontFamily **families;
      int num;
      pango_font_map_list_families(pango_cairo_font_map_get_default(),
                                   &families,&num);
      for (int i=0; i<num; ++i)
        r.push_back(pango_font_family_get_name(families[i]));
      g_free(families);
      std::sort(r.begin(),r.end());
#endif
      return r;
    }

    /// @{ the default used by Pango
    /*static*/ std::string defaultFont();
    /*static*/ std::string defaultFont(const std::string& x);
    /// @}

    /// @{ an extra scaling factor of Pango fonts
    /*static*/ double fontScale();
    /*static*/ double fontScale(double);
    /// @}
    
    /*static*/ int numOpArgs(OperationType::Type o);
    OperationType::Group classifyOp(OperationType::Type o) const {return OperationType::classify(o);}

    void latex(const std::string& filename, bool wrapLaTeXLines);

    void matlab(const std::string& filename) {
      if (cycleCheck()) throw error("cyclic network detected");
      ofstream f(filename);
      MathDAG::SystemOfEquations(*this).matlab(f);
    }

    // for testing purposes
    string latex2pango(const std::string& x) {return latexToPango(x.c_str());}

    /// list of available operations
    /*static*/ std::vector<std::string> availableOperations();
    using AvailableOperationsMapping=classdesc::StringKeyMap<std::vector<OperationType::Type>>;
    /*static*/ Minsky::AvailableOperationsMapping availableOperationsMapping() const;
    
    /// list of available variable types
    /*static*/ std::vector<std::string> variableTypes();
    /// return list of available asset classes
    /*static*/ std::vector<std::string> assetClasses();

    void autoLayout(); ///< auto layout current open group and recentre
    void randomLayout(); ///< randomly layout current open group and recentre
    /// reinitialises canvas to the group located in item
    void openGroupInCanvas() {canvas.openGroupInCanvas(canvas.item);}
    /// reinitialises canvas to the toplevel group
    void openModelInCanvas() {canvas.openGroupInCanvas(model);}

    /// supports navigation to all instances of current variable
    std::shared_ptr<VariableInstanceList> variableInstanceList;
    void listAllInstances();

    std::map<std::string,std::weak_ptr<Item>> namedItems;
    void nameCurrentItem(const std::string& name) {namedItems[name]=canvas.item;}
    void itemFromNamedItem(const std::string& name) {canvas.item=namedItems[name].lock();}

    /// trigger checkMem callback for testing purposes
    bool triggerCheckMemAllocationCallback() const
    {return checkMemAllocation(std::numeric_limits<size_t>::max());}

    VariablePane variablePane;

    /// Used to implement a pause until return pressed for attaching debugger purposes
    char getc() const {return std::getc(stdin);}

    /// add/replace a definition for a flow variable given by \a valueId
    void setDefinition(const std::string& valueId, const std::string& definition);
  };

  /// global minsky object
  Minsky& minsky();
  /// const version to help in const correctness
  inline const Minsky& cminsky() {return minsky();}
  /// RAII set the minsky object to a different one for the current scope.
  struct LocalMinsky
  {
    LocalMinsky(Minsky& m);
    ~LocalMinsky();
  };



}

#ifdef _CLASSDESC
#pragma omit pack minsky::MinskyExclude
#pragma omit unpack minsky::MinskyExclude
#pragma omit TCL_obj minsky::MinskyExclude
#pragma omit xml_pack minsky::MinskyExclude
#pragma omit xml_unpack minsky::MinskyExclude
#pragma omit xsd_generate minsky::MinskyExclude

#pragma omit xml_pack minsky::Integral
#pragma omit xml_unpack minsky::Integral

#pragma omit pack minsky::MinskyMatrix
#pragma omit unpack minsky::MinskyMatrix
#pragma omit xml_pack minsky::MinskyMatrix
#pragma omit xml_unpack minsky::MinskyMatrix
#pragma omit xsd_generate minsky::MinskyMatrix
#endif

#include "minsky.cd"
#endif
