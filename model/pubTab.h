/*
  @copyright Steve Keen 2023
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

#ifndef PUBTAB_H
#define PUBTAB_H

#include "item.h"
#include "pannableTab.h"
#include "publication.h"
#include "renderNativeWindow.h"

namespace minsky
{
  class PubItem: public schema3::PublicationItem
  {
  public:
    PubItem()=default;
    PubItem(const ItemPtr& item): itemRef(item) {
      if (item) editorMode=item->editorMode();
    }
    PubItem(const ItemPtr& item, const schema3::PublicationItem& state):
      schema3::PublicationItem(state), itemRef(item)  {}
    ItemPtr itemRef;
    /// given (x,y) in PubTab, returns coordinates within item. Nb: must be wrapped by EnsureEditorMode 
    Point itemCoords(float x, float y) const;
  };

  struct PubTabBase: public RenderNativeWindow
  {
    float offsx=0, offsy=0;
    float m_zoomFactor=1;
    bool panning=false;
  };
  
  class PubTab: public PannableTab<PubTabBase>
  {
    bool redraw(int x0, int y0, int width, int height) override;
    CLASSDESC_ACCESS(PubTab);
    PubItem* item=nullptr; // weak reference for moving items
    PubItem* m_getItemAt(float x, float y);
    void zoomTranslate(float& x, float& y);
    bool rotating=false;
    ClickType::Type clickType=ClickType::outside;
    float dx,dy;      ///< relative offset between mouse and item origin when moving 
    Exclude<Point> rotateOrigin; ///< reference position for rotating
    LassoBox lasso;
  public:
    PubTab()=default;
    PubTab(const char* name): name(name) {}
    PubTab(const std::string& name): name(name) {}
    std::string name; ///< name of this publication tab
    std::vector<PubItem> items; ///< list of wrapped items and annotations
    void addNote(const std::string& note, float x, float y);
    /// remove this from the global minsky object
    /// calling this method will invalidate any references to this, and potentially call its destructor
    void removeSelf();
    void removeItemAt(float x, float y);
    void rotateItemAt(float x, float y);
    void toggleEditorModeAt(float x, float y) {
      zoomTranslate(x,y);
      if (auto i=m_getItemAt(x,y))
        i->editorMode=!i->editorMode;
    }
    bool getItemAt(float x, float y) override {
      zoomTranslate(x,y);
      return m_getItemAt(x,y);
    }
    void mouseDown(float x, float y) override;
    void controlMouseDown(float x, float y) override {panning=true; PannableTab<PubTabBase>::mouseDown(x,y);}
    void mouseUp(float x, float y) override;
    void mouseMove(float x, float y) override;
    bool keyPress(const EventInterface::KeyPressArgs&) override;
    bool hasScrollBars() const override {return true;}
  };
}

#include "pubTab.cd"
#endif
