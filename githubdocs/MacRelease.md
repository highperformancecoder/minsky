# set the environment variable MACOSX_DEPLOYMENT_TARGET to ensure maximum usability of the compiled package
 - MACOSX_DEPLOYMENT_TARGET=10.9 (ie Mavericks).
 - Earlier releases of MacOSX do not support C++11 standard library adequately for Minsky.

# Prepare mac ports (takes a while)
- download and install the latest XCode from the App Store. 
NB unfortunately, the XCode command line tools package is out of date, so you will need to install the full multigigabyte XCode package, even though we only need the command line tools.
- download ports installer for your version of MacOSX
- edit the file /opt/local/etc/macports/macports.conf, and add the following
  - macosx_deployment_target 10.9
  - buildfromsource         always
- Now install port prerequisistes for Minsky. 
  - tk +quartz (or x11)
  - cairo
  - pango
  - gsl
- if you already have ports installed, you can recompile for the new deployment target with
  port upgrade --force installed

# compile TkTable
  - configure
  - make
  - cp libTktable2.11.dylib /usr/local/lib
  
# compile EcoLab
  - make install
  
# compile Minsky
  - make mac-dist
  
This should leave a .pkg file suitable for distribution in the top level directory.
