## MXE

- The Windows distributable is built using the MXE cross-compiler environment on Linux. 

- Use the [latest tag of MXE](https://github.com/mxe/mxe/tags).
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
- Compile libclipboard from source code:
  * `git clone git@github.com:jtanx/libclipboard.git`
  * `cd libclipboard`
  * `x86_64-w64-mingw32.shared-cmake .`
  * `make install`

- Then compile Minsky with

~~~~
    make MXE=1
~~~~
- Once minskyRESTService.node is compiled, 
~~~~~
    cd gui-js
    npm run export:package:windows
~~~~~
- Installer will be found in `gui-js/dist/executable`

## Debugging
With MXE, there is very limited debugging facilities. Essentially all that is possible is to add print statements. To build Minsky such that messages to the console are visible, do
~~~~
make MXE=1 DEBUG=1 OPT="-O3 -NDEBUG"
~~~~
Note that without the OPT parameter, the compiler core dumps.
