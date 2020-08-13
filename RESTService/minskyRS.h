/*
  @copyright Steve Keen 2019
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

#ifndef MINSKYRS_H
#define MINSKYRS_H
#include "minsky.h"
#include "minsky.rcd"
#include "assetClass.h"
#include "assetClass.rcd"
#include "bookmark.rcd"
#include "canvas.rcd"
#include "CSVParser.rcd"
#include "CSVDialog.rcd"
#include "dimension.rcd"
#include "evalGodley.rcd"
#include "fontDisplay.rcd"
#include "godleyIcon.rcd"
#include "godleyTable.rcd"
#include "group.rcd"
#include "item.rcd"
#include "index.rcd"
#include "index.xcd"
#include "noteBase.rcd"
#include "operationType.rcd"
#include "panopticon.rcd"
#include "parameterSheet.rcd"
#include "parameterSheet.xcd"
#include "parVarSheet.xcd"
#include "plot.rcd"
#include "plotWidget.rcd"
#include "port.rcd"
#include "rungeKutta.rcd"
#include "selection.rcd"
#include <signature.h>
#include "signature.rcd"
#include "signature.xcd"
#include "slider.rcd"
#include "SVGItem.rcd"
#include "tensorInterface.rcd"
#include "tensorVal.rcd"
#include "variable.rcd"
#include "variableSheet.rcd"
#include "variableSheet.xcd"
#include "variableValue.rcd"
#include "variableType.rcd"
#include "variableValueRS.h"
#include "wire.rcd"

namespace classdesc
{
  void RESTProcess(RESTProcess_t& t, const string& d, minsky::Minsky& m);

  template <class T> struct tn<T,void>
  {
    static string name() {return "unknown";}
  };
  
//  template <class C, class D> struct tn<std::chrono::time_point<C,D>>
//  {
//    static string name() {return "std::chrono::time_point";}
//  };
//
//  template <class R, class P> struct tn<std::chrono::duration<R,P>>
//  {
//    static string name() {return "std::chrono::duration";}
//  };
//
//  template <> struct tn<std::istream>
//  {
//    static string name() {return "std::istream";}
//  };
//
//  template <class T> struct tn<std::initializer_list<T>>
//  {
//    static string name() {return "std::initializer_list<"+typeName<T>+">";}
//  };
}

namespace classdesc_access
{
  // mop up
  template <class T> struct access_RESTProcess: public classdesc::NullDescriptor<RESTProcess_t> {};

  template <class T> struct access_json_pack: public classdesc::NullDescriptor<json_pack_t> {};

  template <class T> struct access_json_unpack: public classdesc::NullDescriptor<json_unpack_t> {};
//  template <class C, class D> struct access_RESTProcess<std::chrono::time_point<C,D>>:
//    public classdesc::NullDescriptor<RESTProcess_t> {};
//  template <class C, class D> struct access_json_pack<std::chrono::time_point<C,D>>:
//    public classdesc::NullDescriptor<json_pack_t> {};
//  template <class C, class D> struct access_json_unpack<std::chrono::time_point<C,D>>:
//    public classdesc::NullDescriptor<json_unpack_t> {};
//  
//  template <class R, class P> struct access_RESTProcess<std::chrono::duration<R,P>>:
//    public classdesc::NullDescriptor<RESTProcess_t> {};
//  template <class R, class P> struct access_json_pack<std::chrono::duration<R,P>>:
//    public classdesc::NullDescriptor<json_pack_t> {};
//  template <class R, class P> struct access_json_unpack<std::chrono::duration<R,P>>:
//    public classdesc::NullDescriptor<json_unpack_t> {};
}



#endif
