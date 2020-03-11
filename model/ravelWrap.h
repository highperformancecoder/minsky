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
#include "ravelState.h"

namespace minsky 
{
  using namespace civita;
  class RavelLockGroup;
  
  class Ravel: public ItemT<Ravel, Operation<OperationType::ravel>>
  {
  public:
    struct RavelImpl;
    struct DataCube;

    CLASSDESC_ACCESS(Ravel);
  private:
    typedef RavelState::HandleState HandleState;
    void noRavelSetup();
    /// position of the "move" handle, as a proportion of radius
    const double moveX=0.5, moveY=0.5, moveSz=0.1;
    std::string m_filename;
    std::string explanation; // explanation of Ravel bits displayed as tooltip
    HandleState::HandleSort previousOrder=HandleState::forward;
    
    /// used entirely to defer persisted state data until after first
    /// load from a variable
    RavelState initState;
    
    friend struct SchemaHelper;

    std::vector<string> allSliceLabelsImpl(int axis, HandleState::HandleSort) const;

  public:
    Ravel();
    ~Ravel();
    // copy operations needed for clone, but not really used for now
    // define them as empty operations to prevent double frees if accidentally used
    void operator=(const Ravel&) {}
    Ravel(const Ravel&) {}
    
    // These members have been made public to expose them to the minskyTensorOp class.
    Exclude<RavelImpl*> ravel=nullptr;
    Exclude<DataCube*> dataCube=nullptr;    

    /// local override of axis dimensionality
    Dimensions axisDimensions;

    /// group of ravels that move syncronously
    std::shared_ptr<RavelLockGroup> lockGroup;
    void leaveLockGroup();
    void broadcastStateToLockGroup() const;
    
    /// true to indicate mouse hovering over border
    bool onBorder=false; 
    
    string ravelVersion() const; ///< Ravel version string
    const char* lastErr() const;
    void draw(cairo_t* cairo) const override;
    void resize(const LassoBox&) override;
    double radius() const;
    ClickType::Type clickType(float x, float y) override;
    void onMouseDown(float x, float y);
    void onMouseUp(float x, float y);
    bool onMouseMotion(float x, float y);
    bool onMouseOver(float x, float y);
    void onMouseLeave();
    void loadFile(const std::string&);
    const string& filename() const {return m_filename;}
    void loadDataFromSlice(ITensorVal&) const;
    void loadDataCubeFromVariable(const ITensor&);
    unsigned maxRank() const;
    unsigned rank() const;
    void setRank(unsigned);
    void adjustSlicer(int); ///< adjust currently selected handle's slicer
    bool handleArrows(int dir, bool modifier) override;

    // return selected handle, or -1 if none
    int selectedHandle() const;
    
    /// enable/disable calipers on currently selected handle
    bool displayFilterCaliper() const;
    bool setDisplayFilterCaliper(bool);
    bool toggleDisplayFilterCaliper()
    {return setDisplayFilterCaliper(!displayFilterCaliper());}
    /// @}

    /// returns all slice labels along the selected handle, in specified order
    std::vector<string> allSliceLabels() const;
    /// returns just the picked slice labels along the handle
    std::vector<string> pickedSliceLabels() const;
    /// pick (selected) \a pick labels
    void pickSliceLabels(int axis, const std::vector<string>& pick);
    
    /// @{
    /// the handle sorting order for currently selected handle
    HandleState::HandleSort sortOrder() const;
    HandleState::HandleSort setSortOrder(HandleState::HandleSort);
    /// @}

    /// @} get/set description of selected handle
    string description() const;
    void setDescription(const string&);
    /// @}

    /// @{ get/set selected handle dimension attributes
    Dimension::Type dimensionType() const;
    std::string dimensionUnitsFormat() const;
    /// @throw if type does not match global dimension type
    void setDimension(Dimension::Type type,const std::string& units);
    /// @}
    
    /// get the current state of the Ravel
    RavelState getState() const;
    /// apply the \a state to the Ravel, leaving data, slicelabels etc unchanged
    /// @param preservePositions if true, do not rotate handles
    void applyState(const RavelState&);
    void displayDelayedTooltip(float x, float y) override;
    void exportAsCSV(const std::string& filename) const;

    Units units(bool) const override;
    
  };

  class RavelLockGroup
  {
    static unsigned nextColour;
    unsigned m_colour;
  public:
    RavelLockGroup() {m_colour=nextColour++;}
    // an identifyin tag used to colour locked ravels on canvas
    unsigned colour() const {return m_colour;}
    std::vector<std::weak_ptr<Ravel>> ravels;
    void removeFromGroup(const Ravel&);
  };
}

#include "ravelWrap.cd"
#include "ravelWrap.xcd"

#endif

