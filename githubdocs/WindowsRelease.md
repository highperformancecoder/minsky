## MXE

- The Windows distributable is built using the MXE cross-compiler environment on Linux. 

- Its best to pull from the master branch of [my fork of MXE](https://github.com/highperformancecoder/mxe), as this is where I push any packages/changes I need. 

- make MXE_TARGETS=x86_64-w64-mingw32.static MXE_PLUGIN_DIRS=plugins/tcl.tk boost cairo tcl tk gsl pango librsvg openssl readline ncurses
- For Minsky 2.20 or less, MXE_TARGETS=i686-w64-mingw32.static

- Install necessary prerequisites from your package manager as required by the above line (eg flex, gperf, intltool, scons).

- Then compile Minsky with

~~~~
    make MXE=1
~~~~
- Once minskyRESTService.node is compiled, 
~~~~~
    cd gui-js
    npm run export:package:windows
~~~~~
- Installer will be found in `gui-js/dist/executable`

## Debugging
With MXE, there is very limited debugging facilities. Essentially all that is possible is to add print statements. To build Minsky such that messages to the console are visible, do
~~~~
make MXE=1 DEBUG=1 OPT="-O3 -NDEBUG"
~~~~
Note that without the OPT parameter, the compiler core dumps.
