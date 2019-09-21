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
#ifndef RESTPROCESS_EPILOGUE_H
#define RESTPROCESS_EPILOGUE_H
#include "RESTProcess_base.h"
#include "signature.h"
#include <cairoSurfaceImage.h>
#include <polyPackBase.h>
#include <accessor.h>

namespace classdesc_access
{
  namespace classdesc=cd;

  template <class T>
  struct access_RESTProcess<cd::Exclude<T>>: public cd::NullDescriptor<cd::RESTProcess_t> {};

  template <class T>
  struct access_RESTProcess<cd::shared_ptr<T>>
  {
    template <class U>
    void operator()(cd::RESTProcess_t& repo, const std::string& d, U& a)
    {repo.add(d, new cd::RESTProcessPtr<U>(a));}
  };

  template <class T>
  struct access_RESTProcess<cd::weak_ptr<T>>
  {
    template <class U>
    void operator()(cd::RESTProcess_t& repo, const std::string& d, U& a)
    {repo.add(d, new cd::RESTProcessPtr<U>(a));}
  };

  template <>
  struct access_RESTProcess<cd::string>
  {
    template <class U>
    void operator()(cd::RESTProcess_t& repo, const std::string& d, U& a)
    {repo.add(d, new cd::RESTProcessObject<U>(a));}
  };

  template <class F, class S>
  struct access_RESTProcess<std::pair<F,S>>
  {
    template <class U>
    void operator()(cd::RESTProcess_t& repo, const std::string& d, U& a)
    {
      RESTProcess(repo,d+".first", a.first);
      RESTProcess(repo,d+".second", a.second);
    }
  };

  template <>
  struct access_RESTProcess<ecolab::CairoSurface>:
    public cd::NullDescriptor<cd::RESTProcess_t> {};

  
  // TODO implement this properly to duck type an object 
  template <class T, class G, class S>
  struct access_RESTProcess<ecolab::Accessor<T,G,S>>:
    public cd::NullDescriptor<cd::RESTProcess_t> {};

  template <>
  struct access_RESTProcess<cd::PolyPackBase>: 
    public cd::NullDescriptor<cd::RESTProcess_t> {};

  template <class T> struct access_json_pack<ecolab::Accessor<T>>
  {
    template <class U>
    void operator()(cd::json_pack_t& j, const cd::string& d, U& a)
    {
      j<<a();
    }
  };
  template <class T> struct access_json_unpack<ecolab::Accessor<T>>
  {
    template <class U>
    void operator()(cd::json_pack_t& j, const cd::string& d, U& a)
    {
      // Unpack j into a T and arrange to call the setter with it.
      // Nothing happens if T is not default constructible, or the
      // target object is const
      cd::functional::evalVoid<U&,T,cd::json_pack_t>(a, j);
    }
  };
  
}

namespace classdesc
{

  template <> struct tn<cairo_t>
  {
    static std::string name()
    {return "cairo_t";}
  };

  template <> struct tn<cairo_surface_t>
  {
    static std::string name()
    {return "cairo_surface_t";}
  };

  
  template <class T> 
  json_pack_t RESTProcessObject<T>::signature() const
  {
    json_pack_t r;
    std::vector<minsky::Signature> signature
      {{typeName<T>(),{}},{typeName<T>(),{typeName<T>()}}};
    return r<<signature;
  }

  template <class T> 
  json_pack_t RESTProcessPtr<T>::signature() const
  {
    json_pack_t r;
    auto tn=typeName<typename T::element_type>();
    std::vector<minsky::Signature> signature{{tn,{}},{tn,{tn}}};
    return r<<signature;
  }

  template <class T> 
  json_pack_t RESTProcessSequence<T>::signature() const
  {
    json_pack_t r;
    auto tn=typeName<T>();
    std::vector<minsky::Signature> signature{{tn,{}},{tn,{tn}}};
    return r<<signature;
  }

  template <class T> 
  json_pack_t RESTProcessAssociativeContainer<T>::signature() const
  {
    json_pack_t r;
    auto tn=typeName<T>();
    std::vector<minsky::Signature> signature{{tn,{}},{tn,{tn}}};
    return r<<signature;
  }

  
  
  template <class F, int N=functional::Arity<F>::value >
  struct Args: public std::vector<string>
  {
    Args(): std::vector<string>(Args<F,N-1>()) {
      push_back(typeName<typename functional::Arg<F,N>::type>());
    }
  };

  template <class F> struct Args<F,0>: public std::vector<string> {};

  
  template <class F>
  json_pack_t RESTProcessBase::functionSignature() const
  {
    json_pack_t r;
    return r<<minsky::Signature{typeName<typename functional::Return<F>::T>(), Args<F>()};
  }

  template <class T>
  typename enable_if<is_classdescGenerated<T>, void>::T
  RESTProcessp(RESTProcess_t& repo, string d, T& obj)
  {
    repo.add(d, new RESTProcessObject<T>(obj));
    classdesc_access::access_RESTProcess<typename remove_const<T>::type>()(repo,d,obj);
  }

}

#endif
