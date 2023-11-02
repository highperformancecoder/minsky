#ifndef TYPESCRIPT_API_BASE_H
#define TYPESCRIPT_API_BASE_H
#include "classdesc.h"
#include "function.h"
#include "stringKeyMap.h"
#include <algorithm>
#include <numeric>

namespace minsky
{
  template <class T, class B> struct ItemT;
}

namespace classdesc
{
  template< class... > using void_t = void;
  
  template <class T> struct is_map: public false_type {};
  template <class K, class V> struct is_map<std::map<K,V>>: public true_type {};

  template <class T> struct is_excluded: public false_type {};
  template <class T> struct is_excluded<Exclude<T>>: public true_type {};

  template <class T> struct is_pair: public false_type {};
  template <class F, class S> struct is_pair<std::pair<F,S>>: public true_type {};

  
  template <class T, class = void> struct is_iterator: public std::false_type {};
#ifndef MAC_OSX_TK
  template <class T, class U> struct is_iterator<__gnu_cxx::__normal_iterator<T,U>>: public true_type {};
#endif
  
  // general is_iterator class https://stackoverflow.com/questions/4335962/how-to-check-if-a-template-parameter-is-an-iterator-type-or-not doesn't seem to work
//  template <class T>
//  struct is_iterator<T, void_t<typename std::iterator_traits<T>::iterator_category>>:
//    public std::true_type {};

  template <class T> struct is_itemT: public false_type {};
  template <class T, class B> struct is_itemT<minsky::ItemT<T,B>>: public true_type {};

  template <class T> string typescriptType();

  // typescriptTypep is for "specialisations" using type_traits
  template <class T>
  inline typename enable_if<
    And<
      is_arithmetic<T>,
      And<
        Not<is_const<T>>,
        Not<is_same<T,bool>>
        >
      >, string>::T
  typescriptTypep() {return "number";}

  template <class T>
  typename enable_if<is_same<T,bool>,string>::T
  typescriptTypep() {return "boolean";}
    
  template <class T>
  inline typename enable_if<is_function<T>, string>::T
  typescriptTypep() {return "minsky__dummy";}
    
  /// typescript name corresponding to C++ type T
  template <class T>
  typename enable_if<
    And<
      And<
        is_class<T>,
        And<
          Not<is_reference<T>>,
          Not<is_const<T>>
          >
        >,
      And<
        And<
          Not<is_string<T>>,
          And<
            Not<is_pointer<T>>,
            Not<is_itemT<T>>
            >
          >,
        And<
          And<
            Not<is_container<T>>,
            Not<is_iterator<T>>
            >,
          And<
            Not<is_smart_ptr<T>>,
            Not<is_pair<T>>
            >
          >
        >
      >, std::string>::T
  typescriptTypep() {
    auto cppName=classdesc::typeName<T>();
    // strip leading "::"
    if (cppName.substr(0,2)=="::") cppName=cppName.substr(2);
    
    // minsky namespaced types to global namespace
    auto n=cppName.find("minsky::");
    if (n==0)
      {
        int numColons=0;
        for (char c: cppName)
          numColons += c==':';
        if (numColons==2) // not in a sub namespace
          return cppName.substr(strlen("minsky::"));
      }

    std::replace(cppName.begin(), cppName.end(), ':', '_');
    cppName.erase(std::remove(cppName.begin(), cppName.end(),' '), cppName.end());
    return cppName;
  }

  // enums look like strings to TS
  template <class T>
  typename enable_if<is_enum<T>, string>::T
  typescriptTypep() {return "string";}
  
  template <class T>
  typename enable_if<is_reference<T>, string>::T
  typescriptTypep() {return typescriptType<typename remove_reference<T>::type>();}

  template <class T>
  typename enable_if<is_pointer<T>, string>::T
  typescriptTypep() {return typescriptType<typename remove_pointer<T>::type>();}

  template <class T>
  typename enable_if<is_const<T>,std::string>::T
  typescriptTypep() {return typescriptType<typename remove_const<T>::type>();}

  template <class T>
  typename enable_if<And<is_iterator<T>,Not<is_pointer<T>>>,std::string>::T
  typescriptTypep() {return "minsky__dummy";}

  template <class T>
  typename enable_if<
    And<
      And<
        is_sequence<T>,
        Not<is_const<T>>
        >,        
      And<
        Not<is_arithmetic<typename T::value_type>>,
        Not<is_string<typename T::value_type>>
        >
      >,std::string>::T
  
