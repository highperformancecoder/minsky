.SUFFIXES: .xcd .rcd .tcd .gch .gcd $(SUFFIXES)

# location of minsky executable when building mac-dist
MAC_DIST_DIR=minsky.app/Contents/MacOS

SF_WEB=hpcoder@web.sourceforge.net:/home/project-web/minsky/htdocs

# location of TCL and TK libraries 
TCL_PREFIX=$(shell grep TCL_PREFIX $(call search,lib*/tclConfig.sh) | cut -f2 -d\')
TCL_VERSION=$(shell grep TCL_VERSION $(call search,lib*/tclConfig.sh) | cut -f2 -d\')
TCL_LIB=$(dir $(shell find $(TCL_PREFIX) -name init.tcl -path "*/tcl$(TCL_VERSION)*" -print))
TK_LIB=$(dir $(shell find $(TCL_PREFIX) -name tk.tcl -path "*/tk$(TCL_VERSION)*" -print))

# root directory for ecolab include files and libraries
ECOLAB_HOME=$(shell pwd)/ecolab

ifeq ($(OS),Darwin)
MAKEOVERRIDES+=MAC_OSX_TK=1
endif

ifdef MXE
MAKEOVERRIDES+=MXE_PREFIX=x86_64-w64-mingw32.shared
endif

ifneq ($(MAKECMDGOALS),clean)
# make sure EcoLab is built first, even before starting to include Makefiles
build_ecolab:=$(shell cd ecolab; if $(MAKE) $(MAKEOVERRIDES) $(JOBS) all-without-models >build.log 2>&1; then echo "ecolab built"; fi)
$(warning $(build_ecolab))
ifneq ($(build_ecolab),ecolab built)
$(error "Making ecolab failed: check ecolab/build.log")
endif
include $(ECOLAB_HOME)/include/Makefile
endif

ifndef MXE
ifdef GCC
CPLUSPLUS=g++
LINK=g++
else
# default to clang if present
HAVE_CLANG=$(shell if which clang++>/dev/null; then echo 1; fi)
ifeq ($(HAVE_CLANG),1)
CPLUSPLUS=clang++
LINK=clang++
$(warning clang selected)
endif
endif
endif

MAKEOVERRIDES+=FPIC=1 CLASSDESC=$(shell pwd)/ecolab/bin/classdesc CPLUSPLUS=$(CPLUSPLUS)
ifneq ($(MAKECMDGOALS),clean)
build_RavelCAPI:=$(shell cd RavelCAPI && $(MAKE) $(JOBS) $(MAKEOVERRIDES)) 
$(warning $(build_RavelCAPI))
endif


ifdef DISTCC
CPLUSPLUS=distcc
# number of jobs to do sub-makes
JOBS=-j 20
else
# number of jobs to do sub-makes
JOBS=-j 4
endif

ifneq ($(MAKECMDGOALS),clean)
  HAVE_NODE=$(shell if which node >/dev/null 2>&1; then echo 1; fi)
  $(warning have node=$(HAVE_NODE))
  ifeq ($(HAVE_NODE),1)
    NODE_API=
    ifeq ($(OS),Darwin)
      NODE_HEADER=/usr/local/include/node
    else
      NODE_VERSION=$(shell node -v|sed -E -e 's/[^0-9]*([0-9]*).*/\1/')
      ifdef MXE
        NODE_HEADER=/usr/include/node$(NODE_VERSION)
        NODE_API+=node-api.o
      else
        ifeq ($(OS),CYGWIN)
          NODE_API+=node-api.o
        endif
        NODE_HEADER=$(call search,include/node$(NODE_VERSION))
        ifeq ($(NODE_HEADER),) # Ubuntu stashes node headers at /usr/include/nodejs
          NODE_HEADER=$(call search,include/node)
        endif
      endif
    endif
    # if we haven't found an installed version of the Node SDK, then
    # check if it has been copied into node-addon-api, as is done with
    # release tarballs
    ifeq ($(NODE_HEADER),)
      ifeq ($(words $(wildcard  node_modules/node-addon-api/node_api.h)),0)
        $(error Can't find node header files')
      endif
    else
      NODE_FLAGS+=-I$(NODE_HEADER)
    endif
    FLAGS+=-fno-omit-frame-pointer
    NODE_FLAGS+=-Inode_modules/node-addon-api
    NODE_FLAGS+='-DV8_DEPRECATION_WARNINGS' '-DV8_IMMINENT_DEPRECATION_WARNINGS'
    NODE_FLAGS+='-D__STDC_FORMAT_MACROS' '-DNAPI_CPP_EXCEPTIONS'

    $(warning node flags=$(NODE_FLAGS))

    FLAGS+=$(NODE_FLAGS) 
    # ensure node-addon-api installed
    ifeq ($(words $(wildcard node_modules/node-addon-api)),0)
      npm_install:=$(shell npm install)
    endif

  endif
endif

# override the install prefix here
PREFIX=/usr/local

# override MODLINK to remove tclmain.o, which allows us to provide a
# custom one that picks up its scripts from a relative library
# directory
MODLINK=$(LIBMODS:%=$(ECOLAB_HOME)/lib/%)
MODEL_OBJS=autoLayout.o cairoItems.o canvas.o CSVDialog.o dataOp.o godleyIcon.o godleyTable.o godleyTableWindow.o godleyTab.o grid.o group.o item.o itemTab.o intOp.o lasso.o lock.o minsky.o operation.o operationRS.o operationRS1.o  operationRS2.o panopticon.o parameterTab.o plotTab.o plotWidget.o port.o ravelWrap.o renderNativeWindow.o selection.o sheet.o SVGItem.o switchIcon.o userFunction.o userFunction_units.o variableInstanceList.o variable.o variablePane.o windowInformation.o wire.o 
ENGINE_OBJS=coverage.o clipboard.o derivative.o equationDisplay.o equations.o evalGodley.o evalOp.o flowCoef.o \
	godleyExport.o latexMarkup.o valueId.o variableValue.o node_latex.o node_matlab.o CSVParser.o \
	minskyTensorOps.o mdlReader.o saver.o rungeKutta.o
SCHEMA_OBJS=schema3.o schema2.o schema1.o schema0.o schemaHelper.o variableType.o \
	operationType.o a85.o

ifeq ($(CPLUSPLUS),g++)
PRECOMPILED_HEADERS=model/minsky.gch
endif

GUI_TK_OBJS=tclmain.o minskyTCL.o
RESTSERVICE_OBJS=minskyRS.o RESTMinsky.o

ifeq ($(OS),Darwin)
FLAGS+=-DENABLE_DARWIN_EVENTS -DMAC_OSX_TK
LIBS+=-Wl,-framework -Wl,Security -Wl,-headerpad_max_install_names
MODEL_OBJS+=getContext.o
endif

ALL_OBJS=$(MODEL_OBJS) $(ENGINE_OBJS) $(SCHEMA_OBJS) $(GUI_TK_OBJS) $(TENSOR_OBJS) $(RESTSERVICE_OBJS) RESTService.o httpd.o addon.o typescriptAPI.o


ifneq ($(GUI_TK),1)

EXES=RESTService/minsky-RESTService$(EXE) RESTService/minsky-httpd$(EXE)
ifeq ($(HAVE_NODE),1)
EXES+=gui-js/node-addons/minskyRESTService.node
endif
ifndef MXE
EXES+=RESTService/typescriptAPI
ifndef GUI_TK
ifeq ($(OS), Linux)
EXES+=gui-tk/minsky$(EXE)
endif
endif
endif

else
EXES=gui-tk/minsky$(EXE)
endif


DYLIBS=libminsky.$(DL) libminskyEngine.$(DL) libcivita.$(DL)
MINSKYLIBS=-lminsky -lminskyEngine -lcivita


ifndef NOWERROR
ifndef MXE
FLAGS+=-Werror
endif
endif

FLAGS+=-std=c++14 -Ischema -Iengine -Imodel -Icertify/include -IRESTService -IRavelCAPI/civita -IRavelCAPI -DCLASSDESC -DUSE_UNROLLED -DCLASSDESC_ARITIES=0xf $(OPT) -UECOLAB_LIB -DECOLAB_LIB=\"library\" -DJSON_PACK_NO_FALL_THROUGH_TO_STREAMING -Wno-unused-local-typedefs -Wno-pragmas -Wno-deprecated-declarations -Wno-unused-command-line-argument -Wno-unknown-warning-option -Wno-attributes
# NB see #1486 - we need to update the use of rsvg, then we can remove -Wno-deprecated-declarations
#-fvisibility-inlines-hidden

ifeq ($(DEBUG), 1)
FLAGS+=-Wp,-D_GLIBCXX_ASSERTIONS
endif

VPATH= schema model engine gui-tk RESTService RavelCAPI/civita RavelCAPI $(ECOLAB_HOME)/include 

.h.xcd:
# xml_pack/unpack need to -typeName option, as well as including privates
	$(CLASSDESC) -typeName -nodef -respect_private -I $(CDINCLUDE) \
	-I $(ECOLAB_HOME)/include -I $(CERTIFY_HOME)/certify -I RESTService -i $< \
	xml_pack xml_unpack xsd_generate json_pack json_unpack >$@

.h.rcd:
	$(CLASSDESC) -typeName -nodef -use_mbr_pointers -onbase -overload -respect_private \
	-I $(CDINCLUDE) -I $(ECOLAB_HOME)/include -I RESTService -i $< \
	RESTProcess >$@

.h.gch:
	$(CPLUSPLUS) -c $(FLAGS) $(CXXFLAGS) $(OPT) -o $@ $<

.h.gcd:
	$(CPLUSPLUS) $(FLAGS)  $(CXXFLAGS) -MM -MG $< >$@
	sed -i -e 's/.*\.o:/'$(*D)'\/'$(*F)'.gch:/' $@

.h.tcd:
	$(CLASSDESC) -typeName -nodef -use_mbr_pointers -onbase -overload -respect_private \
	-I $(CDINCLUDE) -I $(ECOLAB_HOME)/include -I RESTService -i $< \
	typescriptAPI >$@

# assorted performance profiling stuff using gperftools, or Russell's custom
# timer calipers
ifdef MEMPROFILE
LIBS+=-ltcmalloc
endif
ifdef CPUPROFILE
OPT+=-g
LIBS+=-lprofiler
endif

TESTS=
ifdef AEGIS
# ensure all exes get built in AEGIS mode
TESTS=tests 
# enable TCL coverage testing
FLAGS+=-DTCL_COV -Werror=delete-non-virtual-dtor -Wno-unknown-pragmas
endif

ifdef MXE
BOOST_EXT=-mt-x64
EXE=.exe
DL=dll
FLAGS+=-D_WIN32 -DUSE_UNROLLED -Wa,-mbig-obj
# DLLS that need to be copied into the binary directory
MXE_DLLS=libboost_filesystem-mt-x64 libboost_thread-mt-x64 libbz2 libcairo-2 libcroco-0 libcrypto-3-x64 libexpat-1 \
libffi-7 libfontconfig-1 libfreetype-6 libfribidi-0 libgcc_s_seh-1 libgdk_pixbuf-2 libgio-2 libglib-2 libgmodule-2 \
libgobject-2 libgsl-25 libgslcblas-0 libharfbuzz-0 libiconv-2 libintl-8 libjpeg-9 liblzma-5 libpango-1 libpangocairo-1 \
libpangoft2-1 libpangowin32-1 libpcre-1 libpixman-1-0 libpng16-16 libreadline8 librsvg-2-2 libssl-3-x64 libstdc++-6 \
libtermcap libwinpthread-1 libxml2-2 tcl86 zlib1
BINDIR=$(subst bin,$(MXE_PREFIX)/bin,$(dir $(shell which $(CPLUSPLUS))))
$(warning $(BINDIR))
DLLS=$(wildcard $(MXE_DLLS:%=$(BINDIR)/%*.dll))
else
EXE=
DL=so
BOOST_EXT=
# try to autonomously figure out which boost extension we should be using
  ifeq ($(shell if $(CPLUSPLUS) test/testmain.cc $(LIBS) -lboost_system>&/dev/null; then echo 1; else echo 0; fi),0)
    ifeq ($(shell if $(CPLUSPLUS) test/testmain.cc $(LIBS) -lboost_system-mt>&/dev/null; then echo 1; else echo 0; fi),1)
      BOOST_EXT=-mt
    else
      $(warning cannot figure out boost extension) 
    endif
  endif
 $(warning Boost extension=$(BOOST_EXT))
endif

ifeq ($(OS),CYGWIN)
FLAGS+=-Wa,-mbig-obj -Wl,-x -Wl,--oformat,pe-bigobj-x86-64
endif

LIBS+=	-LRavelCAPI -lravelCAPI -LRavelCAPI/civita -lcivita \
	-lboost_system$(BOOST_EXT) -lboost_regex$(BOOST_EXT) \
	-lboost_date_time$(BOOST_EXT) -lboost_program_options$(BOOST_EXT) \
	-lboost_filesystem$(BOOST_EXT) -lboost_thread$(BOOST_EXT) -lgsl -lgslcblas -lssl -lcrypto

ifdef MXE
LIBS+=-lcrypt32 -lbcrypt -lshcore
else
LIBS+=-lclipboard -lxcb -lX11 -ldl
endif

# RSVG dependencies calculated here
FLAGS+=$(shell $(PKG_CONFIG) --cflags librsvg-2.0)
LIBS+=$(shell $(PKG_CONFIG) --libs librsvg-2.0)

GUI_LIBS=
# disable a deprecation warning that comes from Wt
FLAGS+=-DBOOST_SIGNALS_NO_DEPRECATION_WARNING

ifndef AEGIS
default: $(EXES) gui-js/libs/shared/src/lib/backend/minsky.ts
	-$(CHMOD) a+x *.tcl *.sh *.pl
ifeq ($(OS),Linux)
ifdef MXE
	sh mkWindowsDist.sh
else
ifndef OBS
	cd gui-js; npm run export:package:linux
endif
endif
endif
ifeq ($(OS),Darwin)
	cd gui-js; npm run export:package:macosx
endif
endif

#chmod command is to counteract AEGIS removing execute privelege from scripts
all: $(EXES) $(TESTS) minsky.xsd 
# only perform link checking if online
# linkchecker not currently working! :(
#ifndef TRAVIS
#	if ping -c 1 www.google.com; then linkchecker -f linkcheckerrc gui-tk/library/help/minsky.html; fi
#endif
	-$(CHMOD) a+x *.tcl *.sh *.pl


$(ALL_OBJS): $(PRECOMPILED_HEADERS)

# this dependency is not worked out automatically because they're hidden by a #ifdef in minsky_epilogue.h
$(MODEL_OBJS): plot.xcd signature.xcd

ifneq ($(MAKECMDGOALS),clean)
include $(ALL_OBJS:.o=.d) $(PRECOMPILED_HEADERS:.gch=.gcd)
endif

ifdef MXE
ifndef DEBUG
# This option removes the black window, but this also prevents being
# able to    type TCL commands on the command line, so only use it for
# release builds. Doesn't seem to work on MXE, though - see ticket #456
FLAGS+=-DCONSOLE
FLAGS+=-mwindows
else
# do not include symbols, as obscure Windows bug causes link-time
# large text section failure. In any case, we do not have a usable
# symbolic debugger available for this build
OPT=-O0
endif
ifdef RAVEL
GUI_TK_OBJS+=RavelLogo.o
else
GUI_TK_OBJS+=MinskyLogo.o
endif
WINDRES=$(MXE_PREFIX)-windres
endif

MinskyLogo.o: MinskyLogo.rc gui-tk/icons/MinskyLogo.ico
	$(WINDRES) -O coff -i $< -o $@

RavelLogo.o: RavelLogo.rc gui-tk/icons/RavelLogo.ico
	$(WINDRES) -O coff -i $< -o $@

getContext.o: getContext.cc
	g++ -ObjC++ $(FLAGS) -I/opt/local/include -Iinclude -c $< -o $@

gui-tk/minsky$(EXE): $(GUI_TK_OBJS)  $(MODEL_OBJS) $(SCHEMA_OBJS) $(ENGINE_OBJS) $(TENSOR_OBJS)
	$(LINK) $(FLAGS) $^ $(MODLINK) -L/opt/local/lib/db48 $(LIBS) $(GUI_LIBS) -o $@
	-find . \( -name "*.cc" -o -name "*.h" \) -print |etags -
ifdef MXE
# make a local copy the TCL libraries
	rm -rf gui-tk/library/{tcl,tk}
	cp -r $(TCL_LIB) gui-tk/library/tcl
	cp -r $(TK_LIB) gui-tk/library/tk
endif

RESTService/minsky-RESTService$(EXE): RESTService.o  $(RESTSERVICE_OBJS) $(MODEL_OBJS) $(SCHEMA_OBJS) $(ENGINE_OBJS) $(TENSOR_OBJS)
	$(LINK) $(FLAGS) $^ -L/opt/local/lib/db48 $(LIBS) -o $@

RESTService/minsky-httpd$(EXE): httpd.o $(RESTSERVICE_OBJS) $(MODEL_OBJS) $(SCHEMA_OBJS) $(ENGINE_OBJS) $(TENSOR_OBJS)
	$(LINK) $(FLAGS) $^ -L/opt/local/lib/db48 $(LIBS) -o $@

RESTService/typescriptAPI: typescriptAPI.o $(RESTSERVICE_OBJS) $(MODEL_OBJS) $(SCHEMA_OBJS) $(ENGINE_OBJS) $(TENSOR_OBJS)
	$(LINK) $(FLAGS) $^  $(LIBS) -o $@


ifndef MXE
gui-js/libs/shared/src/lib/backend/minsky.ts: RESTService/typescriptAPI
	RESTService/typescriptAPI > $@
endif

gui-tk/helpRefDb.tcl: $(wildcard doc/minsky/*.html)
	rm -f $@
	perl makeRefDb.pl doc/minsky/*.html >$@

doc/minsky/labels.pl: $(wildcard doc/*.tex)
	cd doc; sh makedoc.sh

gui-tk/library/help: doc/minsky/labels.pl doc/minsky.html
	rm -rf $@/*
	mkdir -p $@/minsky
	find doc/minsky \( -name "*.html" -o -name "*.css" -o -name "*.png" \) -exec cp {} $@/minsky \;
	cp -r -f doc/minsky.html $@
ifndef TRAVIS
	linkchecker -f linkcheckerrc $@/minsky.html
endif

doc: gui-tk/library/help gui-tk/helpRefDb.tcl

# N-API node embedded RESTService
gui-js/node-addons/minskyRESTService.node: addon.o  $(NODE_API) $(RESTSERVICE_OBJS) $(MODEL_OBJS) $(SCHEMA_OBJS) $(ENGINE_OBJS) $(TENSOR_OBJS)
	mkdir -p gui-js/node-addons
ifdef MXE
	$(LINK) -shared -o $@ $^ $(LIBS)
	mkdir -p gui-js/dynamic_libraries
	cp $(DLLS) gui-js/dynamic_libraries
else
ifeq ($(OS),Darwin)
	c++ -bundle -undefined dynamic_lookup -Wl,-no_pie -Wl,-search_paths_first -mmacosx-version-min=10.13 -arch x86_64 -stdlib=libc++  -o $@  $^ $(LIBS)
else
	$(LINK) -shared -pthread -rdynamic -m64  -Wl,-soname=minskyRESTService.node -o $@ -Wl,--start-group $^ -Wl,--end-group $(LIBS)
endif
endif

RESTService/dummy-addon.node: dummy-addon.o $(NODE_API)
ifdef MXE
	$(LINK) -shared -o $@ $^
else
	$(LINK) -shared -pthread -rdynamic -m64  -Wl,-soname=addon.node -o $@ -Wl,--start-group $^ -Wl,--end-group 
endif

addon.o: addon.cc
	$(CPLUSPLUS) $(FLAGS) $(CXXFLAGS) $(OPT) -c -o $@ $<

dummy-addon.o: dummy-addon.cc
	$(CPLUSPLUS) $(NODE_FLAGS) $(FLAGS) $(CXXFLAGS) $(OPT) -c -o $@ $<

node-api.o: node-api.cc
	$(CPLUSPLUS) $(NODE_FLAGS) $(FLAGS) $(CXXFLAGS) $(OPT) -c -o $@ $<

$(EXES):

tests: $(EXES)
	cd test; $(MAKE)

BASIC_CLEAN=rm -rf *.o *~ "\#*\#" core *.d *.cd *.rcd *.tcd *.xcd *.gcda *.gcno *.so *.dll *.dylib

clean:
	-$(BASIC_CLEAN) minsky.xsd
	-rm -f $(EXES)
	-cd test && $(MAKE)  clean
	-cd gui-tk &&  $(BASIC_CLEAN)
	-cd model && $(BASIC_CLEAN)
	-cd engine && $(BASIC_CLEAN)
	-cd schema && $(BASIC_CLEAN)
	-cd ecolab && $(MAKE) clean
	-cd RavelCAPI && $(MAKE) clean

mac-dist: gui-tk/minsky gui-js/node-addons/minskyRESTService.node
# create executable in the app package directory. Make it 32 bit only
#	mkdir -p minsky.app/Contents/MacOS
#	sh -v mkMacDist.sh
	sh -v mkMacRESTService.sh

minsky.xsd: gui-tk/minsky
	gui-tk/minsky exportSchema.tcl 3

upload-schema: minsky.xsd
	scp minsky.xsd $(SF_WEB)

install: $(EXES)
ifneq ($(GUI_TK),1)
	mkdir -p $(PREFIX)/bin $(PREFIX)/lib/minsky/resources/node-addons
	cp RESTService/minsky-RESTService $(PREFIX)/bin
	cp RESTService/minsky-httpd $(PREFIX)/bin
	cp gui-js/node-addons/*.node $(PREFIX)/lib/minsky/resources/node-addons
ifndef GUI_TK
ifeq ($(OS), "Linux")
ifndef MXE
	cp gui-tk/minsky$(EXE) $(PREFIX)/bin
	cp -r gui-tk/*.tcl gui-tk/accountingRules gui-tk/icons gui-tk/library $(PREFIX)/lib/minsky
endif
endif
endif
else
	mkdir -p $(PREFIX)/bin $(PREFIX)/lib/minsky
	cp gui-tk/minsky$(EXE) $(PREFIX)/bin
	cp -r gui-tk/*.tcl gui-tk/accountingRules gui-tk/icons gui-tk/library $(PREFIX)/lib/minsky
endif



# runs the regression tests
sure: all tests
	xvfb-run bash test/runtests.sh

# produce doxygen annotated web pages
doxydoc: $(wildcard *.h) $(wildcard *.cc) \
	$(wildcard GUI/*.h) $(wildcard GUI/*.cc) \
	$(wildcard engine/*.h) $(wildcard engine/*.cc) \
	$(wildcard schema/*.h) $(wildcard schema/*.cc) Doxyfile
	 doxygen

# upload doxygen webpages to SF
install-doxydoc: doxydoc
	rsync -r -z --progress --delete doxydoc $(SF_WEB)


# upload manual to SF
install-manual: doc/minsky/labels.pl
	rsync -r -z --progress --delete doc/minsky.html doc/minsky $(SF_WEB)/manual

# run this after every full release
install-release: install-doxydoc install-manual upload-schema

# run the regression suite checking for the TCL code coverage
tcl-cov:
	rm -f minsky.cov minsky.cov.{pag,dir} coverage.o
	-env MINSKY_COV=`pwd`/minsky.cov $(MAKE) AEGIS=1 sure
	cd test; $(MAKE) tcl-cov
	sh test/run-tcl-cov.sh

dist:
	sh makeDist.sh $(NODE_HEADER)

js-dist:
	sh makeJsDist.sh

lcov:
	$(MAKE) clean
	-$(MAKE) GCOV=1 tests
	lcov -i -c -d . --no-external -o lcovi.info
# ensure schema export code is exercised
	-$(MAKE) GCOV=1 minsky.xsd
	-$(MAKE) GCOV=1 sure
	lcov -c -d .  --no-external -o lcovt.info
	lcov -a lcovi.info -a lcovt.info -o lcov.info
	lcov -r lcov.info */ecolab/* "*.cd" "*.xcd" "*.rcd" "*.tcd" -o lcovr.info 
	genhtml -o coverage lcovr.info

compile_commands.json: Makefile
	rm *.o
	bear $(MAKE)

clang-tidy: compile_commands.json
	run-clang-tidy

compile-ts:
	cd gui-js && npx tsc | sed -e 's/\x1b\[[0-9;]*m//g'|sed -e 's/(\([0-9]*\),[0-9]*)/:\1/g'

windows-package:
	sh mkWindowsDist.sh
