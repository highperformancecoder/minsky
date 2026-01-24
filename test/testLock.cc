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
  
  // Toggle to locked state - throws if Ravel not available
  if (!ravelCAPI::available())
    EXPECT_THROW(lock.toggleLocked(), std::exception);
  else
    lock.toggleLocked();

  // Note: actual behavior depends on whether lock is connected to a Ravel
  // If not connected, toggle may not change state or may set empty state
}

TEST_F(LockTest, LockedState)
{
  Lock lock;
  
  // Test that locked() reflects the state of lockedState
  EXPECT_TRUE(lock.lockedState.empty());
  EXPECT_FALSE(lock.locked());
}

TEST_F(LockTest, RavelInput)
{
  Lock lock;
  // Initially not connected to any Ravel
  EXPECT_EQ(nullptr, lock.ravelInput());
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
  Units u = lock.units(false);
  // Lock passes through units from connected Ravel or returns dimensionless
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
    lock1.toggleLocked();

  EXPECT_EQ(lock2.locked(), false);
}

TEST_F(LockTest, ApplyLockedState)
{
  Lock lock;
  
  // Should not throw even if not connected to a Ravel
  EXPECT_NO_THROW(lock.applyLockedStateToRavel());
}

TEST_F(LockTest, Draw)
{
  Lock lock;
  
  // Test that lock can be drawn (uses different icons for locked/unlocked)
  cairo::Surface surf(cairo_recording_surface_create(CAIRO_CONTENT_COLOR, nullptr));
  EXPECT_NO_THROW(lock.draw(surf.cairo()));
}

TEST_F(LockTest, StaticIcons)
{
  // Toasted that static SVG icons are initialized
  EXPECT_NO_THROW({
    auto& locked = Lock::lockedIcon;
    auto& unlocked = Lock::unlockedIcon;
    // Icons should be usable
  });
}
