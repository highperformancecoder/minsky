#ifndef TCLACCESSOR_H
#define TCLACCESSOR_H
#include <TCL_obj_base.h>
#include <string>

namespace minsky
{
  /**
     use this like 
     class Item: public TCLAccessor<Item,double>
     {
        ...
        Item(): TCLAcessor<Item,double>("rotation",(Getter)&Item::rotation,(Setter)&Item::rotation) {}
        double rotation() const;
        double rotation(double);
     };

     @param T subclass of this we're inserting the accessor into
     @param V type of the accessor
     @param N used to distinguish base classes when multiple accessors are deployed:

     class Item: public TCLAccessor<Item,double,0>, public TCLAccessor<Item,double,1>
     {
        ...
        Item(): 
          TCLAcessor<Item,double,0>("rotation",
             (TCLAcessor<Item,double,0>::Getter)&Item::rotation,
             (TCLAcessor<Item,double,0>::Setter)&Item::rotation), 
          TCLAcessor<Item,double,1>("scale",
             (TCLAcessor<Item,double,1>::Getter)&scale::scale,
             (TCLAcessor<Item,double,1>Setter)&Item::scale) 
          {}
        ...
     };
  */

  template <class T, class V, int N=0>
  class TCLAccessor: ecolab::cmd_data
  {
  public:
    using Getter=V (T::*)() const;
    using Setter=V (T::*)(V);
    TCLAccessor(const std::string& name, Getter g, Setter s):
      name(name), _self(static_cast<T&>(*this)), g(g), s(s) {}
    TCLAccessor(const TCLAccessor& x):
      name(x.name), _self(static_cast<T&>(*this)), g(x.g), s(x.s) {}
    // assignment need do nothing, everything is set up for this
    TCLAccessor& operator=(const TCLAccessor& x) {return *this;}
    void proc(int argc, Tcl_Obj *const argv[]) {
      ecolab::tclreturn r;
      if (argc<=1)
        r << (_self.*g)();
      else
        r << (_self.*s)(ecolab::TCL_args(argc, argv));
    }
    void proc(int argc, Tcl_Obj *const argv[]) const {
      ecolab::tclreturn()<<(_self.*g)();
    }
    void proc(int, const char **) {}  
    std::string name; ///< name of accessor seen by TCL
  private:
    T& _self; ///< reference to the outer object accessor are working on
    Getter g;   ///< getter - self.*g() should return a V
    Setter s;   ///< setter - self.*s(v) should set the attribute to v and return a V
  };
}

#ifdef _CLASSDESC
#pragma omit TCL_obj minsky::TCLAccessor
#endif

namespace classdesc_access
{
  template <class T, class V, int N>
  struct access_TCL_obj<minsky::TCLAccessor<T,V,N>>
  {
    template <class U>
    void operator()(classdesc::TCL_obj_t&, const std::string& d, U& a)
    {
      Tcl_CreateObjCommand(ecolab::interp(),(d+"."+a.name).c_str(),ecolab::TCL_oproc,(ClientData)&a,nullptr);
     }
  };
}

#include "tclAccessor.cd"
#endif
