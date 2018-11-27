Platforms
---------
Minsky can be built on any of the following platforms:

* Windows: Cygwin
* Linux: MXE (for running on Windows). This is how the Windows release is built.
* MacOSX: MacPorts (possibly HomeBrew, though not tested)
* Linux

Presumably, (with effort), it could be built on any posix compliant platform.

Dependencies
------------
* gcc/g++ (C++11 compatible, ver 5 or later)
* gmake
* boost 1.47.0 or later 
* json_spirit
* TCL/Tk
* cairo
* pango
* GSL (GNU Scientific Library) 
* librsvg
* Berkeley DB (libdb) (Minsky doesn't use BDB, but it helps for installing EcoLab from source code)
* Most recent ecolab.5 release

The server component is and experimental implementation for supporting a web-based Minsky client. It is not needed for building and using the standard desktop application.

For building server (experimental)

* websocketpp 2.x (get from Github)
* soci

It is recommended that you use your package manager wherever possible to install the prerequisites. Package managers are:

* Cygwin setup.exe for Cygwin, which is a nice GUI interface
* MXE: type make <package-name> in the top level directory. See the [Windows Release build notes](WindowsRelease.md) for more details.
* MacPorts: use the ports command: eg ports install <package-name>. see the [MacOSX Release build notes][MacRelease.md] for more details.
* Debian/Ubuntu: use apt: eg apt install <package-name>-dev. Note the trailing -dev, which installs the header files and static library stubs.
* Redhat/Fedora/CentOS: use yum: eg yum install <package-name>-devel.


Notes
-----

* On Linux, MinGW and Cygwin, you can use the packaged pango (after possibly having to install development version sof the packages). On Mac, installation from sources seems to be necessary, which involves the whole palaver of GLib, libffi and gettext.

* For building json_spirit and soci, you need cmake. On MinGW, you can use the normal Windows binary for CMake. Just type (cmake -G "Unix Makefiles" .) - if you don't specify the generator, it will default to Visual Studio.

* Because of the antique version of Berkley DB installed by default on MacOS, you will probably need to install a modern 4.x version in order to get ecolab to build on MacOSX.

* These packages can be installed to /usr/local or $(HOME)/usr according to preference.


* Once the dependencies have been installed, cd to the Minsky source directory, and type "make", which builds the executable "minsky" at that location.

* Tktable 2.11 is available through the SourceForge CVS website, however, there are some important bugfixes that are currently only available from the version in the Minsky file releases area. Hopefully, this fixes will be picked up by the Tktable respository.

* TkTable doesn't really have a standard install location. I would recommend copying the dynamic library into /usr/local/lib or ~/usr/lib, in which case Minsky will will find it. Otherwise, you will need to edit Minsky's Makefile.

* The macosx build of Tcl/Tk, including the prebuilt ActiveTCL distribution is threaded. This causes thread safety problems with Tk. See ticket 13. Unfortunately, turning off threading is not possible in the macosx build, so the unix Tcl/Tk build should be used instead. For development purposes, use the standard X11 build of Tk. Tk for Aqua can be built in the unix folder by specifying --with-aqua on the configure line, but this can only be used for 32 bit builds (not 64). This is how the Minsky Mac binary distritbuion is built. 
