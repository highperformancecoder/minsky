## MXE

- The Windows distributable is built using the MXE cross-compiler environment on Linux. 

- Use the mxe-for-minsky branch of [my MXE fork](https://github.com/highperformancecoder/mxe/tree/mxe-for-minsky). (build-2022-04-09 at time of writing)

- You also need to patch plugins/tcl.tk/tk.mk to be built statically, as the shared build blows up the Windows linker.
  ```diff
  @@ -22,10 +22,10 @@ define $(PKG)_BUILD
     cd '$(SOURCE_DIR)/win' && autoreconf -fi
     cd '$(BUILD_DIR)' && '$(SOURCE_DIR)/win/configure' \
         $(MXE_CONFIGURE_OPTS) \
  -        --enable-threads \
  +        --enable-threads --disable-shared \
         --with-tcl='$(PREFIX)/$(TARGET)/lib' \
         $(if $(findstring x86_64,$(TARGET)), --enable-64bit) \
  -        CFLAGS='-D__MINGW_EXCPT_DEFINE_PSDK'
  +        CFLAGS='-D__MINGW_EXCPT_DEFINE_PSDK -DSTATIC_BUILD=1'
     $(MAKE) -C '$(BUILD_DIR)' -j '$(JOBS)' LIBS='-lmincore -lnetapi32 -lz -ltclstub86 -limm32 -lcomctl32 -luuid -lole32 -lgdi32 -lcomdlg32'
     $(MAKE) -C '$(BUILD_DIR)' -j 1 install
  endef
  ```

- `make MXE_TARGETS=x86_64-w64-mingw32.shared MXE_PLUGIN_DIRS=plugins/tcl.tk boost cairo tcl tk gsl pango librsvg openssl readline ncurses`
- For Minsky 2.20 or less, `MXE_TARGETS=i686-w64-mingw32.static`

- Install necessary prerequisites from your package manager as required by the above line (eg flex, gperf, intltool, scons).
- Ensure the usr/bin directory of the cloned repo is in your PATH.
- Compile libclipboard from source code: - [not needed after 3.2.0-alpha.15](https://sourceforge.net/p/minsky/tickets/1662/)
  * `git clone git@github.com:jtanx/libclipboard.git`
  * `cd libclipboard`
  * `x86_64-w64-mingw32.shared-cmake .`
  * `make install`

- Add a code signing certificate (.pfx file), and specify its name in `WINDOWS_SIGN_CERTIFICATE_NAME` environment   variable and password in WINDOWS_SIGN_TOKEN_PASSWORD environment variable.

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
