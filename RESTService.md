# Minsky REST Service

REST (https://en.wikipedia.org/wiki/Representational_state_transfer) is an architectural style for manipulating an object via a simple protocol such as http. In this case, the Minsky REST server instantiates a Minsky model object, and the object can be manipulated using two items: a URI and a JSON encoded set of arguments. A simple example of this is querying or setting the minsky t attribute:
   * /minsky/t, null  - gets the value of t
   * /minsky/t, 0.1   - set the value of t to 0.1, and returns it.
   
## Available REST servers
There are currently two REST servers available in the RESTService branch
   * RESTService - an interactive command line utility allowing you to type the URI on the first line, then a piece of JSON on the second line. Pressing return on the second line is equivalent to passing the JSON null object.
   * httpd - implements an http server. It takes one argument - a port number to listen on.
   ~~~~
   httpd 8000
   ~~~~
   In a web browser, you can perform a get request (second json argument is null) in a web browser by browsing to http://localhost:8000/minsky/t
   To call a method taking arguments, you need to issue a put request. A number of utilities can be used to do this, for example wget
   ~~~~
   wget --method=put --body-data=0.2 -O- -o/dev/null --content-on-error=on http://localhost:8000/minsky/t
   ~~~~
   
## The Minsky model
The leadin string /minsky on the URI introduces the global minsky model.
The available attributes and methods are described in [Minsky's API](https://minsky.sourceforge.io/doxydoc/html/). 

## Attributes
An attribute can be accessed as a getter (REST arguments is null) or a setter (REST argument contains a suitable JSON encoded value) (see previous /minsky/t example).

## Methods
Methods can be called also by specifying the arguments as a json array. A single argument function cann be called with a one element JSON array, or just the JSON for the argument. 

~~~~
/minsky/convertVarType ["foobar", "flow"]
/minsky/load "examples/GoodwinLinear02.mky"
~~~~

An exception to this latter rule is if the argument is a container, such as std::vector, in which case nested arrays are needed to indicate that a single sequence argument is intended ie

~~~~
/minsky/intVector [[1,2,3,4]]
~~~~

not

~~~~
/minsky/intVector [1,2,3,4]
~~~~

Overloaded methods are supported, with limitations. Best effort is made to resolve which method overload the JSON arguments refer to, but sometimes this is just not possible. Methods with different numbers of arguments are always resolvable.

## Introspection of the model

There are a number of meta methods, introduced by the '@' sign for query types/signatures and available methods:
* @type - `/minsky/@type null` -> ::minsky::Minsky
* @signature `/minsky/t/@signature null` -> [{"args":[],"ret":"double"},{"args":["double"],"ret":"double"}]
  This means that /minsky/t is an overloaded method, taking 0 or 1 argument of type double, and returning a double. All attributes have a similar sort of signature representing the getter and setter methods.
* @list - `/minsky/variableValues/@list null` -> ["/@elem","/@insert","/@size"]

## Containers

Container types (eg std::vector, std::map) have some additional properties, introduced by the '@' sign.
* @size - returns the number of elements in the container
* @elem - indexes into the container. /@elem/0 refers the the first element of a vector, or /@elem/bar refers to the "bar" element of a map with string key type.
* @insert allows new elements to be inserted into the container

## enums

The "enumerators" of an enum can be listed using the /@enum command. These are the strings that you can pass to arguments expecting an enum type, eg an enum setter. /@enum/@list will list available enums. 

For example, a list of operations can be obtained with
~~~~
/@enum/::minsky::OperationType::Type=>["constant","time","integrate","differentiate","data","ravel","add","subtract","multiply","divide","log","pow","lt","le","eq","min","max","and_","or_","copy","sqrt","exp","ln","sin","cos","tan","asin","acos","atan","sinh","cosh","tanh","abs","floor","frac","not_","sum","product","infimum","supremum","any","all","infIndex","supIndex","runningSum","runningProduct","difference","innerProduct","outerProduct","index","gather","numOps"]
~~~~
and variable types with
~~~~
/@enum/::minsky::VariableType::Type=>["undefined","constant","parameter","flow","integral","stock","tempFlow","numVarTypes"]
~~~~


