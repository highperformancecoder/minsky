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
* gcc/g++ (C++11 compatible)
* gmake
* boost 1.66.0 or later 
* json_spirit
* TCL/Tk
* cairo
* pango
* GSL (GNU Scientific Library) 
* librsvg

Hint: - if building on a Linux system, you can take a sneak peek at either [minsky.spec](https://build.opensuse.org/package/view_file/home:hpcoder1/minsky/minsky.dsc?expand=1) (for Redhat/Fedora/CentOS/OpenSUSE) or [minsky.dsc](https://build.opensuse.org/package/view_file/home:hpcoder1/minsky/minsky.dsc?expand=1) (for Debian/Ubuntu) at  to see what packages you can install to get your dependencies installed via the system's built-in package manager.

Package Managers
----------------
It is recommended that you use your package manager wherever possible to install the prerequisites. Package managers are:

* Cygwin setup.exe for Cygwin, which is a nice GUI interface
* MXE: type make <package-name> in the top level directory. See the [Windows Release build notes](WindowsRelease.md) for more details.
* MacPorts: use the ports command: eg ports install _package-name_. see the [MacOSX Release build notes](MacRelease.md) for more details.
* Debian/Ubuntu: use apt: eg apt install _package-name_-dev. Note the trailing -dev, which installs the header files and static library stubs.
* Redhat/Fedora/CentOS: use yum: eg yum install _package-name_-devel.
* SUSE/OpenSUSE: use zypper, or the yast2 GUI interface: eg zypper install _package-name_-devel.

* In all of the above command line package managers, you can use the search subcommand to find the exact name of a package to install:
~~~~
> apt search rsvg
librsvg2-2/artful,now 2.40.18-1 amd64 [installed]
  SAX-based renderer library for SVG files (runtime)

librsvg2-bin/artful 2.40.18-1 amd64
  command-line and graphical viewers for SVG files

librsvg2-common/artful,now 2.40.18-1 amd64 [installed]
  SAX-based renderer library for SVG files (extra runtime)

librsvg2-dev/artful 2.40.18-1 amd64
  SAX-based renderer library for SVG files (development)

librsvg2-doc/artful,artful 2.40.18-1 all
  SAX-based renderer library for SVG files (documentation)
~~~~

* json_spirit and ecolab are available prebuilt for Linux from the [OpenSUSE build service](https://build.opensuse.org/project/show/home:hpcoder1). You will need to follow the instructions to add it as a software repository.

Notes
-----
* If you're working on the development head of Minsky, you should really build from EcoLab's development head.
* json_spirit can also be built from [sources](https://www.codeproject.com/Articles/20027/JSON-Spirit-A-C-JSON-Parser-Generator-Implemented) instead of adding the opensuse build service as a repository. Github clones of json_spirit also exist. 

* For building json_spirit and soci, you need cmake, which you can install from your package manager. On MinGW, you can use the normal Windows binary for CMake. Just type (cmake -G "Unix Makefiles" .) - if you don't specify the generator, it will default to Visual Studio.

* For building json_spirit and soci, you need cmake. On MinGW, you can use the normal Windows binary for CMake. Just type (cmake -G "Unix Makefiles" .) - if you don't specify the generator, it will default to Visual Studio. On unix systems, "cmake ." suffices.

* Because of the antique version of Berkley DB installed by default on MacOS, you will probably need to install a modern 4.x version in order to get ecolab to build on MacOSX.

* These packages can be installed to /usr/local or $(HOME)/usr according to preference when built from source code. EcoLab will look first in $(HOME)/usr then in /usr/local, then finally in /usr. This allows the user to selectively override their own build environment, the global build environment according to their permissions.

* Once the dependencies have been installed, cd to the Minsky source directory, and type "make", which builds the executable "minsky" at that location.

* The macosx build of Tcl/Tk, including the prebuilt ActiveTCL distribution is threaded. This causes thread safety problems with Tk. See ticket 13. Unfortunately, turning off threading is not possible in the macosx build, so the unix Tcl/Tk build should be used instead. For development purposes, use the standard X11 build of Tk. Tk for Aqua can be built in the unix folder by specifying --with-aqua on the configure line, but this can only be used for 32 bit builds (not 64). This is how the Minsky Mac binary distritbuion is built. 
