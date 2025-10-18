/*
  @copyright Steve Keen 2025
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

#include "schemaHelper.h"
#include <gtest/gtest.h>
#include <vector>

using namespace minsky;
using namespace classdesc;

TEST(ZStream, EncodeDecodeRoundTrip)
{
  // Create some test data
  pack_t originalData(400);
  for (size_t i = 0; i < 100; ++i)
    originalData.append(Basic_Type<char>('A' + (i % 26)));

  // Encode (compress) the data
  CDATA encoded = encode(originalData);
  EXPECT_GT(encoded.size(), 0);

  // Decode (decompress) the data
  pack_t decodedData = decode(encoded);
  
  // Verify the round-trip
  EXPECT_EQ(originalData.size(), decodedData.size());
  EXPECT_EQ(0, memcmp(originalData.data(), decodedData.data(), originalData.size()));
}

TEST(ZStream, EncodeDecodeEmpty)
{
  // Test with empty data
  pack_t emptyData(0);
  
  CDATA encoded = encode(emptyData);
  pack_t decodedData = decode(encoded);
  
  EXPECT_EQ(0, decodedData.size());
}

TEST(ZStream, EncodeDecodeLargeData)
{
  // Test with larger data that will require buffer resizing during inflation
  pack_t largeData(4000);
  for (size_t i = 0; i < 1000; ++i)
    largeData.append(Basic_Type<int>(i));

  CDATA encoded = encode(largeData);
  pack_t decodedData = decode(encoded);
  
  EXPECT_EQ(largeData.size(), decodedData.size());
  EXPECT_EQ(0, memcmp(largeData.data(), decodedData.data(), largeData.size()));
}

TEST(ZStream, MultipleInflateOperations)
{
  // Test multiple inflate operations to ensure no memory leaks
  pack_t testData(200);
  for (size_t i = 0; i < 50; ++i)
    testData.append(Basic_Type<int>(i));

  CDATA encoded = encode(testData);
  
  // Perform multiple decode operations
  for (int iteration = 0; iteration < 10; ++iteration)
  {
    pack_t decodedData = decode(encoded);
    EXPECT_EQ(testData.size(), decodedData.size());
  }
}
