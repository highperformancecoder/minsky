#include "minskyRS.h"
#include "RESTMinsky.h"
#include "minsky_epilogue.h"
#include <Python.h>

using namespace minsky;
namespace
{
  struct ModuleMinsky: public RESTMinsky
  {
    ModuleMinsky() {RESTProcess(registry,"minsky",static_cast<Minsky&>(*this));}
  };

  ModuleMinsky& moduleMinsky()
  {
    static ModuleMinsky minsky;
    return minsky;
  }
  
  // JSON for now
  PyObject* call(PyObject* self, PyObject* args)
  {
    const char* command="", *jsonArguments=nullptr;
    if (!PyArg_ParseTuple(args, "ss", &command, &jsonArguments))
      {
        // try a no argument version
        PyErr_Clear();
        PyArg_ParseTuple(args, "s",&command);
      }
    try
      {
        cout<<"command="<<command<<endl;
        json_pack_t arguments(json5_parser::mValue{});
        if (jsonArguments)
          read(jsonArguments, arguments);
        auto result=write(moduleMinsky().registry.process(command, arguments),json5_parser::raw_utf8);
        cout<<"result="<<result<<endl;
        return Py_BuildValue("s",result.c_str());
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
  return PyModule_Create(&pyminsky);
}
