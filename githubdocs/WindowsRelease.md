## MXE

- The Windows distributable is built using the MXE cross-compiler environment on Linux. 

- Its best to pull from the develop branch of [my fork of MXE](https://github.com/highperformancecoder/mxe), as this is where I push any packages/changes I need. 

- make MXE_TARGETS=x86_64-w64-mingw32.static MXE_PLUGIN_DIRS=plugins/tcl.tk boost cairo tcl tk json_spirit gsl pango librsvg openssl
- For Minsky 2.20 or less, MXE_TARGETS=i686-w64-mingw32.static

- For Minsky 1.x, you will also need:
~~~~
   make MXE_TARGETS=i686-w64-mingw32.static MXE_PLUGIN_DIRS=plugins/tcl.tk tktable
~~~~

- Install necessary prerequisites from your package manager as required by the above line (eg flex, gperf, intltool, scons).

- Once these are built, if ecolab hasn't been included as a submodule, unpack ecolab, and do

~~~~
   make MXE=1 install.
~~~~
     This installs EcoLab to ~/usr/mxe/ecolab

- Then compile Minsky with

~~~~
    make MXE=1
~~~~
- Once minsky.exe is compiled, copy the entire directory onto a windows system with a posix system like cygwin installed, as well as the WiX msi generator. 
- The run GUI/makeMsi.sh to generate the installer version, which creates an msi file.
