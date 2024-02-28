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

// TODO figure out how to switch buffer types... For now, use JSON
//#define REST_PROCESS_BUFFER classdesc::PythonBuffer
#include "pythonBuffer.h"

#include "RESTMinsky.h"
#include "minsky_epilogue.h"
#include <Python.h>

using namespace minsky;
namespace
{
  struct ModuleMinsky: public Minsky
  {
    RESTProcess_t registry;
    ModuleMinsky() {
      classdesc_access::access_RESTProcess<minsky::Minsky>()(registry,"minsky",static_cast<Minsky&>(*this));
      registry.add("minsky", new RESTProcessObject<minsky::Minsky>(*this));
      
    }
  };

  ModuleMinsky& moduleMinsky()
  {
    static ModuleMinsky minsky;
    return minsky;
  }
  
  PyObject* callMinsky(const string& command, const PythonBuffer& arguments);

  // JSON for now
  PyObject* call(PyObject* self, PyObject* args)
  {
    string command=PyUnicode_AsUTF8(PySequence_GetItem(args,0));
    PythonBuffer arguments;
    if (PySequence_Size(args)>1)
      {
        arguments=PythonBuffer(RESTProcessType::array);
        for (size_t i=1; i<PySequence_Size(args); ++i)
          arguments.push_back(PySequence_GetItem(args,i));
      }
    if (PyErr_Occurred())
      PyErr_Print();
    return callMinsky(command, arguments);
  }

  struct CppWrapper: public PyObject
  {
    string command;
    map<string, PyObjectRef> methods;
    static CppWrapper* create(const string& command) {return new CppWrapper(command);}
    CppWrapper(CppWrapper&&)=default;
  private:
    CppWrapper(const string& command); // private to force creation on heap
    CppWrapper(const CppWrapper&)=delete;
    void operator=(const CppWrapper&)=delete;
  };

struct CppWrapperType: public PyTypeObject
  {
    // container commands that take a key as as an argument
    static bool containerSpecialCommand(const std::string& command)
    {
      static set<string> specialCommands{".@elem",".@elemNoThrow"};
      auto n=command.rfind('.');
      return n!=string::npos && specialCommands.count(command.substr(n));
    }
    
    static PyObject* call(PyObject* self, PyObject* args, PyObject *kwargs)
    {
      auto cppWrapper=static_cast<CppWrapper*>(self);
      PythonBuffer arguments(RESTProcessType::array);
      auto command=cppWrapper->command;
      if (containerSpecialCommand(command) && PySequence_Size(args))
        // handle special commands which embed the argument in the path string
        command+='.'+write(PythonBuffer(PySequence_GetItem(args,0)).get<json_pack_t>());
      else
        for (size_t i=0; i<PySequence_Size(args); ++i)
          arguments.push_back(PySequence_GetItem(args,i));
      if (PyErr_Occurred())
        PyErr_Print();
      return callMinsky(command, arguments);
    }

    static void deleteCppWrapper(PyObject* x) {delete static_cast<CppWrapper*>(x);}

    static PyObject* getAttro(PyObject* self, PyObject* attr)
    {
      auto cppWrapper=static_cast<CppWrapper*>(self);
      auto i=cppWrapper->methods.find(PyUnicode_AsUTF8(attr));
      if (i!=cppWrapper->methods.end())
        {
          Py_INCREF(i->second);
          return i->second;
        }
      PyErr_SetString(PyExc_AttributeError, "Method not found");
      return nullptr;
    }
    
    static int setAttro(PyObject* self, PyObject* name, PyObject* attr)
    {
      auto cppWrapper=static_cast<CppWrapper*>(self);
      auto key=PyUnicode_AsUTF8(name);
      if (attr)
        {
          cppWrapper->methods[key]=PyObjectRef(attr);
          assert(Py_REFCNT(cppWrapper->methods[key]));
        }
      else
        cppWrapper->methods.erase(key);
      return 0;
    }

