/*
 * remoteClipBoard.h
 *
 *  Created on: 3 mai 2013
 *      Author: tika
 */

#ifndef REMOTECLIPBOARD_H_
#define REMOTECLIPBOARD_H_

#include <boost/variant.hpp>
#include "iClipBoard.h"

namespace minsky { namespace gui {

/**
 *  Simple OS-independent clip board support.
 *  Should also be used for remote client/server operations.
 */
class RemoteClipBoard: public IClipBoard
{
private:
  /**
   *  Clip board contents type. all contents types need registered here.
   *
   *  @note Actual data must provide its own garbage management.
   */
  typedef boost::variant<DataChunk>  Contents;

    //
    //  Data.
    //
private:
  Contents contents;  ///< clip board contents.
  bool     empty;     ///< very simplistic contents checking, since there is only one type.

public:
  /**
   *  Constructor.
   */
  RemoteClipBoard();

  /**
   *  Destructor.
   */
  virtual ~RemoteClipBoard();

  /**
   * Stores a copy of a data chunk to the clip board.
   * @param data data to copy to the clip board.
   */
  virtual void insert(const DataChunk& data);

  /**
   * Gets data from the clip board.
   * @param data on return will contain the contents of the clip board.
   * @returns true if data was present on the clip board.
   */
  virtual bool get(DataChunk& data);

  /**
   *  Indicates whether the clip board contains data of a certain type
   */
  virtual bool contains(const DataChunk& data);
};

}} // namespace minsky::gui

#endif /* REMOTECLIPBOARD_H_ */
