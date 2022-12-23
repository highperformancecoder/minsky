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

namespace minsky 
{
  using namespace civita;
  class RavelLockGroup;
  
  class Ravel: public ItemT<Ravel, Operation<OperationType::ravel>>, public classdesc::Exclude<ravel::Ravel>
  {
  public:

    CLASSDESC_ACCESS(Ravel);
  private:
    using HandleState=ravel::HandleState;
    /// position of the "move" handle, as a proportion of radius
    const double moveX=0.5, moveY=0.5, moveSz=0.1;
    //std::string m_filename;
    std::string explanation; // explanation of Ravel bits displayed as tooltip
    ravel::HandleSort::Order previousOrder=ravel::HandleSort::forward;
    
    /// used entirely to defer persisted state data until after first
    /// load from a variable
    ravel::RavelState initState;
    
    friend struct SchemaHelper;

    std::vector<std::string> allSliceLabelsImpl(int axis, ravel::HandleSort::Order) const;

    /// return hypercube corresponding to the current Ravel state. \a data returns a pointer to the current data slice

  public:
    Ravel();
    // copy operations needed for clone, but not really used for now
    // define them as empty operations to prevent double frees if accidentally used
    void operator=(const Ravel&) {}
    Ravel(const Ravel&) {}

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
    void onMouseLeave() override {ravel::Ravel::onMouseLeave();}
    /// return hypercube corresponding to the current Ravel state
    Hypercube hypercube() const;
    void populateHypercube(const Hypercube&);
    /// @return input rank
    unsigned maxRank() const;
    /// adjust output dimensions to first \a r handles
    void setRank(unsigned r);
    void adjustSlicer(int); ///< adjust currently selected handle's slicer
    bool onKeyPress(int, const std::string&, int) override; 

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
      for (size_t i=0; i<numHandles(); ++i)
        r.push_back(handleDescription(i));
      return r;
    }
    
    /// dimension details associated with handle 
    Dimension dimension(int handle) const;
    
    /// @{
    /// the handle sorting order for currently selected handle
    ravel::HandleSort::Order sortOrder() const;
    ravel::HandleSort::Order setSortOrder(ravel::HandleSort::Order);
    /// @}

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

    /// @{ get/set selected handle dimension attributes
    Dimension::Type dimensionType() const;
    std::string dimensionUnitsFormat() const;
    /// @throw if type does not match global dimension type
    void setDimension(Dimension::Type type,const std::string& units);
    /// @}
    
    /// get the current state of the Ravel
    ravel::RavelState getState() const {return getRavelState();}
    /// apply the \a state to the Ravel, leaving data, slicelabels etc unchanged
    /// @param preservePositions if true, do not rotate handles
    void applyState(const ravel::RavelState& state);
    void displayDelayedTooltip(float x, float y) override;
    void exportAsCSV(const std::string& filename) const;

    Units units(bool) const override;

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

#endif

