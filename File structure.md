# File structure

### doc

Latex Documentation

### engine

The main engine written in C++

### examples

Example minsky model `.mky` files

### gui-tk

* Deprecated, will not be supported post version 3.0.

[GUI for tk](http://wiki.tcl.tk/477) in C++.
TK is a cross-platform windowing toolkit for Tcl, providing tools necessary to develop graphical applications that run on:
- Windows
- Unix
- Mac OS X
- and other platforms ...

### gui-js

The new front end, implemented in Angular and Typescript, on top of the Electron framework.

#### gui-js/apps/minsky-electron

Code directly used for Electron's main process

#### gui-js/apps/minsky-web

Code run in the main BrowserWindow thread

#### gui-js/libs/core

Support code for BrowserWindow threads

#### gui-js/libs/menu

Code supporting the main window menu bar

#### gui-js/node-addons

Where the compiled C++ backend `minskyRESTService.node` is placed, and any supporting dynamic libs

#### gui-js/libs/shared

Share code between BrowserWindow threads and the main thread

#### gui-js/libs/ui-components

Code supporting different UI elements.

### minsky.app

Mac App configuration

### model

Main application logic

### obsCheck

A regression test of Linux distributables via the [OpenSUSE Build Service](https://build.opensuse.org)

### RESTService

Code supporting creating the Backend (aka REST service, or RESTApi). Built in this directory are:

* `minsky-RESTService`, a command line tool for exploring the REST API
* `minsky-httpd`, a simple HTTP server, supporting the REST API
* `minskyRESTService.node`, a node addon that implements the REST API, but callable from within a nodeJs process. This target is actually stashed in gui-js/node-addons, ready for frontend use.

### schema

Schemas for main model entities such as `Variable`, `Port`, `Wire`, `Operation`, ...


### tensor

A tensor library called "Civita", that will be spun out as a separate standalone library at some point.

### test

Directory of unit and regression tests. To run the tests, type `make
sure` in the top level directory.

### submodule directories

* certify, SSL support for https connections
* ecolab, used for plots, classdesc and TCL descriptors (deprecated)
* exprtk, algebraic expression library
* RavelCAPI, A C++ wrapper library around the Ravel™ C API



