/*
 * remoteClipBoard.cpp
 *
 *  Created on: 3 mai 2013
 *      Author: tika
 */

#include <boost/variant/get.hpp>
#include "remoteClipBoard.h"

namespace minsky { namespace gui {

using namespace boost;

//
//  Utility class to check the type of the clip board contents.
//

RemoteClipBoard::RemoteClipBoard()
  : empty(true)
{
}

RemoteClipBoard::~RemoteClipBoard()
{
}

void RemoteClipBoard::insert(const DataChunk& data)
{
  contents = data;
  empty = false;
}

bool RemoteClipBoard::get(DataChunk& data)
{
  bool b = true;
//  try
//  {
//    boost::get<DataChunk&>(data);
//  }
//  catch(bad_get& e)
  {
    b = false;
  }
  return b;
}

bool RemoteClipBoard::contains(const DataChunk& data)
{
  return (!empty);
}

}} // namespace minsky::gui