    CppWrapperType()
    {
      memset(this,0,sizeof(PyTypeObject));
      Py_INCREF(this);
      tp_name="CppWrapperType";
      tp_call=call;
      tp_basicsize=sizeof(CppWrapper);
      tp_dealloc=deleteCppWrapper;
      tp_getattro=getAttro;
      tp_setattro=setAttro;
      PyType_Ready(this);
    }
  } cppWrapperType;

  CppWrapper::CppWrapper(const string& command): command(command) {
    memset(this,0,sizeof(PyObject));
    ob_refcnt=1;
    ob_type=&cppWrapperType;
    methods.emplace("__dict__",cppWrapperType.tp_dict);
    Py_XINCREF(cppWrapperType.tp_dict);
  }

  void attachMethods(PyObjectRef& pyObject, const std::string& command)
  {
    if (!pyObject) return;
    try
      {
        auto methods=moduleMinsky().registry.process(command+".@list",{});
        if (methods.type()!=RESTProcessType::array) return;
        for (auto& i: methods.array())
          {

            string methodName(i.get_str());
            auto uqMethodName=methodName.substr(1); // remove leading '.'
            if (uqMethodName.find('.')!=string::npos) continue; // ignore recursive commands
            PyObjectRef method{CppWrapper::create(command+methodName)};
            if (uqMethodName.find('@')!=string::npos)
              {
                // make special commands representable in python
                replace(uqMethodName.begin(),uqMethodName.end(),'@','_');
              }
            else
              {  // and recurse
                attachMethods(method, command+methodName);
              }
            PyObject_SetAttrString(pyObject, uqMethodName.c_str(), method.release());
          }
        PyObject_SetAttrString(pyObject, "_list", newPyObject(methods));
        PyObject_SetAttrString(pyObject, "_type", CppWrapper::create(command+".@type"));
        PyObject_SetAttrString(pyObject, "_signature", CppWrapper::create(command+".@signature"));
      }
    catch (...) { } // do not log, nor report errors back to python - there are too many
    if (PyErr_Occurred())
      PyErr_Print();
  }
  
  PyObject* callMinsky(const string& command, const PythonBuffer& arguments)
  {
    try
      {
        PythonBuffer result(moduleMinsky().registry.process(command, arguments.get<json_pack_t>()));
        auto pyResult=result.getPyObject();
        if (result.type()==RESTProcessType::object)
          {
            PyObjectRef r(CppWrapper::create(command));
            PyObject_SetAttrString(r,"_properties",pyResult.release());
            attachMethods(r, command);
            return r.release();
          }
        if (PyErr_Occurred())
          PyErr_Print();
        return pyResult.release();
      }
    catch (const std::exception& ex)
      {
        PyErr_SetString(PyExc_RuntimeError, ex.what());
        return nullptr;
      }
    catch (...)
      {
        PyErr_SetString(PyExc_RuntimeError, "Unknown exception");
        return nullptr;
      }
      
  }

  PyMethodDef moduleMethods[] = {
    {"call", call, METH_VARARGS, "Backend call"},
    {NULL, NULL, 0, NULL} 
  };
  
  PyModuleDef pyminsky = {
    PyModuleDef_HEAD_INIT,
    "pyminsky", // Module name
    "Python interface to Minsky's backend API",
    -1,   // Optional size of the module state memory
    moduleMethods, // Optional module methods
    NULL, // Optional slot definitions
    NULL, // Optional traversal function
    NULL, // Optional clear function
    NULL  // Optional module deallocation function
  };

}

namespace minsky
{
  LocalMinsky::LocalMinsky(Minsky&) {}
  LocalMinsky::~LocalMinsky() {}
  // GUI callback needed only to solve linkage problems
  void doOneEvent(bool idleTasksOnly) {}
  Minsky& minsky() {return ::moduleMinsky();}
}

PyMODINIT_FUNC PyInit_pyminsky(void)
{
  auto module=PyModule_Create(&pyminsky);
  if (module)
    {
      PyModule_AddObject(module, "t", CppWrapper::create("minsky.t"));
      PyObjectRef minsky=CppWrapper::create("minsky");
      attachMethods(minsky,"minsky");
      PyModule_AddObject(module, "minsky", minsky.release());
    }
  return module;
}