  typescriptTypep() {return "Sequence<"+typescriptType<typename T::value_type>()+">";}
  
  template <class T>
  typename enable_if<
    And<
      is_sequence<T>,
      And<
        Not<is_const<T>>,
        Or<is_arithmetic<typename T::value_type>, is_string<typename T::value_type>>
        >
      >,std::string>::T
  typescriptTypep() {return typescriptType<typename T::value_type>()+"[]";}
  
  template <class T>
  typename enable_if<
    And<
      And<
        is_associative_container<T>,
        Not<is_map<T>>
        >,
      Not<is_const<T>>
      >,std::string>::T
  typescriptTypep() {return "Container<"+typescriptType<typename T::value_type>()+">";}
  
  template <class T>
  typename enable_if<
    And<
      is_map<T>,
      Not<is_const<T>>
      >,std::string>::T
  typescriptTypep() {
    auto k=typescriptType<typename T::key_type>();
    auto v=typescriptType<typename T::mapped_type>();
    return "Map<"+k+","+v+">";}
  
  template <class T>
  typename enable_if<
    And<
      is_smart_ptr<T>,
      Not<is_const<T>>
      >,std::string>::T
  typescriptTypep() {return typescriptType<typename T::element_type>();}
    
  template <class T>
  typename enable_if<
    And<
      is_pair<T>,
      Not<is_const<T>>
      >,std::string>::T
  typescriptTypep() {return "Pair<"+typescriptType<typename T::first_type>()+","
      +typescriptType<typename T::second_type>()+">";}

  // don't process ItemT because of clone return type.
  template <class T>
  typename enable_if<is_itemT<T>, std::string>::T
  typescriptTypep() {return "Item";}
  
  template <class T> string typescriptType() {return typescriptTypep<T>();}

  template <> inline string typescriptType<string>() {return "string";}
  template <> inline string typescriptType<void>() {return "void";}

}

namespace classdesc
{
  namespace typescriptAPI_ns
  {
    using classdesc::typescriptType;
    using namespace classdesc::functional;

    // arrange for sequences to be passed as javascript arrays or objects
    template <class T>
    typename enable_if<
      And<
        Not<is_container<T>>,
        Or<Not<is_class<T>>,is_string<T>>
      >,string>::T
    parameterType() {return typescriptType<T>();}
    
    template <class T>
    typename enable_if<
      And<
        is_class<T>,
        And<Not<is_container<T>>, Not<is_string<T>>>
      >,string>::T
    parameterType() {return "object";}

    template <class T>
    typename enable_if<
      And<
        is_container<T>,
        Not<is_map<T>>
        >, string>::T
    parameterType() {return parameterType<typename T::value_type>()+"[]";}
    
    template <class T>
    typename enable_if<
      And<
        is_container<T>,
        is_map<T>
        >, string>::T
    parameterType() {return "object";}
    
    template <class T>
    typename enable_if<
      And<
        is_container<T>,
        is_same<T, StringKeyMap<T>>
        >, string>::T
    parameterType() {return "object";}
  
    struct Property
    {
      std::string type;
      std::string construction;
      Property(const std::string& type={}, const std::string& construction={}):
        type(type), construction(construction) {}
    };

    struct ArgDetail
    {
      std::string name, type;
      ArgDetail(const std::string& name={}, const std::string& type={}):
        name(name), type(type) {}
      bool operator==(const ArgDetail& x) const {return type==x.type;}
      bool operator!=(const ArgDetail& x) const {return !operator==(x);}
    };
    
    struct Method
    {
      std::string returnType;
      std::vector<ArgDetail> args;
      template <class M, int N>
      struct AddArgs
      {
        static void addArgs(std::vector<ArgDetail>& args)
        {
          constexpr int i=Arity<M>::value-N+1;
          args.emplace_back("a"+std::to_string(i), parameterType<typename Arg<M,i>::T>());
          AddArgs<M,N-1>::addArgs(args);
        }
      };
      // recursion stop
      template <class M> struct AddArgs<M, 0> {
        static void addArgs(std::vector<ArgDetail>&) {}
      };
      template <class M> void addArgs() {AddArgs<M,Arity<M>::value>::addArgs(args);}
    };
    
    struct ClassType
    {
      std::string super;
      std::string valueType; // used for container to instantiate proxies around elements
      std::map<std::string,Property> properties;
      std::map<std::string, Method> methods;
    };
  }
}



