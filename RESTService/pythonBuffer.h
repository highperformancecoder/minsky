/*
  @copyright Steve Keen 2023
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

/* A buffer implementation for marshalling function arguments from python onjects
*/

#ifndef CLASSDESC_SIMPLE_BUFFER_H
#define CLASSDESC_SIMPLE_BUFFER_H

#include <json_pack_base.h>
#include "signature.h"
#include <deque>
#include <Python.h>

namespace classdesc
{
  /// @{ utility python object constructors
  inline PyObject* newPyObject(const bool& x) {if (x) Py_RETURN_TRUE; Py_RETURN_FALSE;}
  template <class T> inline typename enable_if<And<is_integral<T>, Not<is_same<T,bool>>>, PyObject*>::T
  newPyObject(const T& x) {return PyLong_FromLong(x);}
  template <class T> inline typename enable_if<is_floating_point<T>,PyObject*>::T
  newPyObject(const T& x) {return PyFloat_FromDouble(x);}
  inline PyObject* newPyObject(const string& x) {return PyUnicode_FromString(x.c_str());}
  inline PyObject* newPyObject(const char* x) {return PyUnicode_FromString(x);}

  // objects
  inline PyObject* newPyObjectJson(const json5_parser::mValue& j) {
    switch (j.type())
        {
        case json5_parser::obj_type:
          {
            auto pyObject=PyDict_New();
            for (auto& i: j.get_obj())
              PyDict_SetItemString(pyObject, i.first.c_str(), newPyObjectJson(i.second));
            return pyObject;
          }
        case json5_parser::array_type:
          {
            auto arr=j.get_array();
            auto pyObject=PyList_New(arr.size());
            for (size_t i=0; i<arr.size(); ++i)
              PyList_SetItem(pyObject, i, newPyObjectJson(arr[i]));
            return pyObject;
          }
        case json5_parser::str_type:
          return newPyObject(j.get_str());
        case json5_parser::bool_type:
          return newPyObject(j.get_bool());
        case json5_parser::int_type:
          return newPyObject(j.get_int64());
        case json5_parser::real_type:
          return newPyObject(j.get_real());
        case json5_parser::null_type:
          Py_RETURN_NONE;
        }
    assert(false); // silly compiler cannot figure out this is unreachable code
    Py_RETURN_NONE;
  }

  inline PyObject* newPyObject(const json_pack_t& j) {return newPyObjectJson(j);}

  /// sequences
  template <class T> inline typename enable_if<is_sequence<T>, PyObject*>::T
  newPyObject(const T& x) {
    auto pyObject=PyList_New(x.size());
    for (size_t i=0; i<x.size(); ++i)
      PyList_SetItem(pyObject, i, newPyObject(x[i]));
    return pyObject;
  }

  class PythonBuffer;
  inline PyObject* newPyObject(const PythonBuffer& x);

  template <class T> inline typename enable_if<
    And<
      Not<is_arithmetic<T>>,
      Not<is_sequence<T>>
      >, PyObject*>::T
  newPyObject(const T& x) {
    json_pack_t tmp; // go via JSON serialisation for arbitrary objects
    tmp<<x;
    return newPyObjectJson(tmp);
  }
  /// @}

  // container for handling PyObject lifetimes in an RAII fashion
  class PyObjectRef
  {
    PyObject* ref=nullptr;
  public:
    PyObjectRef()=default;
    PyObjectRef(PyObject* ref): ref(ref) {
      assert(!ref || Py_REFCNT(ref));
    }
    ~PyObjectRef() {Py_XDECREF(ref);}
    PyObjectRef(const PyObjectRef& x)=delete;
    PyObjectRef& operator=(const PyObjectRef& x)=delete;
    PyObjectRef(PyObjectRef&& x): ref(x.ref) {
      x.ref=nullptr;
      assert(!ref || Py_REFCNT(ref));
    }
    PyObjectRef& operator=(PyObjectRef&& x) {
      Py_XDECREF(ref);
      ref=x.ref;
      assert(!ref || Py_REFCNT(ref));
      x.ref=nullptr;
      return *this;
    }
    operator PyObject*() const {return ref;}
    operator bool() const {return ref;}
    PyObject* release() {auto tmp=ref; ref=nullptr; return tmp;}
  };
    
  class PythonBuffer
  {
  public:
    using Array=std::deque<PythonBuffer>;
    PythonBuffer() {Py_INCREF(pyObject);}
    
