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

namespace classdesc_access
{
  namespace classdesc=cd;

  template <class T>
  struct access_RESTProcess<cd::Exclude<T>>: public cd::NullDescriptor<cd::RESTProcess_t> {};

  template <class T>
  struct access_RESTProcess<cd::shared_ptr<T>>
  {
    void operator()(cd::RESTProcess_t& repo, const std::string& d, cd::shared_ptr<T>& a)
    {repo.add(d, new cd::RESTProcessPtr<cd::shared_ptr<T>>(a));}
  };

  template <class T>
  struct access_RESTProcess<cd::weak_ptr<T>>
  {
    void operator()(cd::RESTProcess_t& repo, const std::string& d, cd::weak_ptr<T>& a)
    {repo.add(d, new cd::RESTProcessPtr<cd::weak_ptr<T>>(a));}
  };

  template <>
  struct access_RESTProcess<cd::string>
  {
    void operator()(cd::RESTProcess_t& repo, const std::string& d, cd::string& a)
    {repo.add(d, new cd::RESTProcessObject<cd::string>(a));}
  };

  template <>
  struct access_RESTProcess<ecolab::CairoSurface>:
    public cd::NullDescriptor<cd::RESTProcess_t> {};
  
}

namespace classdesc
{

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

  template <class F>
  json_pack_t RESTProcessBase::functionSignature() const
  {
    json_pack_t r;
    return r<<minsky::Signature
      {typeName<typename functional::Return<F>::T>(),{/*TODO*/}};
  }

  template <class T>
  typename enable_if<is_classdescGenerated<T>, void>::T
  RESTProcess(RESTProcess_t& repo, string d, T& obj)
  {
    repo.add(d, new RESTProcessObject<T>(obj));
    classdesc_access::access_RESTProcess<T>()(repo,d,obj);
  }
}

#endif
