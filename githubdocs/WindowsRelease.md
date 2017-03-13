## MXE

- The Windows distributable is built using the MXE cross-compiler environment on Linux. 

- Its best to pull from [my fork of MXE](https://github.com/highperformancecoder/mxe), as this is where I push any packages/changes I need. 

- make boost cairo tcl tk TkTable json_spirit gsl pango librsvg

- Once these are built, unpack ecolab, and do

   make MXE=1 install.

  This installs EcoLab to ~/usr/mxe/ecolab

- Then compile Minsky with

make MXE=1

- Once minsky.exe is compiled, copy the entire directory onto a windows system with a posix system like cygwin installed, as well as the WiX msi generator. 
- The run GUI/makeMsi.sh to generate the installer version, which creates an msi file.
