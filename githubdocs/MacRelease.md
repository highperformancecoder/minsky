# set the environment variable MACOSX_DEPLOYMENT_TARGET to ensure maximum usability of the compiled package
 - MACOSX_DEPLOYMENT_TARGET=10.9 (ie Mavericks).
 - Earlier releases of MacOSX do not support C++11 standard library adequately for Minsky.

# Prepare mac ports (takes a while)
- download and install the latest XCode from the App Store. 
NB unfortunately, the XCode command line tools package is out of date, so you will need to install the full multigigabyte XCode package, even though we only need the command line tools.
- download ports installer for your version of MacOSX
- Now install port prerequisistes for Minsky. 
  - port install cairo pango gsl librsvg boost cmake pkgconfig tk
  - Minksy 3.x does not use Tk, so the X11 version suffices. For 2.x or earlier, tcl/tk needs to be installed from source code if using Aqua. See below. 
  - json_spirit needs to be installed from source code, but is not needed for Minsky 3.x
- Currently, Mac builds are done on a High Sierra virtual machine, and the binary packages for High Sierra are used, so that is the mininum OS version for the MacOSX Minsky release. 

# Enabling MacPorts to support earlier versions of MacOSX to the current system 
- edit the file /opt/local/etc/macports/macports.conf, and add the following
  - macosx_deployment_target 10.12
  - buildfromsource         always
- Install rust, needed to build librsvg. Note, this cannot be built from source in this configuration, so do a binary install:
  - port -b install rust
- if you already have ports installed, you can recompile for the new deployment target with
  port upgrade --force installed

# compile TCL/Tk from source code

This step should no longer be needed for Minsky 3.x, but is currently, because the current EcoLab library requires it.

Because we need to use an internal function with tk when compiling Minsky for Aqua, we have to staticly link to the library. 

- tar zxvf tcl-core8.6.9-src.tar.gz
- pushd tcl8.6.9/unix/
- ./configure --prefix=$HOME/usr --disable-shared --enable-aqua
- make -j install
- popd
- tar zxvf tk8.6.9.1-src.tar.gz
- cd tk8.6.9/unix 
- ./configure --prefix=$HOME/usr --disable-shared --enable-aqua
- make -j install

# compile libclipboard from source code

- git clone git@github.com:jtanx/libclipboard.git
- cd libclipboard
- cmake .
- make -j
- sudo make install
  
# compile Minsky
By default, Minsky builds in Aqua mode. This may cause build errors if the prerequisites are built for X11. To disable Aqua, specify MAC_OSX_TK= on the make command line.
  - make MAC_OSX_TK=1 -j mac-dist
  
This builds the minskyRESTService.node, and rewrites the dynamic library references, signs the executables, produces a dmg file in dist/executables and submits the binary package to Apple for notarisation. This must be run on the Macintosh console, as it makes use of code signing, which has to be run on the console, not over an ssh connection. Notarisation takes 5-10 minutes, and if approved, should be stapled to the .dmg file. The Make step prints out the command to run the stapler.
