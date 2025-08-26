## MXE

- The Windows distributable is built using the MXE cross-compiler environment on Linux. 

- Use the mxe-for-minsky branch of [my MXE fork](https://github.com/highperformancecoder/mxe/tree/mxe-for-minsky). 

- `make MXE_TARGETS=x86_64-w64-mingw32.shared boost cairo gsl pango librsvg openssl readline ncurses postgresql sqlite`
- For Minsky 2.20 or less, `MXE_TARGETS=i686-w64-mingw32.static`
- Prior to Minsky 3.16.16, TCL/Tk is a required dependency. To install TCL/Tk, do `make MXE_TARGETS=x86_64-w64-mingw32.shared MXE_PLUGIN_DIRS=plugins/tcl.tk tcl tk`
- For RAVELPRO builds, you need to compile soci from source code. This should pick up all the supported drivers automatically if postgresql and sqlite have been built above. Leave the install directory as the default selected by `x86_64-w64-mingw32.shared-cmake`.

- Install necessary prerequisites from your package manager as required by the above line (eg flex, gperf, intltool, scons).
- Ensure the usr/bin directory of the cloned repo is in your PATH.
- Compile libclipboard from source code: - [not needed after 3.2.0-alpha.15](https://sourceforge.net/p/minsky/tickets/1662/)
  * `git clone git@github.com:jtanx/libclipboard.git`
  * `cd libclipboard`
  * `x86_64-w64-mingw32.shared-cmake .`
  * `make install`

- Code signing
  * We use jsign, and a recent version of the Java runtime.
  * With SafeNet hardware code signing token, you need the SafeNet library installed, and a recent version of jsign (6.0) that supports the ETOKEN store type.
  * Prior to the SafeNet token, we used a code signing certificate (.pfx file), and specify its name in `WINDOWS_SIGN_CERTIFICATE_NAME` environment   variable and password in WINDOWS_SIGN_TOKEN_PASSWORD environment variable.
  * Intricate details of code signing can be found in the `mkWindowsDist.sh` script.
- Install lld on your system (the LLVM linker), then link it to the MXE linker `ln -sf /usr/bin/ld.lld $(MXEHOME)/usr/bin/x86_64-w64-mingw32.shared-ld`. This make a dramatic improvement to build times, with the link portion of the build shrinking from over 13 minutes to less than 2 seconds.
- Then compile Minsky with

~~~~
    make MXE=1
~~~~
- Once minskyRESTService.node is compiled, 
~~~~~
    cd gui-js
    npm run export:package:windows
~~~~~
  NB this step is now called by the `make MXE=1` target.
- Installer will be found in `gui-js/dist/executable`, and also copied to /tmp.
- After a complete rebuild of MXE, some of the dependent DLL names may have changed, in which case you'll get an error message about not being able to read 'setMessageCallback'. This is a bugger to debug, but you can use depends.exe to open the minskyRESTService.node file, add the minsky.exe directory as a search path, and look for dlls referenced but not found. It's a bit of guesswork, as most of these are system libraries that depends doesn't know about, but if any of these are from the MXE build, you'll need to add it the the `MXE_DLLS` in the minsky Makefile.

## Debugging
With MXE, there is very limited debugging facilities. Essentially all that is possible is to add print statements. To build Minsky such that messages to the console are visible, do
~~~~
make MXE=1 DEBUG=1 OPT="-O3 -NDEBUG"
~~~~
Note that without the OPT parameter, the compiler core dumps.
