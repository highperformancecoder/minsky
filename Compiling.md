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
* json_spirit (for older versions of Minsky, more recent versions have git submoduled json5_parser, so json_spirit is not needed)
* [jtanx's libclipboard](https://github.com/jtanx/libclipboard)
* TCL/Tk
* cairo
* pango
* GSL (GNU Scientific Library) 
* librsvg
* openSSL
* readline
* nodeJS development (for building gui-js). Perform `npm install` in top level directory before building to ensure node-addon-api is installed.
* unittest++ (for building tests)
* Xss - sometimes you need to explicitly install this - required by Tk on XWindows systems

Hint: - if building on a Linux system, you can take a sneak peek at either [minsky.spec](https://build.opensuse.org/package/view_file/home:hpcoder1/minsky/minsky.dsc?expand=1) (for Redhat/Fedora/CentOS/OpenSUSE) or [minsky.dsc](https://build.opensuse.org/package/view_file/home:hpcoder1/minsky/minsky.dsc?expand=1) (for Debian/Ubuntu) at  to see what packages you can install to get your dependencies installed via the system's built-in package manager.

Package Managers
----------------
It is recommended that you use your package manager wherever possible to install the prerequisites. Package managers are:

* Cygwin setup.exe for Cygwin, which is a nice GUI interface
* MXE: type make <package-name> in the top level directory. See the [Windows Release build notes](githubdocs/WindowsRelease.md) for more details.
* MacPorts: use the ports command: eg ports install _package-name_. see the [MacOSX Release build notes](githubdocs/MacRelease.md) for more details.
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

* json_spirit, libclipboard and ecolab are available prebuilt for Linux from the [OpenSUSE build service](https://build.opensuse.org/project/show/home:hpcoder1). You will need to follow the instructions to add it as a software repository.

Notes
-----
* Use "git submodule update --init --recursive" to pick up the correct version of ecolab.
* json_spirit can also be built from [sources](hhttps://github.com/highperformancecoder/json_spirit) instead of adding the opensuse build service as a repository. Other github clones of json_spirit also exist, but the highperformancecoder version has some modifications needed for Minsky, which have been pull requested upstream. Note that most recent json_spirits do not build mValues by default, so you need to configure that option with cmake.
* [libclipboard's source code](https://github.com/jtanx/libclipboard)

* For building json_spirit and libclipboard, you need cmake, which you can install from your package manager. On MinGW, you can use the normal Windows binary for CMake. Just type (cmake -G "Unix Makefiles" .) - if you don't specify the generator, it will default to Visual Studio.

* If you build json_spirit as a library, then enable JSON_SPIRIT_MVALUE_ENABLED : eg
"cmake -DJSON_SPIRIT_MVALUE_ENABLED=ON .."

* Because of the antique version of Berkley DB installed by default on MacOS, you will probably need to install a modern 4.x version in order to get ecolab to build on MacOSX.

* These packages can be installed to `/usr/local` or `$HOME/usr` according to preference when built from source code. EcoLab will look first in `$HOME/usr` then in `/usr/local`, then finally in `/usr`. This allows the user to selectively override their own build environment, the global build environment according to their permissions.

* Once the dependencies have been installed, type `make` (`make -j9` for parallel build) in the top level directory. This creates the `minskyRESTService.node` addon file, and places it in `gui-js/node-addons`, which is the Minsky backend.


Javascript Front End
--------------------

* cd into `gui-js`
* First time, run `npm install` to install the development environment
* To run minsky, type `npm start`, which compiles and runs the front end.
* To create a built, installable version, type `npm run export:package:$os` where `os` is one of `windows`, `linux` or `mac`