namespace classdesc
{
  struct typescriptAPI_t: public std::map<std::string, typescriptAPI_ns::ClassType>
  {
    template <class T> void addClass();
    template <class T, class Base> void addSubclass();
  };

  using typescriptAPI_ns::typescriptType;
  using typescriptAPI_ns::parameterType;
  
  template <class C, class B>
  typename enable_if<Not<is_map<B>>, void>::T
  typescriptAPI(typescriptAPI_t&,const std::string&);

  template <class C, class B>
  typename enable_if<is_map<B>, void>::T
  typescriptAPI(typescriptAPI_t& t,const std::string&)
  {
    if (typescriptType<C>().substr(0,4)!="Map<")
      t[typescriptType<C>()].super="Map<"+typescriptType<typename B::key_type>()+","+
        typescriptType<typename B::mapped_type>()+">";
  }

  template <class C, class B, class T>
  typename enable_if<is_arithmetic<T>, void>::T
  typescriptAPI_type(typescriptAPI_t& t, const std::string& d, T(B::*))
  {
    t[typescriptType<C>()].methods.emplace(tail(d), typescriptAPI_ns::Method{parameterType<T>(), {{"...args",typescriptType<T>()+"[]"}}});
  }

  
  
  template <class C, class B, class T>
  typename enable_if<is_arithmetic<T>, void>::T
  typescriptAPI_type(typescriptAPI_t& t, const std::string& d, T*)
  {
    t[typescriptType<C>()].methods.emplace(tail(d), typescriptAPI_ns::Method{typescriptType<T>(), {{"...args",typescriptType<T>()+"[]"}}});
  }

  template <class VT>
  std::string construct(const std::string& container, const std::string name)
  {
    string tn=typescriptType<VT>();
    return "new "+container+"(this.$prefix()+'."+name+"'"+
      ((is_string<VT>::value || is_enum<VT>::value || is_arithmetic<VT>::value)?"":","+tn)+");";
  }
  
  template <class C, class B, class T>
  typename enable_if<is_sequence<T>, void>::T
  typescriptAPI_type(typescriptAPI_t& t, const std::string& d, T*)
  {
    auto tn=typescriptType<typename T::value_type>();
    t[typescriptType<C>()].properties.emplace
      (tail(d), typescriptAPI_ns::Property
       {"Sequence<"+tn+">", construct<typename T::value_type>("Sequence<"+tn+">", tail(d))});
  }

  template <class C, class B, class T>
  typename enable_if<is_sequence<T>, void>::T
  typescriptAPI_type(typescriptAPI_t& t, const std::string& d, T (B::*))
  {typescriptAPI_type<C,B>(t,d,(T*){});}

  template <class C, class B, class T>
  typename enable_if<Or<is_same<typename remove_const<T>::type,std::string>,
                        is_enum<typename remove_const<T>::type>>, void>::T
  typescriptAPI_type(typescriptAPI_t& t, const std::string& d, T(B::*))
  {
    t[typescriptType<C>()].methods.emplace
      (tail(d), typescriptAPI_ns::Method{"string",{{"...args","string[]"}}});
  }
  
  template <class C, class B, class T>
  typename enable_if<Or<is_same<typename remove_const<T>::type,std::string>,
                        is_enum<typename remove_const<T>::type>>, void>::T
  typescriptAPI_type(typescriptAPI_t& t, const std::string& d, T*)
  {
    t[typescriptType<C>()].methods.emplace(tail(d), typescriptAPI_ns::Method{"string", {{"...args","string[]"}}});
  }

  template <class C, class B>
  void typescriptAPI_type(typescriptAPI_t& t, const std::string& d, const char**)
  {typescriptAPI_type<C,B>(t,d,static_cast<const std::string*>(nullptr));}
  
  template <class C, class B, class T>
  void typescriptAPI_type(typescriptAPI_t& t, const std::string& d, std::set<T>(B::*))
  {
    string tn=typescriptType<T>();
    t[typescriptType<C>()].properties.emplace
      (tail(d), typescriptAPI_ns::Property{"Container<"+tn+">", construct<T>("Container<"+tn+">", tail(d))});
  }
  
