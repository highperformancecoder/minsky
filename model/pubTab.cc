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

#include "lasso.h"
#include "minsky.h"
#include "cairoItems.h"
#include "pubTab.h"
#include "publication.rcd"
#include "pubTab.xcd"
#include "pubTab.rcd"
#include "pannableTab.rcd"
#include "pannableTab.xcd"
#include "minsky_epilogue.h"
using namespace ecolab::cairo;

namespace minsky
{
  namespace {
    struct EnsureEditorMode
    {
      PubItem& item;
      bool editorModeToggled;
      bool variableDisplay=false, buttonDisplay=false;
      LassoBox origBox;
      float origIWidth, origIHeight;
      double origRotation;
      EnsureEditorMode(PubItem& item):
        item(item), editorModeToggled(item.editorMode!=item.itemRef->editorMode()),
        origIWidth(item.itemRef? item.itemRef->iWidth(): 0),
        origIHeight(item.itemRef? item.itemRef->iHeight(): 0),
        origRotation(item.itemRef? item.itemRef->rotation(): 0)
      {
        if (!item.itemRef) return;
        if (auto g=item.itemRef->godleyIconCast())
          {
            if ((variableDisplay=g->variableDisplay()))
              g->toggleVariableDisplay();
            buttonDisplay=g->editor.drawButtons;
            g->editor.disableButtons();
          }
        if (editorModeToggled)
          item.itemRef->toggleEditorMode();
          
        item.itemRef->iWidth(item.zoomX*origIWidth);
        item.itemRef->iHeight(item.zoomY*origIHeight);
        item.itemRef->rotation(item.rotation);
      }
      ~EnsureEditorMode()
      {
        if (!item.itemRef) return;
        if (editorModeToggled)
          item.itemRef->toggleEditorMode();
        if (auto g=item.itemRef->godleyIconCast())
          {
            if (variableDisplay)
              g->toggleVariableDisplay();
            if (buttonDisplay)
              g->editor.enableButtons();
          }
        item.itemRef->iWidth(origIWidth);
        item.itemRef->iHeight(origIHeight);
        item.itemRef->rotation(origRotation);
     }
    };
  }

  Point PubItem::itemCoords(float x, float y) const
  {
    if (!itemRef) return {0,0};
    return {x-this->x+itemRef->x(), y-this->y+itemRef->y()};
  }

  void PubTab::addNote(const std::string& note, float x, float y)
  {
    items.emplace_back(std::make_shared<Item>());
    items.back().itemRef->detailedText(note);
    items.back().x=x-offsx;
    items.back().y=y-offsy;
    minsky().pushHistory();
    requestRedraw();
  }

  
  void PubTab::removeSelf()
  {
    auto& publicationTabs=minsky::minsky().publicationTabs;
    for (auto i=publicationTabs.begin(); i!=publicationTabs.end(); ++i)
      if (this==&*i) {
        publicationTabs.erase(i);
        minsky().pushHistory();
        return;
      }
  }

  void PubTab::removeItemAt(float x, float y)
  {
    zoomTranslate(x,y);
    if (auto item=m_getItemAt(x,y))
        for (auto i=items.begin(); i!=items.end(); ++i)
          if (&*i==item)
            {
              items.erase(i);
              minsky().pushHistory();
              requestRedraw();
              return;
            }
  }

  void PubTab::rotateItemAt(float x, float y)
  {
    zoomTranslate(x,y);
    item=m_getItemAt(x,y);
    if (item) {
      rotateOrigin=Point{x,y};
      rotating=true;
    }
  }
 
  bool PubTab::redraw(int x0, int y0, int width, int height)
  {
    if (!surface.get()) {
      return false;
    }
    auto cairo=surface->cairo();
    const CairoSave cs(cairo);
    cairo_translate(cairo, offsx, offsy);
    cairo_scale(cairo, m_zoomFactor, m_zoomFactor);
    cairo_set_line_width(cairo, 1);
    // stash toplevel zoomfactor and set it to 1.
    auto zf=cminsky().model->relZoom;
    cminsky().model->relZoom=1;
    for (auto& i: items)
      {
        const CairoSave cs(cairo);
        cairo_translate(cairo, i.x, i.y);
        try
          {
            const EnsureEditorMode ensureEditorMode(i);
            i.itemRef->draw(cairo);
          }
        catch (...) {}
      }
    if (clickType==ClickType::onResize)
      {
        cairo_rectangle(cairo,std::min(lasso.x0,lasso.x1), std::min(lasso.y0,lasso.y1),
                        abs(lasso.x0-lasso.x1), abs(lasso.y0-lasso.y1));
        cairo_stroke(cairo);
      }
    cminsky().model->relZoom=zf; // restore zoomFactor
    return !items.empty() || clickType!=ClickType::outside;
  }

