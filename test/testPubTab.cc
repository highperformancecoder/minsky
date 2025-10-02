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
#undef True
#include <gtest/gtest.h>
using namespace minsky;

namespace
{
  class MinskyFixture : public Minsky, public ::testing::Test
  {
  public:
    LocalMinsky lm;
    MinskyFixture(): lm(*this)
    {
    }
  };
}

class PubTabTest : public PubTab, public ::testing::Test
{
};

TEST_F(MinskyFixture,removeSelf)
  {
    assert(!publicationTabs.empty());
    auto numPubTabs=publicationTabs.size();
    publicationTabs[0].removeSelf();
    EXPECT_EQ(numPubTabs-1,publicationTabs.size());
  }

  TEST_F(PubTabTest,addNote)
    {
      addNote("hello",100,100);
      EXPECT_EQ(1,items.size());
      EXPECT_EQ("hello",items[0].itemRef->detailedText());
      EXPECT_TRUE(getItemAt(100,100));
      EXPECT_EQ(100,items[0].x);
      EXPECT_EQ(100,items[0].y);
    }

  TEST_F(PubTabTest,removeItem)
    {
      addNote("hello",100,100);
      EXPECT_EQ(1,items.size());
      removeItemAt(100,100);
      EXPECT_EQ(0,items.size());
    }

    TEST_F(PubTabTest,rotateItemAt)
    {
      addNote("hello",100,100);
      EXPECT_EQ(1,items.size());
      rotateItemAt(100,100);
      mouseUp(150,150);
      EXPECT_NEAR(45,items[0].rotation,0.1);
    }
    
    TEST_F(PubTabTest,toggleEditorModeAt)
    {
      addNote("hello",100,100);
      EXPECT_EQ(1,items.size());
      toggleEditorModeAt(100,100);
      EXPECT_TRUE(items[0].editorMode);
    }

    TEST_F(PubTabTest,zoomPan)
    {
      addNote("hello",100,100);
      EXPECT_EQ(1,items.size());
      moveTo(100,100);
      zoom(100,100,2);
      EXPECT_FALSE(getItemAt(100,100));
      EXPECT_TRUE(getItemAt(300,300));
      EXPECT_EQ(2,zoomFactor());
      vector expected{100,100};
      for (size_t i=0; i<2; ++i)
        EXPECT_EQ(expected[i], position()[i]);
    }

    TEST_F(PubTabTest,mousePan)
      {
        addNote("hello",100,100);
        EXPECT_EQ(1,items.size());
        controlMouseDown(0,0);
        mouseUp(100,100);
        EXPECT_FALSE(getItemAt(100,100));
        EXPECT_TRUE(getItemAt(200,200));
      }

    TEST_F(PubTabTest,moveItem)
      {
        addNote("hello",100,100);
        EXPECT_EQ(1,items.size());
        mouseDown(100,100);
        mouseUp(200,150);
        EXPECT_EQ(200,items[0].x);
        EXPECT_EQ(150,items[0].y);
      }
    TEST_F(PubTabTest,resizeItem)
      {
        addNote("hello",100,100);
        EXPECT_EQ(1,items.size());
        auto w=0.5*items[0].itemRef->width();
        auto h=0.5*items[0].itemRef->height();
        auto x=items[0].x;
        auto y=items[0].y;
        mouseDown(x+w,y+h);
        mouseUp(x+2*w,y+3*h);
        EXPECT_NEAR(1.5,items[0].zoomX,0.1);
        EXPECT_NEAR(2,items[0].zoomY,0.1);
      }
    TEST_F(PubTabTest,mouseFocus)
      {
        addNote("hello",100,100);
        EXPECT_EQ(1,items.size());
        mouseMove(100,100);
        EXPECT_TRUE(items[0].itemRef->mouseFocus);
      }
     TEST_F(MinskyFixture,moveSlider)
       {
         VariablePtr var(VariableBase::parameter, "foobar");
         // this member should always be initialised, but occasionally is not, causing the test to fail. Why?
         var->value(0);
         var->adjustSliderBounds();
         model->addItem(var);
         var->updateBoundingBox();
         auto& tab=publicationTabs[0];
         tab.items.emplace_back(var);
         auto& item=tab.items[0];
         auto x=var->x()+item.x, y=var->y()+item.y-0.5f*var->height();
         EXPECT_TRUE(var->clickType(x-item.x,y-item.y)==ClickType::inItem);
         tab.mouseDown(x,y);
         tab.mouseUp(x+0.5*var->width(),y);
         EXPECT_NEAR(var->sliderMax(),var->value(),0.1);
       }
     TEST_F(MinskyFixture,redraw)
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

     TEST_F(MinskyFixture,addNewTab)
       {
         EXPECT_EQ(1,publicationTabs.size());
         addNewPublicationTab("foo");
         EXPECT_EQ(2,publicationTabs.size());
         EXPECT_EQ("foo",publicationTabs.back().name);
       }
     TEST_F(MinskyFixture,addItem)
       {
         canvas.addOperation(OperationType::time);
         canvas.mouseUp(100,100);
         canvas.item=model->items[0];
         addCanvasItemToPublicationTab(0);
         EXPECT_EQ(1,publicationTabs.size());
         EXPECT_EQ(1,publicationTabs[0].items.size());
         EXPECT_EQ("Operation:time",publicationTabs[0].items[0].itemRef->classType());
       }
