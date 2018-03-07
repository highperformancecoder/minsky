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

#ifndef RAVELWRAP_H
#define RAVELWRAP_H

#include "operation.h"

namespace minsky 
{
  struct Ravel;
  struct DataCube;

  class RavelWrap: public ItemT<RavelWrap, DataOp>
  {
    Ravel* ravel=nullptr;
    DataCube* dataCube=nullptr;
    void noRavelSetup();
    /// position of the "move" handle, as a proportion of radius
    const double moveX=0.5, moveY=0.5, moveSz=0.1;
  public:
    RavelWrap();
    ~RavelWrap();
    const char* ravelVersion() const; ///< Ravel version string
    const char* lastErr() const;
    void draw(cairo_t* cairo) const override;
    void resize(const LassoBox&) override;
    ClickType::Type clickType(float x, float y) override;
    void onMouseDown(float x, float y);
    void onMouseUp(float x, float y);
    bool onMouseMotion(float x, float y);
    bool onMouseOver(float x, float y);
    void onMouseLeave();
    void loadFile(const char*);
    void loadDataFromSlice();
  };
}

#ifdef CLASSDESC
#pragma omit pack minsky::RavelWrap
#pragma omit unpack minsky::RavelWrap
#endif

namespace classdesc_access
{
  template <> struct access_pack<minsky::RavelWrap>: 
    public access_pack<minsky::DataOp> {};
  template <> struct access_unpack<minsky::RavelWrap>: 
    public access_unpack<minsky::DataOp> {};
}
#include "ravelWrap.cd"

#endif

