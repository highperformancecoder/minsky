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
#ifndef INTRUSIVEMAP_H
#define INTRUSIVEMAP_H
#include <TCL_obj_base.h>
#include <set>
#include <stdlib.h>

namespace minsky
{
  template <class K>
  struct KeyAssertion
  {
    KeyAssertion() {} // why?
    KeyAssertion(const K& id) {}
  };

  template <>
  struct KeyAssertion<int>
  {
    KeyAssertion() {} // why?
    KeyAssertion(int id) {assert(id>=0);}
  };
  

  template <class Key, class Val>
  struct IntrusiveWrap: public Val
  {
    const Key m_id;
  public:
    typedef const Key first_type;
    typedef Val second_type;
    // Val can access id by declaring a virtual method of this name
    Key id() const {return m_id;};
    IntrusiveWrap(const Key& id, const Val& v=Val()): Val(v), m_id(id)
    {KeyAssertion<Key>(m_id);}
    template <class K, class V>
    IntrusiveWrap(const std::pair<K,V> x):
      IntrusiveWrap(x.first,x.second) {}
    bool operator<(const IntrusiveWrap& x) const {
      return m_id<x.m_id;
    }
    IntrusiveWrap& operator=(const Val& v) {
      Val::operator=(v);
      return *this;
    }
    IntrusiveWrap& operator=(IntrusiveWrap& v) {
      Val::operator=(v);
      return *this;
    }
  };


  /**
     An intrusive map is like a std::map, except that the value type
     must include an member "id" of type Key, which holds the key
     value.
  */

  template <class Key, class Val>
  class IntrusiveMap: public std::set<IntrusiveWrap<Key, Val> >
  {
    typedef std::set<IntrusiveWrap<Key, Val> > Super; 
  public:
    typedef typename Super::const_iterator const_iterator;
    typedef typename Super::value_type value_type;
    typedef Key key_type;
    typedef Val mapped_type;

    /// track writeable access into this Map
    std::set<Key> updateAccess;

    template <class... A> explicit IntrusiveMap(A... a): Super(std::forward<A>(a)...) {}

    // iterator class allows for assignment of value portion
    struct iterator: public Super::const_iterator
    {
      iterator() {}
      iterator(const_iterator it): const_iterator(it) {}
      value_type& operator*() {
        return const_cast<value_type&>(Super::const_iterator::operator*());
      }
      value_type* operator->() {
        return &operator*();
      }
      const value_type& operator*() const {
        return const_cast<value_type&>(Super::const_iterator::operator*());
      }
      const value_type* operator->() const {
        return &operator*();
      }
   };

    iterator begin() {return iterator(Super::begin());}
    const_iterator begin() const {return Super::begin();}

    iterator end() {return iterator(Super::end());}
    const_iterator end() const {return Super::end();}

    iterator find(const Key& k) {
      updateAccess.insert(k);
      return iterator(Super::find(value_type(k)));
    }
    const_iterator find(const Key& k) const {return Super::find(value_type(k));}

    size_t count(const Key& k) const {return Super::count(value_type(k));}

    std::pair<iterator, bool> insert(const value_type& x) {
      std::pair<const_iterator, bool> p = Super::insert(x);
      return make_pair(iterator(p.first), p.second);
    }

    template <class I1, class I2>
    void insert(I1 i1, I2 i2) {Super::insert(i1,i2);}
          
    void insert(const Key& k, const Val& v)
    {insert(value_type(k,v));}

    value_type& operator[](const Key& k) {
      const_iterator it=find(k);
      if (it==end())
        it = Super::insert(value_type(k)).first;
      return const_cast<value_type&>(*it);
    }

    value_type operator[](const Key& k) const {
      const_iterator it=find(k);
      if (it==end())
        return value_type(k);
      else
        return *it;
    }

    void swap(IntrusiveMap& x) {Super::swap(x);}

    std::set<Key> keys() const {
      std::set<Key> r;
      for (const value_type& i: *this)
        r.insert(i.id());
      return r;
    }

  };

  template <class K, class V>
  struct TrackedIntrusiveMap: public IntrusiveMap<K,V>
  {
    const std::set<K>& accessLog() const {return this->updateAccess;}
    void clearAccessLog() {this->updateAccess.clear();}
    // returns true if item is in access log
    bool hasBeenAccessed(K x) {return this->updateAccess.count(x);}
    template <class... A> TrackedIntrusiveMap(A... a):
      IntrusiveMap<K,V>(std::forward<A>(a)...) {}
    
  };

}

#ifdef _CLASSDESC
#pragma omit TCL_obj minsky::IntrusiveMap
#pragma omit pack minsky::IntrusiveMap
#pragma omit unpack minsky::IntrusiveMap
#endif

namespace ecolab
{
  // note: this definition needs to exist before TCL_obj_stl.h is
  // included FOR THE FIRST TIME, otherwise it is ignored.
  template <class K, class T>
  inline void keys_of(const minsky::IntrusiveMap<K,T>& o)
  { 
    tclreturn r;
    for (const typename minsky::IntrusiveMap<K,T>::value_type&  i: o)
      r<<i.id();
  }
  // ensures IntrusiveMap is treated as map, not a set
  template <class Key, class Val>
  struct is_map<minsky::IntrusiveWrap<Key,Val> >: public true_type
  {
    static string keys() {return ".#keys";}
    static string type() {return ".@is_map";}
  };
}

namespace classdesc
{
  template <class K, class T> 
  struct is_associative_container<minsky::IntrusiveMap<K,T> >:
    public std::true_type {};
}

#include <TCL_obj_stl.h>
#include <pack_base.h>
#include <pack_stl.h>

namespace classdesc_access
{
  namespace cd=classdesc;

  template <class K, class V>
  struct access_pack<minsky::IntrusiveMap<K, V> >
  {
    template <class T>
    void operator()(cd::pack_t& b, const cd::string& d, T& a)
    {
      b<<a.size();
      for (auto i=a.begin(); i!=a.end(); ++i)
        {
          b<<i->id();
          b<<static_cast<const V&>(*i);
        }
    }
  };

  template <class K, class V>
  struct access_unpack<minsky::IntrusiveMap<K, V> >
  {
    template <class T, class U>
    void insert(T& a, const U& v) {a.insert(v);}
    template <class T, class U>
    void insert(const T& a, const U& v) {}

    template <class T>
    void operator()(cd::pack_t& b, const cd::string& d, T& a)
    {
      size_t sz;
      unpack(b,d,sz);
      a.clear();
      for (size_t i=0; i<sz; ++i)
        {
          K id;
          unpack(b,d,id);
          typename T::value_type v(id);
          unpack(b,d,static_cast<V&>(v));
          insert(a, v);
        }
    }
  };

}

#include "intrusiveMap.cd"
#endif
