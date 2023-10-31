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

#define REST_PROCESS_BUFFER classdesc::PythonBuffer
#include "pythonBuffer.h"

#include "minskyRS.h"
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
    try
      {
        cout<<"command="<<command<<endl;
        auto result=moduleMinsky().registry.process(command, arguments).getPyObject();
        if (PyErr_Occurred())
          PyErr_Print();
        return result;
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