  PubItem* PubTab::m_getItemAt(float x, float y) 
  {
    for (auto& i: items)
      {
        const EnsureEditorMode e(i);
        if (i.itemRef && i.itemRef->contains(i.itemCoords(x,y)))
          return &i;
      }
    return nullptr;
  }
  
  void PubTab::zoomTranslate(float& x, float& y)
  {
    x-=offsx; y-=offsy;
    auto scale=1.f/m_zoomFactor;
    x*=scale; y*=scale;
  }

  void PubTab::mouseDown(float x, float y)
  {
    zoomTranslate(x,y);
    item=m_getItemAt(x,y);
    if (item)
      {
        const EnsureEditorMode e(*item);
        auto p=item->itemCoords(x,y);
        clickType=item->itemRef->clickType(p.x(),p.y());
        dx=dy=0;
        switch (clickType)
          {
          case ClickType::onResize:
            {
              auto scale=item->itemRef->zoomFactor();
              lasso.x0=x>item->x? x-item->itemRef->width()*scale: x+item->itemRef->width()*scale;
              lasso.y0=y>item->y? y-item->itemRef->height()*scale: y+item->itemRef->height()*scale;
              lasso.x1=x;
              lasso.y1=y;
            }
            break;
          case ClickType::inItem:
            item->itemRef->onMouseDown(p.x(),p.y());
            break;
          default:
            dx=item->x-x;
            dy=item->y-y;
            break;
          }
      }
  }
  
  void PubTab::mouseUp(float x, float y)
  {
    if (panning)
      {
        PannableTab<PubTabBase>::mouseUp(x,y);
        panning=false;
        return;
      }
    mouseMove(x,y);
    if (item)
      switch (clickType)
        {
        case ClickType::onResize:
          item->zoomX=abs(lasso.x1-lasso.x0)/(item->itemRef->width());
          item->zoomY=abs(lasso.y1-lasso.y0)/(item->itemRef->height());
          item->x=0.5*(lasso.x0+lasso.x1);
          item->y=0.5*(lasso.y0+lasso.y1);
          break;
        case ClickType::inItem:
          {
            zoomTranslate(x,y);
            const EnsureEditorMode e(*item);
            auto p=item->itemCoords(x,y);
            item->itemRef->onMouseUp(p.x(),p.y());
            minsky().requestReset();
          }
          break;
        default: break;
        }
    minsky().pushHistory();
    item=nullptr;
    clickType=ClickType::outside;
    rotating=false;
  }
  
  void PubTab::mouseMove(float x, float y)
  {
    zoomTranslate(x,y);
    if (panning)
      {
        PannableTab<PubTabBase>::mouseMove(x,y);
        return;
      }
    for (auto& i: items) i.itemRef->mouseFocus=false;
    if (item)
      {
        if (rotating)
          {
            const EnsureEditorMode e(*item);
            item->itemRef->rotate(Point{x,y},rotateOrigin);
            item->rotation=item->itemRef->rotation();
          }
        else
          switch (clickType)
            {
            case ClickType::onResize:
              lasso.x1=x;
              lasso.y1=y;
              break;
            case ClickType::inItem:
              {
                const EnsureEditorMode e(*item);
                auto p=item->itemCoords(x,y);
                item->itemRef->mouseFocus=true;
                if (item->itemRef->onMouseMotion(p.x(),p.y()))
                  requestRedraw();
              }
              break;
            default:
              item->x=x+dx;
              item->y=y+dy;
              break;
            }
        requestRedraw();
      }
    else
      // indicate mouse focus
      if (auto i=m_getItemAt(x,y))
        {
          i->itemRef->mouseFocus=true;
          const EnsureEditorMode e(*i);
          auto p=i->itemCoords(x,y);
          i->itemRef->onMouseOver(p.x(),p.y());
          requestRedraw();
        }
  }

  bool PubTab::keyPress(const KeyPressArgs& args)
  {
    float x=args.x, y=args.y;
    zoomTranslate(x,y);
    if (auto item=m_getItemAt(x,y))
      {
        EnsureEditorMode em(*item);
        if (item->itemRef->onKeyPress(args.keySym,args.utf8,args.state))
          {
            requestRedraw();
            return true;
          }
      }
    return false;
  }
}
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(schema3::PublicationItem);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PubTab);
CLASSDESC_ACCESS_EXPLICIT_INSTANTIATION(minsky::PubItem);
