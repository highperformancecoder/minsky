# Scripting Minsky in Python
There are a number of applications for driving Minsky from scripts, ranging from automating tasks over multiple .mky files, regression tests, parameter sweeps and ensemble runs. Since early 2024. it has been possible to script Minsky using Python. Note Python 2 is _not_ supported, you must use Python 3.x

## Setting the python path to find the pyminsky module

- On *Linux*, this step is unnecessary if you have installed Minsky from the Open Build Service, as the pyminsky module is installed in a standard location for the standard python or python3 executable for that distro. If you are using a non-standard python version, you will need to copy or link the module (pyminsky.so) to somewhere in your python path.
- On *Windows*, the python module (pyminsky.pyd) is installed in the same directory as Ravel executable. On my system that is $LOCALAPPDATA/Programs/minsky. So the following snippet should set up you python path:
  ~~~~
  import sys,os
  sys.path.append(os.environ['LOCALAPPDATA']+'\\Programs\\minsky')
  ~~~~
- On *MacOS*, the python module (pyminsky.so) is located in the Resource directory of whereever you have installed ravel.app. Assuming you have installed this in the standard location, the following snippet should work
  ~~~~
  import sys,os
  sys.path.append('/Applications/ravel.app/Contents/Resources/build')
  ~~~~

## The minsky object
Minsky is structured around a singleton object "minsky". So pyminsky has a minsky object that refers to the C++ Minsky singleton. An example usage is:
 ~~~~
from pyminsky import minsky
print(minsky.minskyVersion())
print(minsky.t())   # print the current timestep
minsky.t(10.2)      # set the timestep to 10.2
~~~~

## C++ reflection
The pyminsky module reflects the C++ code. Pretty much all public methods and attributes of the C++ classes are accessible from Python. 

### Attributes
Attributes appear as an overloaded _getter/setter_ pair. Calling the attribute without an argument is treated as a getter, and with an argument as a setter. C++ `bool`, numeric and string types map to the obvious Python equivalents. Standard vectors, lists and sets map to python lists. Standard maps map to a list first/second pairs containing the key-value pairs. Structs and classes are mapped to dicts.

Compound types have additional syntax elements. You can access the individual methods and attributes of a class or struct member, eg
~~~~
>>> minsky.model.items()
[{'bb': {}, 'bookmark': False, 'deleteCallback': '', 'm_sf': 1.0, 'm_x': 0.0, 'm_y': 0.0, 'mouseFocus': False, 'onBorder': False, 'onResizeHandles': False, 'selected': False}]
~~~~

Sequences can have a len operator applied, and be indexed
~~~~
>>> len(minsky.model.items())
1
>>> minsky.model.items[0]()
{'bb': {}, 'bookmark': False, 'deleteCallback': '', 'm_sf': 1.0, 'm_x': 0.0, 'm_y': 0.0, 'mouseFocus': False, 'onBorder': False, 'onResizeHandles': False, 'selected': False}
>>> minsky.model.items[0].name()
'hello'
~~~~

Maps can be indexed by their keys:
~~~~
>>> len(minsky.variableValues)
3
>>> minsky.variableValues.keys()
[':hello', 'constant:one', 'constant:zero']
>>> minsky.variableValues[':hello'].name()
'hello'
~~~~

### enums

enums are mapped to python strings. pyminsky has an enum dict containing all the enums defined in the system, and their possible values. 
~~~~
>>> pyminsky.enum
{'::minsky::Canvas::LassoMode::type': ['none', 'lasso', 'itemResize'], '::minsky::ClickType::Type': ['onItem', 'onPort', 'outside', 'inItem', 'onResize', 'legendMove', 'legendResize'], '::minsky::GodleyAssetClass::AssetClass': ['noAssetClass', 'asset', 'liability', 'equity'], '::minsky::GodleyAssetClass::DisplayStyle': ['DRCR', 'sign'], '::minsky::Group::IORegion::type': ['none', 'input', 'output', 'topBottom'], '::minsky::JustificationStruct::Justification': ['left', 'right', 'centre'], '::minsky::Minsky::CmdData': ['no_command', 'is_const', 'is_setterGetter', 'generic'], '::minsky::Minsky::MemCheckResult': ['OK', 'proceed', 'abort'], '::minsky::OperationType::Group': ['general', 'constop', 'binop', 'function', 'reduction', 'scan', 'tensor', 'statistics'], '::minsky::OperationType::Type': ['constant', 'time', 'integrate', 'differentiate', 'data', 'ravel', 'euler', 'pi', 'zero', 'one', 'inf', 'percent', 'add', 'subtract', 'multiply', 'divide', 'min', 'max', 'and_', 'or_', 'log', 'pow', 'polygamma', 'lt', 'le', 'eq', 'userFunction', 'copy', 'sqrt', 'exp', 'ln', 'sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'sinh', 'cosh', 'tanh', 'abs', 'floor', 'frac', 'not_', 'Gamma', 'fact', 'sum', 'product', 'infimum', 'supremum', 'any', 'all', 'infIndex', 'supIndex', 'runningSum', 'runningProduct', 'difference', 'differencePlus', 'innerProduct', 'outerProduct', 'index', 'gather', 'meld', 'merge', 'slice', 'size', 'shape', 'mean', 'median', 'stdDev', 'moment', 'histogram', 'covariance', 'correlation', 'linearRegression', 'numOps'], '::minsky::VariableType::Type': ['undefined', 'constant', 'parameter', 'flow', 'integral', 'stock', 'tempFlow', 'numVarTypes']}
~~~~

You can set/get their values like any other C++ type, but if you attempt to set an enum to a string value not in its range of values, nothing will happen, no error is raised.

### Method calls
C++ methods are called, and passed python parameters are converted into their C++ equivalents where possible, and an exception thrown where not. Mostly, the obvious thing will work, but some peculiarities can occur. For example if a method is overloaded with both an int argument and a double argument, then calling the method on 3 will call the int version, but calling it on 3.0 will call the double version. Sometimes it can be non-obvious which method will be called, so some experimentation is in order. Structs, and class public members on parameters can be set by passing a dict object. members not set within the dict are set to the default values, as though default constructed in C++.

## Introspection and Minsky API

Introspection is possible on any C++ object by the following methods:
- `_list()`: return a list of methods/attributes available on the object
- `_signature()`: return and arguments types for all overloads of a function
- `_type()`: type of the object. This is a C++ name, but allows you to look up the type in the [API documentation](https://minsky.sourceforge.io/doxydoc/html/index.html)
- `keys()`: returns the list of keys of a map object (eg std::map, std::unordered_map, etc).
- `pyminsky.enum`: is a dict containing all enumerators for all enums registered in the system

Minsky is open source, so ultimately the documented API is found by consulting the source code. However, as useful annotated API summary is created by running the Doxygen tool in the top level directory, [the results of which for the latest release can be found online](https://minsky.sourceforge.io/doxydoc/html/index.html).
