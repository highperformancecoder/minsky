/*
  @copyright Steve Keen 2024
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
#include "minsky.h"
#include "minsky_epilogue.h"
#include <UnitTest++/UnitTest++.h>
using namespace minsky;

namespace
{
  struct MinskyFixture: public Minsky
  {
    LocalMinsky lm;
    MinskyFixture(): lm(*this)
    {
    }
  };
}

SUITE(PubTab)
{
  TEST_FIXTURE(MinskyFixture,removeSelf)
    {
      assert(!publicationTabs.empty());
      auto numPubTabs=publicationTabs.size();
      publicationTabs[0].removeSelf();
      CHECK_EQUAL(numPubTabs-1,publicationTabs.size());
    }

    TEST_FIXTURE(PubTab,addNote)
      {
        addNote("hello",100,100);
        CHECK_EQUAL(1,items.size());
        CHECK_EQUAL("hello",items[0].itemRef->detailedText());
        CHECK(getItemAt(100,100));
        CHECK_EQUAL(100,items[0].x);
        CHECK_EQUAL(100,items[0].y);
      }

    TEST_FIXTURE(PubTab,removeItem)
      {
        addNote("hello",100,100);
        CHECK_EQUAL(1,items.size());
        removeItemAt(100,100);
        CHECK_EQUAL(0,items.size());
      }

      TEST_FIXTURE(PubTab,rotateItemAt)
      {
        addNote("hello",100,100);
        CHECK_EQUAL(1,items.size());
        rotateItemAt(100,100);
        mouseUp(150,150);
        CHECK_CLOSE(45,items[0].rotation,0.1);
      }
      
      TEST_FIXTURE(PubTab,toggleEditorModeAt)
      {
        addNote("hello",100,100);
        CHECK_EQUAL(1,items.size());
        toggleEditorModeAt(100,100);
        CHECK(items[0].editorMode);
      }

      TEST_FIXTURE(PubTab,zoomPan)
      {
        addNote("hello",100,100);
        CHECK_EQUAL(1,items.size());
        moveTo(100,100);
        zoom(100,100,2);
        CHECK(!getItemAt(100,100));
        CHECK(getItemAt(300,300));
        CHECK_EQUAL(2,zoomFactor());
        vector expected{100,100};
        CHECK_ARRAY_EQUAL(expected,position(),2);
      }

      TEST_FIXTURE(PubTab,mousePan)
        {
          addNote("hello",100,100);
          CHECK_EQUAL(1,items.size());
          controlMouseDown(0,0);
          mouseUp(100,100);
          CHECK(!getItemAt(100,100));
          CHECK(getItemAt(200,200));
        }

      TEST_FIXTURE(PubTab,moveItem)
        {
          addNote("hello",100,100);
          CHECK_EQUAL(1,items.size());
          mouseDown(100,100);
          mouseUp(200,150);
          CHECK_EQUAL(200,items[0].x);
          CHECK_EQUAL(150,items[0].y);
        }
      TEST_FIXTURE(PubTab,resizeItem)
        {
          addNote("hello",100,100);
          CHECK_EQUAL(1,items.size());
          auto w=0.5*items[0].itemRef->width();
          auto h=0.5*items[0].itemRef->height();
          auto x=items[0].x;
          auto y=items[0].y;
          mouseDown(x+w,y+h);
          mouseUp(x+2*w,y+3*h);
          CHECK_CLOSE(1.5,items[0].zoomX,0.1);
          CHECK_CLOSE(2,items[0].zoomY,0.1);
        }
      TEST_FIXTURE(PubTab,mouseFocus)
        {
          addNote("hello",100,100);
          CHECK_EQUAL(1,items.size());
          mouseMove(100,100);
          CHECK(items[0].itemRef->mouseFocus);
        }
       TEST_FIXTURE(PubTab,moveSlider)
         {
           VariablePtr var(VariableBase::parameter, "foobar");
           items.emplace_back(var);
           var->initSliderBounds();
           auto x=var->x()+items[0].x, y=var->y()+items[0].y-0.5f*var->height();
           CHECK(var->clickType(x-items[0].x,y-items[0].y)==ClickType::onSlider);
           mouseDown(x,y);
           mouseUp(x+0.5*var->width(),y);
           CHECK_CLOSE(var->sliderMax,var->value(),0.1);
         }
       TEST_FIXTURE(MinskyFixture,redraw)
         {
           setGodleyIconResource("bank.svg");
           load("1Free.mky");
           // send the Godley table to the pub tab
           for (auto& i: model->items)
             if (auto g=dynamic_cast<GodleyIcon*>(i.get()))
               {
                 publicationTabs[0].items.emplace_back(i);
                 break;
               }
           if (publicationTabs[0].items[0].itemRef->editorMode())
             publicationTabs[0].items[0].itemRef->toggleEditorMode();
           publicationTabs[0].renderToSVG("1FreePubNeither.svg");
           publicationTabs[0].items[0].editorMode=true;
           publicationTabs[0].renderToSVG("1FreeWiringNotPubEditor.svg");
           publicationTabs[0].items[0].itemRef->toggleEditorMode();
           publicationTabs[0].renderToSVG("1FreeWiringEditorPubEditor.svg");
           publicationTabs[0].items[0].editorMode=false;
           publicationTabs[0].renderToSVG("1FreeWiringEditorPubNot.svg");
         }

       TEST_FIXTURE(MinskyFixture,addNewTab)
         {
           CHECK_EQUAL(1,publicationTabs.size());
           addNewPublicationTab("foo");
           CHECK_EQUAL(2,publicationTabs.size());
           CHECK_EQUAL("foo",publicationTabs.back().name);
         }
       TEST_FIXTURE(MinskyFixture,addItem)
         {
           canvas.addOperation(OperationType::time);
           canvas.mouseUp(100,100);
           canvas.item=model->items[0];
           addCanvasItemToPublicationTab(0);
           CHECK_EQUAL(1,publicationTabs.size());
           CHECK_EQUAL(1,publicationTabs[0].items.size());
           CHECK_EQUAL("Operation:time",publicationTabs[0].items[0].itemRef->classType());
         }

}
