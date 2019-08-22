#ifndef RESTPROCESS_EPILOGUE_H
#define RESTPROCESS_EPILOGUE_H
#include "RESTProcess_base.h"

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
  typename enable_if<is_classdescGenerated<T>, void>::T
  RESTProcess(RESTProcess_t& repo, string d, T& obj)
  {
    repo.add(d, new RESTProcessObject<T>(obj));
    classdesc_access::access_RESTProcess<T>()(repo,d,obj);
  }
}

#endif
