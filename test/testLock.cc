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

#include "lock.h"
#include "minsky.h"
#include "minsky_epilogue.h"

#include <gtest/gtest.h>

using namespace minsky;
using namespace std;

namespace
{
  struct LockTest: public ::testing::Test
  {
    Minsky minsky;
    LocalMinsky lm;
    
    LockTest(): lm(minsky) {}
  };
}

TEST_F(LockTest, Construction)
{
  Lock lock;
  // Initially should be unlocked
  EXPECT_FALSE(lock.locked());
  EXPECT_TRUE(lock.lockedState.empty());
}

TEST_F(LockTest, CopyConstructor)
{
  Lock lock1;
  Lock lock2(lock1);
  
  // Copy should also be unlocked initially
  EXPECT_FALSE(lock2.locked());
}

TEST_F(LockTest, Assignment)
{
  Lock lock1, lock2;
  lock2 = lock1;
  
  EXPECT_FALSE(lock2.locked());
}

TEST_F(LockTest, ToggleLocked)
{
  Lock lock;
  EXPECT_FALSE(lock.locked());
  EXPECT_TRUE(lock.lockedState.empty());
  EXPECT_EQ(nullptr, lock.ravelInput());

  // Toggle to locked state - throws if Ravel not available
  if (!ravelCAPI::available())
    EXPECT_THROW(lock.toggleLocked(), std::exception);
  else
    {
      Ravel ravel;
      auto startingState=ravel.getState();
      Wire wire(ravel.ports(0), lock.ports(1));
      lock.toggleLocked();
      ravel.collapseAllHandles(); // change ravel state
      EXPECT_TRUE(lock.locked());
      EXPECT_TRUE(startingState==lock.lockedState);
      EXPECT_FALSE(ravel.getState()==lock.lockedState);
      EXPECT_EQ(&ravel, lock.ravelInput());
    }
}

TEST_F(LockTest, Ports)
{
  Lock lock;
  // Lock should have ports for input/output
  EXPECT_GT(lock.portsSize(), 0);
}

TEST_F(LockTest, Units)
{
  Lock lock;
  // Test that units can be queried
  EXPECT_NO_THROW(lock.units(false));
}

TEST_F(LockTest, MultipleInstances)
{
  Lock lock1, lock2;
  
  // Multiple lock instances should be independent
  EXPECT_FALSE(lock1.locked());
  EXPECT_FALSE(lock2.locked());
  
  // Changes to one should not affect the other
  if (!ravelCAPI::available())
    EXPECT_THROW(lock1.toggleLocked(), std::exception);
  else
    {
      Ravel ravel;
      auto startingState=ravel.getState();
      Wire wire1(ravel.ports(0), lock1.ports(1));
      Wire wire2(ravel.ports(0), lock2.ports(1));
      lock1.toggleLocked();
      ravel.collapseAllHandles(); // change ravel state
      EXPECT_TRUE(lock1.locked());
      EXPECT_FALSE(lock2.locked());
      EXPECT_TRUE(startingState==lock1.lockedState);
      EXPECT_FALSE(ravel.getState()==lock1.lockedState);
      EXPECT_EQ(&ravel, lock1.ravelInput());
      EXPECT_EQ(&ravel, lock2.ravelInput());
    }      
}

TEST_F(LockTest, ApplyLockedState)
{
  Lock lock;
  
  // Should not throw even if not connected to a Ravel
  EXPECT_NO_THROW(lock.applyLockedStateToRavel());

  if (ravelCAPI::available())
    {
      Ravel ravel;
      auto startingState=ravel.getState();
      Wire wire(ravel.ports(0), lock.ports(1));
      lock.toggleLocked();
      ravel.collapseAllHandles(); // change ravel state
      EXPECT_FALSE(ravel.getState()==startingState);
      lock.applyLockedStateToRavel();
      EXPECT_TRUE(ravel.getState()==startingState);
    }
}

TEST_F(LockTest, Draw)
{
  Lock lock;
  
  // Test that lock can be drawn (uses different icons for locked/unlocked)
  cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR, nullptr));
  EXPECT_NO_THROW(lock.draw(surf.cairo()));
}

