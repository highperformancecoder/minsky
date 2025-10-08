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

#include "minsky.h"
#include "minsky_epilogue.h"

#undef True
#include <gtest/gtest.h>
using namespace minsky;
using namespace std;

namespace
{
}


class RavelLockGroupTest : public Canvas, public ::testing::Test
{
protected:
  Minsky minsky; // provides model and selection
  LocalMinsky lminsky{minsky};
public:
  RavelLockGroupTest() : Canvas(make_shared<Group>()) {}
};

TEST_F(RavelLockGroupTest, LockGroup)
{
  auto a=make_shared<Ravel>(), b=make_shared<Ravel>(),
    c=make_shared<Ravel>(), d=make_shared<Ravel>();
  selection.items={a,b,c};
  lockRavelsInSelection();
  EXPECT_TRUE(a->lockGroup);
  EXPECT_TRUE(b->lockGroup);
  EXPECT_TRUE(c->lockGroup);
  EXPECT_TRUE(a->lockGroup==b->lockGroup && a->lockGroup==c->lockGroup);
  auto lockGroup=a->lockGroup;
  EXPECT_EQ(3, lockGroup->ravels().size());
  typedef weak_ptr<Ravel> W;
  EXPECT_TRUE(find_if(lockGroup->ravels().begin(), lockGroup->ravels().end(),
                [&](W w){return w.lock()==a;}) != lockGroup->ravels().end());
  EXPECT_TRUE(find_if(lockGroup->ravels().begin(), lockGroup->ravels().end(),
                [&](W w){return w.lock()==b;}) != lockGroup->ravels().end());
  EXPECT_TRUE(find_if(lockGroup->ravels().begin(), lockGroup->ravels().end(),
                [&](W w){return w.lock()==c;}) != lockGroup->ravels().end());

  a->leaveLockGroup();
  EXPECT_FALSE(a->lockGroup);
  EXPECT_EQ(2, lockGroup->ravels().size());
  EXPECT_TRUE(find_if(lockGroup->ravels().begin(), lockGroup->ravels().end(),
                [&](W w){return w.lock()==a;}) == lockGroup->ravels().end());
  EXPECT_TRUE(find_if(lockGroup->ravels().begin(), lockGroup->ravels().end(),
                [&](W w){return w.lock()==b;}) != lockGroup->ravels().end());
  EXPECT_TRUE(find_if(lockGroup->ravels().begin(), lockGroup->ravels().end(),
                [&](W w){return w.lock()==c;}) != lockGroup->ravels().end());

  // create a different group
  selection.items={a,d};
  lockRavelsInSelection();
  auto lockGroup1=a->lockGroup;
  EXPECT_TRUE(a->lockGroup);
  EXPECT_TRUE(lockGroup!=a->lockGroup);
  EXPECT_TRUE(a->lockGroup==d->lockGroup);
  EXPECT_EQ(2, lockGroup1->ravels().size());

  // now try combining a and b into a group. This should create a 3rd lockGroup, and dissolve the first two
  selection.items={a,b};
  lockRavelsInSelection();
  EXPECT_TRUE(a->lockGroup==b->lockGroup);
  // check that the previous groups would deleted were it not for us holding a reference here
  EXPECT_EQ(1, lockGroup.use_count());
  EXPECT_EQ(1, lockGroup1.use_count());
  EXPECT_FALSE(c->lockGroup);
  EXPECT_FALSE(d->lockGroup);

  lockGroup=a->lockGroup;
  // now add c back in, should be same as a & b
  selection.items={a,c};
  lockRavelsInSelection();
  EXPECT_TRUE(a->lockGroup==c->lockGroup);
  EXPECT_TRUE(b->lockGroup==c->lockGroup);
  EXPECT_FALSE(d->lockGroup);
}

TEST_F(RavelLockGroupTest, JoinLockGroupBroadcast)
{
  // Skip test if ravels not available
  if (!minsky.ravelAvailable())
    {
      cerr << "Ravel not available, skipping test" << endl;
      return;
    }
  auto a=make_shared<Ravel>(), b=make_shared<Ravel>();
  
  // Add ravels to model so joinLockGroup can find them
  minsky.model->addItem(a);
  minsky.model->addItem(b);
  
  // Create a lock group with ravel a and set some state
  a->lockGroup.reset(new RavelLockGroup);
  a->lockGroup->addRavel(a);
  auto colour=a->lockGroup->colour();
  
  // Set calipers on ravel a to have some distinguishable state
  auto aState=a->getState();
  ASSERT_FALSE(aState.handleStates.empty());
  aState.handleStates[0].displayFilterCaliper=true;
  aState.handleStates[0].minLabel="1991";
  aState.handleStates[0].maxLabel="1991";
  a->applyState(aState);

  // Verify ravel b has different state
  ASSERT_FALSE(b->getState().handleStates.empty());
  EXPECT_FALSE(b->getState().handleStates[0].displayFilterCaliper);
  EXPECT_NE("1991",b->getState().handleStates[0].minLabel);
  EXPECT_NE("1991",b->getState().handleStates[0].maxLabel);
  
  // Now have b join the lock group using joinLockGroup method
  b->joinLockGroup(colour);
  
  // Verify both ravels are in the same lock group
  EXPECT_EQ(a->lockGroup, b->lockGroup);
  EXPECT_EQ(2, a->lockGroup->ravels().size());
  
  // Verify state was broadcast - check if b got a's caliper settings
  EXPECT_TRUE(b->getState()==aState);
}
