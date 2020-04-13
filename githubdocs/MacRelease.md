# set the environment variable MACOSX_DEPLOYMENT_TARGET to ensure maximum usability of the compiled package
 - MACOSX_DEPLOYMENT_TARGET=10.9 (ie Mavericks).
 - Earlier releases of MacOSX do not support C++11 standard library adequately for Minsky.

# Prepare mac ports (takes a while)
- download and install the latest XCode from the App Store. 
NB unfortunately, the XCode command line tools package is out of date, so you will need to install the full multigigabyte XCode package, even though we only need the command line tools.
- download ports installer for your version of MacOSX
- edit the file /opt/local/etc/macports/macports.conf, and add the following
  - macosx_deployment_target 10.12
  - buildfromsource         always
- Install rust, needed to build librsvg. Note, this cannot be built from source in this configuration, so do a binary install:
  - port -b install rust
- Now install port prerequisistes for Minsky. 
  - port install cairo pango gsl librsvg boost cmake pkgconfig
  - tcl/tk needs to be installed from source code if using Aqua. See below. If using X11, then you can use the MacPorts build of tk.
  - json_spirit needs to be installed from source code
- if you already have ports installed, you can recompile for the new deployment target with
  port upgrade --force installed


# compile TCL/Tk from source code

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

# compile json_spirit from source code (install boost and cmake first)
- tar xvf json_spirit_v4.08.tar.gz
- cd json_spirit_v4.08
- cmake .
- make -j
- sudo make install

# compile EcoLab
Note - EcoLab is now included with Minsky as a git submodule, so this section is only needed for older versions
MAC_OSC_TK=1 enables the Aqua build.
  - make -j MAC_OSX_TK=1 BDB= install 
  - note disabling BDB is required, as MacOSX has a positively ancient version of Berkeley DB installed.
  
# compile Minsky
By default, Minsky builds in Aqua mode. This may cause build errors if the prerequisites are built for X11. To disable Aqua, specify MAC_OSX_TK= on the make command line.
  - make MAC_OSX_TK=1 mac-dist
  
This should leave a .pkg file suitable for distribution in the top level directory.