  template <class C, class B, class K, class V>
  void typescriptAPI_type(typescriptAPI_t& t, const std::string& d, std::map<K,V>(B::*))
  {
    string k=typescriptType<K>();
    string v=typescriptType<V>();
    t[typescriptType<C>()].properties.emplace
      (tail(d), typescriptAPI_ns::Property
       {"Map<"+k+","+v+">", construct<V>("Map<"+k+","+v+">",tail(d))});
  }
  
  template <class C, class B, class V>
  void typescriptAPI_type(typescriptAPI_t& t, const std::string& d, StringKeyMap<V>(B::*))
  {
    string v=typescriptType<V>();
    t[typescriptType<C>()].properties.emplace
      (tail(d), typescriptAPI_ns::Property{"Map<string,"+v+">",construct<V>("Map<string,"+v+">",tail(d))});
  }
  
  template <class C, class B, class M>
  typename enable_if<functional::is_function<M>, void>::T
  typescriptAPI_type(typescriptAPI_t& t, const std::string& d, M)
  {
    auto& methods=t[typescriptType<C>()].methods;
    auto iter=methods.find(typeName<C>());
    if (iter==methods.end())
      {
        auto res=methods.emplace
          (tail(d),
           typescriptAPI_ns::Method{parameterType<typename functional::Return<M>::T>()});
        if (res.second) // first occurance of this method
          {
            typescriptAPI_ns::Method& m=res.first->second;
            m.addArgs<M>();
          }
        else // method seen before
          {
            auto oldArgs=std::move(res.first->second.args);
            typescriptAPI_ns::Method& m=res.first->second;
            m.addArgs<M>();
            auto newArgs=res.first->second.args;
            // check if arguments are same, in which case leave the method call
            if (oldArgs!=res.first->second.args) // overloaded method
              {
                res.first->second.args={{"...args","any[]"}}; // set up general case or arbitrary args
                //check if all arguments are the same (ie just numbers of arguments differ)
                auto firstArg=!oldArgs.empty()? oldArgs[0]: res.first->second.args[0];
                if (firstArg.type=="any[]") return;
                if (firstArg.name=="...args") firstArg.type.erase(firstArg.type.length()-2); //strip off []
                for (auto& i: oldArgs)
                  if (i!=firstArg) return;
                for (auto& i: newArgs)
                  if (i!=firstArg) return;
                res.first->second.args={{"...args",firstArg.type+"[]"}}; //restrict to common type
              }
          }
      }
  }

  template <class C, class B, class T>
  typename enable_if<
    And<
      And<
        is_class<T>,
        Not<is_same<typename remove_const<T>::type,std::string>>
        >,
      And<
        Not<is_sequence<T>>,
        Not<is_excluded<T>>
        >
      >, void>::T
  typescriptAPI_type(typescriptAPI_t& t, const std::string& d, T(B::*))
  {
    typescriptAPI<T,T>(t,d);
    t[typescriptType<C>()].properties.emplace(tail(d), typescriptType<T>());
  }

  template <class C, class B, class T>
  typename enable_if<is_excluded<T>, void>::T
  typescriptAPI_type(typescriptAPI_t& t, const std::string& d, T(B::*)) {}

  
  template <class C, class B, class T>
  typename enable_if<
    And<
      And<
        is_class<T>,
        Not<is_same<typename remove_const<T>::type,std::string>>
        >,
      Not<is_sequence<T>>
      >, void>::T
  typescriptAPI_type(typescriptAPI_t& t, const std::string& d, T*)
  {
    typescriptAPI<T,T>(t,d);
    t[typescriptType<C>()].properties.emplace(tail(d), typescriptType<T>());
  }


  
  template <class T> void typescriptAPI(typescriptAPI_t&,const std::string&,T,...) {} //not used

  inline void typescriptAPI_onbase(typescriptAPI_t&,...) {} //not used

  template <class C, class B>
  void typescriptAPI_type(typescriptAPI_t&,const std::string&,is_constructor,...) {} //not used
  
  template <class C, class B, class T>
  void typescriptAPI_type(typescriptAPI_t& t,const std::string& d,is_const_static,T a)
  {typescriptAPI_type<C,B>(t,d,a);} 

}

namespace classdesc_access
{
  template <class T> struct access_typescriptAPI;

  template <class T>  struct access_typescriptAPI<std::shared_ptr<T>>
  {
    template <class U>
    void type(classdesc::typescriptAPI_t& t,const std::string& d)
    {classdesc::typescriptAPI<U,T>(t,d);}
  };
}

using classdesc::typescriptAPI;
using classdesc::typescriptAPI_onbase;
#endif
