/*
  @copyright Steve Keen 2018
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

#ifndef RAVELWRAP_H
#define RAVELWRAP_H

#include "operation.h"
#include "cairoRenderer.h"
#include "dynamicRavelCAPI.h"
#include "hypercube.h"
#include "handleLockInfo.h"
#include "renderNativeWindow.h"
#include "SVGItem.h"

namespace minsky 
{
  using namespace civita;
  class RavelLockGroup;

  class Ravel;
  class RavelPopup: public RenderNativeWindow
  {
    Ravel& ravel;
    float width, height, scale=1;
    float localX(float x) const;
    float localY(float y) const;
    bool redraw(int x0, int y0, int width, int height) override;
    CLASSDESC_ACCESS(RavelPopup);
  public:
    RavelPopup(Ravel& ravel): ravel(ravel) {}
    void mouseDown(float x, float y) override;
    void mouseUp(float x, float y) override;
    void mouseMove(float x, float y) override;
    void mouseOver(float x, float y);
    void mouseLeave();
    bool keyPress(const EventInterface::KeyPressArgs&) override;
  };
  
  class Ravel: public ItemT<Ravel, Operation<OperationType::ravel>>, public classdesc::Exclude<ravel::Ravel>
  {
  public:

    CLASSDESC_ACCESS(Ravel);
  private:
    using HandleState=ravel::HandleState;
    /// position of the "move" handle, as a proportion of radius
    const double moveX=0.5, moveY=0.5, moveSz=0.1;

    /// indicate whether icon is in editor mode or icon mode
    bool m_editorMode=false;
    //std::string m_filename;
    std::string explanation; // explanation of Ravel bits displayed as tooltip
    ravel::HandleSort::Order previousOrder=ravel::HandleSort::staticForward;
    
    /// used entirely to defer persisted state data until after first
    /// load from a variable
    ravel::RavelState initState;
    
    friend struct SchemaHelper;
    friend RavelPopup;

    std::vector<std::string> allSliceLabelsImpl(int axis, ravel::HandleSort::Order) const;

    ravel::Ravel wrappedRavel;
  public:
    static SVGRenderer svgRenderer; ///< SVG icon to display when not in editor mode
    RavelPopup popup; ///< popup Ravel control window
    Ravel();
    // copy operations needed for clone, but not really used for now
    // define them as empty operations to prevent double frees if accidentally used
    void operator=(const Ravel&) {}
    Ravel(const Ravel& x): ItemT<Ravel, Operation<OperationType::ravel>>(x), popup(*this)  {
      applyState(x.getState());
    }

    const Ravel* ravelCast() const override {return this;}
    Ravel* ravelCast() override {return this;}

    /// local override of axis dimensionality
    Dimensions axisDimensions;

    /// group of ravels that move syncronously
    std::shared_ptr<RavelLockGroup> lockGroup;
    void leaveLockGroup();
    void broadcastStateToLockGroup() const;
    
    void draw(cairo_t* cairo) const override;
    void resize(const LassoBox&) override;
    bool inItem(float x, float y) const override;
    void onMouseDown(float x, float y) override;
    void onMouseUp(float x, float y) override;
    bool onMouseMotion(float x, float y) override;
    bool onMouseOver(float x, float y) override;
    void onMouseLeave() override {wrappedRavel.onMouseLeave();}
    /// return hypercube corresponding to the current Ravel state
    Hypercube hypercube() const;
    void populateHypercube(const Hypercube&);
    /// @return input rank (aka numHandles)
    unsigned maxRank() const {return wrappedRavel.numHandles();}
    unsigned numHandles() const {return wrappedRavel.numHandles();}
    /// number of slice labels along axis \a axis
    size_t numSliceLabels(size_t axis) const {return wrappedRavel.numSliceLabels(axis);}
    /// adjust output dimensions to first \a r handles
    void setRank(unsigned r);
    void adjustSlicer(int); ///< adjust currently selected handle's slicer
    bool onKeyPress(int, const std::string&, int) override; 
    /// redistribute handles according to current state
    void redistributeHandles() {wrappedRavel.redistributeHandles();}
    /// sets the type of the next reduction operation
    void nextReduction(ravel::Op::ReductionOp op) {wrappedRavel.nextReduction(op);}
    /// set the reduction type for \a handle
    void handleSetReduction(int handle, ravel::Op::ReductionOp op) {wrappedRavel.handleSetReduction(handle, op);}
    /// current handle mouse is over, or -1 if none
    int selectedHandle() const {return wrappedRavel.selectedHandle();}

    /// enable/disable calipers on currently selected handle
    bool displayFilterCaliper() const;
    bool setDisplayFilterCaliper(bool x);
    bool toggleDisplayFilterCaliper()
    {return setDisplayFilterCaliper(!displayFilterCaliper());}
    /// @}

    /// returns all slice labels along the selected handle, in specified order
    std::vector<std::string> allSliceLabels() const;
    /// returns all slice labels along an axis(dimension) identified by its number
    std::vector<std::string> allSliceLabelsAxis(int axis) const;
    /// returns just the picked slice labels along the handle
    std::vector<std::string> pickedSliceLabels() const;
    /// pick (selected) \a pick labels
    void pickSliceLabels(int axis, const std::vector<std::string>& pick);

    /// return all handle names
    std::vector<std::string> handleNames() const {
      std::vector<std::string> r;
      for (size_t i=0; i<wrappedRavel.numHandles(); ++i)
        r.push_back(wrappedRavel.handleDescription(i));
      return r;
    }
    
    /// dimension details associated with handle 
    Dimension dimension(int handle) const;
    
    /// @{
    /// the handle sorting order for currently selected handle
    ravel::HandleSort::Order sortOrder() const;
    ravel::HandleSort::Order setSortOrder(ravel::HandleSort::Order);
    /// @}

    void resortHandleIfDynamic();
    
    /// set a given handle sort order
    ravel::HandleSort::Order setHandleSortOrder(ravel::HandleSort::Order, int handle);

    /// @return true if currently selected handle is sortable by value
    bool handleSortableByValue() const;
    /// Sort handle by value. Only applicable for rank 1 ravels
    ///@param dir direction to sort (forward/reverse, all other cases ignored)
    void sortByValue(ravel::HandleSort::Order dir);
    
    /// @} get/set description of selected handle
    std::string description() const;
    void setDescription(const std::string&);
    /// @}
    /// return the description field for handle \a handle.
    std::string handleDescription(int handle) const {return wrappedRavel.handleDescription(handle);}
    /// set the description field for \a handle
    void setHandleDescription(int handle, const std::string& description)
    {wrappedRavel.setHandleDescription(handle,description);}

    /// @{ get/set dimension attributes of selected handle, or handle at given index
    Dimension::Type dimensionType() const;
    Dimension::Type dimensionType(int) const;
    std::string dimensionUnitsFormat() const;
    std::string dimensionUnitsFormat(int) const;
    /// @throw if type does not match global dimension type
    void setDimension(Dimension::Type type,const std::string& units);
    /// @throw if type does not match global dimension type
    void setDimension(int handleIndex, Dimension::Type type,const std::string& units);
    /// @}
    
    /// get the current state of the Ravel
    ravel::RavelState getState() const {return wrappedRavel? wrappedRavel.getRavelState(): initState;}
    /// apply the \a state to the Ravel, leaving data, slicelabels etc unchanged
    /// @param preservePositions if true, do not rotate handles
    void applyState(const ravel::RavelState& state);
    void displayDelayedTooltip(float x, float y) override;
    void exportAsCSV(const std::string& filename) const;

    Units units(bool) const override;
    
    /// indicate whether icon is in editor mode or icon mode
    bool editorMode() const override {return m_editorMode;}
    void toggleEditorMode() override {m_editorMode=!m_editorMode;updateBoundingBox();}

    };

  class RavelLockGroup
  {
    static unsigned nextColour;
    unsigned m_colour;
    std::vector<std::weak_ptr<Ravel>> m_ravels;
  public:
    RavelLockGroup() {m_colour=nextColour++;}
    void addRavel(const std::weak_ptr<Ravel>& ravel);
    const std::vector<std::weak_ptr<Ravel> >& ravels() const {return m_ravels;}
    /// broadcast first ravel's state to the remainder
    void initialBroadcast();
    /// broadcast state from \a ravel to the lock group
    void broadcast(const Ravel& ravel);
    /// an identifying tag used to colour locked ravels on canvas
    unsigned colour() const {return m_colour;}
    std::vector<HandleLockInfo> handleLockInfo;

    /// checks handleLockInfo for non repeated handles, etc
    /// @throw if error
    void validateLockHandleInfo();
    
    /// populate \a handlesToLock by all handles present in the lock group
    std::vector<std::string> allLockHandles() const;

    /// return tooltips of the ravels in this lockGroup
    std::vector<std::string> ravelNames() const;

    /// return the handle descriptions of of ravel \a ravel_idx in ravels
    std::vector<std::string> handleNames(size_t ravel_idx) const;

    /// set handlesToLock to the handles in \a handles
    void setLockHandles(const std::vector<std::string>& handles);
    /// add ravel's handles to handleLockInfo, for a ravel stashed in m_ravels
    void addHandleInfo(const std::weak_ptr<Ravel>& ravel);
    void removeFromGroup(const Ravel&);
  };
}

#include "ravelWrap.cd"
#include "ravelWrap.rcd"
#include "ravelWrap.xcd"

#endif

