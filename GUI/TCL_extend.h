/*
  @copyright Steve Keen 2013
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

#ifndef TCL_EXTEND_H
#define TCL_EXTEND_H
#include "variable.h"

#include "TCL_obj_base.h"


namespace minsky
{
  /// A hook class allowing custom TCL methods to be added to a class
  template <class T> class TclExtendBase 
  {
  protected:
    T& ref;
    friend struct classdesc_access::access_TCL_obj<TclExtendBase>;
  public:
    TclExtendBase(T& ref): ref(ref) {}
  };

  template <class T> struct TclExtend: public TclExtendBase<T>
  {
    TclExtend(T& x): TclExtendBase<T>(x) {}
  };

  
  // specialisations to handle particular classes in the Minsky project
  template <> struct TclExtend<std::shared_ptr<minsky::VariableBase> >: 
    public TclExtendBase<std::shared_ptr<minsky::VariableBase> >
  {
    TclExtend(std::shared_ptr<VariableBase>& ref): TclExtendBase<std::shared_ptr<VariableBase>>(ref) {}
    std::string name(TCL_args args) {
      if (args.count)
        return ref->name(args);
      else
        return ref->name();
    }
    std::string init(TCL_args args) {
      if (args.count)
        return ref->init(args);
      else
        return ref->init();
    }
    double value(TCL_args args) {
      if (args.count)
        return ref->value(args);
      else
        return ref->value();
    }
  };

  template <> struct TclExtend<minsky::Wire>: 
    public TclExtendBase<minsky::Wire>
  {
    TclExtend(Wire& ref): TclExtendBase<Wire>(ref) {}
    ecolab::array<float> coords(TCL_args args) {
      if (args.count)
        {
          ecolab::array<float> coords;
          args>>coords;
          return ref.coords(coords);
        }
      return ref.coords();
    }

  };

  template <> struct TclExtend<std::shared_ptr<minsky::IntOp> >: 
    public TclExtendBase<std::shared_ptr<minsky::IntOp> >
  {
    TclExtend(std::shared_ptr<IntOp>& ref): TclExtendBase<std::shared_ptr<IntOp>>(ref) {}
    string description(TCL_args args=TCL_args()) {
      if (args.count) {ref->description(args);}
      return ref->description();
    }
  };
}

#ifdef _CLASSDESC
#pragma omit pack minsky::TclExtendBase
#pragma omit unpack minsky::TclExtendBase
#pragma omit TCL_obj minsky::TclExtendBase
#pragma omit pack minsky::TclExtend
#pragma omit unpack minsky::TclExtend
#endif

namespace classdesc_access
{
  namespace cd=classdesc;
  template <class T> struct access_pack<minsky::TclExtendBase<T> >:
    public cd::NullDescriptor<cd::pack_t> {};
  template <class T> struct access_unpack<minsky::TclExtendBase<T> >:
    public cd::NullDescriptor<cd::pack_t> {};

   template <class T> struct access_pack<minsky::TclExtend<T> >:
    public cd::NullDescriptor<cd::pack_t> {};
  template <class T> struct access_unpack<minsky::TclExtend<T> >:
    public cd::NullDescriptor<cd::pack_t> {};

  template <class T> struct access_TCL_obj<minsky::TclExtendBase<T> >
  {
    template <class U>
    void operator()(TCL_obj_t& t,const std::string& d, U& a)
    {::TCL_obj(t,d,a.ref);}
  };
}

#include "TCL_extend.cd"
#endif