    explicit PythonBuffer(RESTProcessType::Type type) {
      switch (type)
        {
        case RESTProcessType::null: pyObject=Py_None; Py_INCREF(pyObject); return;
        case RESTProcessType::boolean: pyObject=Py_False; Py_INCREF(pyObject); return;
        case RESTProcessType::int_number: pyObject=PyLong_FromLong(0); return;
        case RESTProcessType::float_number: pyObject=PyFloat_FromDouble(0.0); return;
        case RESTProcessType::string: pyObject=PyUnicode_FromString(""); return;
        case RESTProcessType::object: pyObject=PyDict_New(); return;
        case RESTProcessType::array: pyObject=PyList_New(0); return;
        }
    }
    template <class T>
    explicit PythonBuffer(const T& x): pyObject(newPyObject(x)) {}
    explicit PythonBuffer(const json_pack_t& x): pyObject(newPyObjectJson(x)) {}
    explicit PythonBuffer(const json5_parser::mArray& x): pyObject(newPyObjectJson(json_pack_t(x))) {}
    explicit PythonBuffer(PyObject* x) {
      if (x) 
        pyObject=x; 
      else
        pyObject=Py_None;
      Py_XINCREF(x);
    }
    
    ~PythonBuffer() {Py_DECREF(pyObject);}

    PythonBuffer(const PythonBuffer& x): pyObject(x.pyObject) {Py_INCREF(pyObject);}
    PythonBuffer& operator=(const PythonBuffer& x)
    {pyObject=x.pyObject; Py_INCREF(pyObject); return *this;}
  
    /// return type conforms to the sequence concept
    Array array() const {
      Array r;
      if (PySequence_Check(pyObject))
        for (Py_ssize_t i=0; i<PySequence_Size(pyObject); ++i)
          r.push_back(PythonBuffer(PySequence_GetItem(pyObject,i)));
      else
        r.push_back(*this);
      return r;
    }
    RESTProcessType::Type type() const {
      if (pyObject==Py_None) return RESTProcessType::null;
      if (PyBool_Check(pyObject)) return RESTProcessType::boolean;
      if (PyLong_Check(pyObject)) return RESTProcessType::int_number;
      if (PyFloat_Check(pyObject)) return RESTProcessType::float_number;
      if (PyUnicode_Check(pyObject)) return RESTProcessType::string;
      if (PySequence_Check(pyObject)) return RESTProcessType::array;
      return RESTProcessType::object;
    }

    //template <class T> T get() const;
    template <class T> typename enable_if<is_integral<T>, T>::T
    get() const {return PyLong_AsLongLong(pyObject);}
    template <class T> typename enable_if<is_floating_point<T>,T>::T
    get() const {return PyFloat_AsDouble(pyObject);}
    //    template <> bool get<bool>() const {return PyLong_AsLong(pyObject);}
    template <class T> typename enable_if<is_string<T>, T>::T
    get() const {return PyUnicode_AsUTF8(pyObject);}
    
    template <class T> typename enable_if<is_same<T,json_pack_t>, T>::T
    get() const {
      switch (type())
        {
        case RESTProcessType::null:
          return json_pack_t(json5_parser::mValue::null);
        case RESTProcessType::boolean:
          return json_pack_t(get<bool>());
        case RESTProcessType::int_number:
          return json_pack_t(get<int64_t>());
        case RESTProcessType::float_number:
          return json_pack_t(get<double>());
        case RESTProcessType::string:
          return json_pack_t(get<string>());
        case RESTProcessType::array:
          {
            json_pack_t::Array arr;
            for (auto& i: array())
              arr.push_back(i.get<json_pack_t>());
            return json_pack_t(arr);
          }
        case RESTProcessType::object:
          {
            json_pack_t::Object obj;
            if (PyMapping_Check(pyObject))
              {
                PyObjectRef keyValues(PyMapping_Items(pyObject));
                for (Py_ssize_t i=0; i<PySequence_Size(keyValues); ++i)
                  {
                    PyObjectRef keyValue=PySequence_GetItem(keyValues, i);
                    PyObjectRef keyRef(PyObject_Str(PySequence_GetItem(keyValue,0)));
                    string keyStr=PyUnicode_AsUTF8(keyRef);
                    obj[keyStr]=PythonBuffer(PySequence_GetItem(keyValue,1)).get<json_pack_t>();
                  }
              }
            else
              {
                auto dir(PyObject_Dir(pyObject));
                for (Py_ssize_t i=0; i<PySequence_Size(dir); ++i)
                  {
                    auto key=PySequence_GetItem(pyObject, i);
                    PyObjectRef keyRef(PyObject_Str(key));
                    string keyStr=PyUnicode_AsUTF8(keyRef);
                    obj[keyStr]=PythonBuffer(PyObject_GetAttr(pyObject, key)).get<json_pack_t>();
                  }
              }
            return json_pack_t(obj);
          }
        }
      assert(false); // silly compiler cannot figure out this is unreachable code
      return json_pack_t();
    }
    
