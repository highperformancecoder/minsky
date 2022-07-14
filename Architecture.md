# Architecture

### engine

The main engine written in C++

`DAG` ie. Directed Acyclical Graph

- `derivative` - contains implementation of symbolic differentiation
- `equations` - `MathDAG` and more ...
- `evalGodley` - evaluate a Godley Table
- `evalOp` - evaluate an Operation. `EvalOp`, `ConstantEvalOp`, `EvalOpVector`
- `expr` - syntactic sugar for representing expressions
- `flowCoef` - a numerical coefficient times a variable (a "flow")
- `godleyExport` - export a Godley Table to CSV or Latex
- `integral` - additional stock variable, that integrates its flow variable
- `variableValue` - `VariableValue`, `ValueVector`, `VariableValues`


### model

Main application logic:

The main app uses [Cairo](https://www.cairographics.org/)

Cairo is a 2D graphics library with support for multiple output devices.
Currently supported output targets include the X Window System (via both Xlib and XCB), Quartz, Win32, image buffers,
PostScript, PDF, and SVG file output. Experimental backends include OpenGL, BeOS, OS/2, and DirectFB.

- `cairoItems` - display [Cairo](https://www.cairographics.org/) screen items
- `equationDisplayItem`
- `godley` - `GodleyTable` - godley table data container and logic
- `godleyIcon` - `GodleyIcon` - Godley table icon
- `group` - `Group`, `GroupPtr`, `GroupItems`, `ClosestPort` - grouping of ...
- `item` - `ClickType`, `ItemPortVector`, `Item` - generic display item
- `minsky` - `MinskyExclude`, `MinskyMatrix`, `Minsky`, `LocalMinsky` - main app to render canvas etc
- `notebase` - `NoteBase`
- `operationBase` - `Constant`, `NamedOp`, `IntOp`, `DataOp`, `OperationPtr`, `Operations` - perform operation
- `port` - `Port`
- `selection` - `Selection`, `LassoBox` - select screen items
- `slider` - `Slider` - slider
- `svgItem` - `SVGItem` - generic SVG screen item
- `switchIcon` - `SwitchIcon` - switch icon
- `variable` - `Variable`, `VarConstant`, `VariablePtr` - variable
- `wire` - `Wire` - wire (ie. connection) between DAG nodes
- `zoom` zoom transformation

### schema

Schemas for main model entities (ie. data containers) such as `Variable`, `Port`, `Wire`, `Operation`, ...

### test

Various tests