    template <class T> void push_back(const T& x) {
      if (PyList_Check(pyObject))
        {
          PythonBuffer pb(x);
          if (PyList_Append(pyObject, pb.pyObject))
            Py_INCREF(pb.pyObject); // TODO - does PySequence_SetItem already do this?
        }
    }

    /// returns a copy of the controlled object. Reference counter is incremented.
    PyObjectRef getPyObject() const {Py_INCREF(pyObject); return pyObject;}

    std::string str() const {
      PyObjectRef pyStr(PyObject_Str(pyObject));
      return PyUnicode_AsUTF8(pyStr);
    }
  private:
    PyObject* pyObject=Py_None; // note - this needs to be INCREF'd in constructors, not immortal before 3.12
    

  };

  inline PyObject* newPyObject(const PythonBuffer& x) {return x.getPyObject();}

  inline const PythonBuffer& operator>>(const PythonBuffer& b, char& x)
  {x=b.get<string>()[0]; return b;}

  inline PythonBuffer& operator<<(PythonBuffer& b, const char& x)
  {b=PythonBuffer(string{x}); return b;}

  // numbers
  template <class T>
  inline typename enable_if<
    And<is_arithmetic<T>,Not<is_const<T>>>,
    const PythonBuffer&>::T
  operator>>(const PythonBuffer& b, T& x)
  {x=b.get<T>(); return b;}

  template <class T>
  inline typename enable_if<
    is_arithmetic<T>, PythonBuffer&>::T
  operator<<(PythonBuffer& b, const T& x)
  {b=PythonBuffer(x); return b;}

  // strings
  template <class T>
  typename enable_if<And<is_string<T>,Not<is_const<T>>>, const PythonBuffer&>::T
  operator>>(const PythonBuffer& b, T& x)
  {x=b.get<string>(); return b;}

  template <class T>
  inline typename enable_if<And<is_string<T>>, PythonBuffer&>::T
  operator<<(PythonBuffer& b, const T& x)
  {b=PythonBuffer(string(x)); return b;}

  // enums
  template <class T>
  inline typename enable_if<And<is_enum<T>,Not<is_const<T>>>, const PythonBuffer&>::T
  operator>>(const PythonBuffer& b, T& x)
  {auto tmp=b.get<string>(); x=enum_keys<T>()(tmp); return b;}

  template <class T>
  inline typename enable_if<And<is_enum<T>>, PythonBuffer&>::T
  operator<<(PythonBuffer& b, const T& x)
  {b=PythonBuffer(to_string(x)); return b;}

  // sequences
  template <class T>
  inline typename enable_if<And<is_sequence<T>,Not<is_const<T>>>, const PythonBuffer&>::T
  operator>>(const PythonBuffer& b, T& x)
  {
    resize(x,0);
    if (b.type()!=RESTProcessType::array) return b;
    auto arr=b.array();
    for (auto& i: arr)
      {
        typename T::value_type e;
        i>>e;
        push_back(x,e);
      }
    return b;
  }

  template <class T>
  inline typename enable_if<is_sequence<T>, PythonBuffer&>::T
  operator<<(PythonBuffer& b, const T& x)
  {
    b=PythonBuffer(RESTProcessType::array);
    for (auto& i: x)
      {
        b.push_back(i);
      }
    return b;
  }

  // const
  template <class T>
  inline typename enable_if<is_const<T>, const PythonBuffer&>::T
  operator>>(const PythonBuffer& b, T& x)
  {return b;}


  inline const PythonBuffer& operator>>(const PythonBuffer& b, const char* x)
  {
    throw std::runtime_error("cannot unpack to char*, please use string instead");
  }
  
  // everything else
  template <class T>
  inline typename enable_if<
    And<
      Not<is_arithmetic<T>>,
      Not<is_string<T>>,
      Not<is_sequence<T>>,
      Not<is_enum<T>>,
      Not<is_const<T>>
      >, const PythonBuffer&>::T
  operator>>(const PythonBuffer& b, T& x)
  {
    b.get<json_pack_t>()>>x;
    return b;
  }

  template <class T>
  inline typename enable_if<
    And<
      Not<is_arithmetic<T>>,
      Not<is_string<T>>,
      Not<is_sequence<T>>,
      Not<is_enum<T>>
      >, PythonBuffer&>::T
  operator<<(PythonBuffer& b, const T& x)
  {
    json_pack_t tmp; tmp<<x;
    string s=write(tmp);
    b=PythonBuffer(s);
    return b;
  }
}

namespace classdesc_access
{
  template <>
  struct access_json_pack<classdesc::PythonBuffer> {
    template <class U>
    void operator()(classdesc::json_pack_t& j, const std::string&, U& x) {
      j<<x;
    }
  };
}

#endif
