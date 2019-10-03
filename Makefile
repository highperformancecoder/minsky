.SUFFIXES: .xcd $(SUFFIXES)

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

include $(ECOLAB_HOME)/include/Makefile

# override the install prefix here
PREFIX=/usr/local


# override MODLINK to remove tclmain.o, which allows us to provide a
# custom one that picks up its scripts from a relative library
# directory
MODLINK=$(LIBMODS:%=$(ECOLAB_HOME)/lib/%)
MODEL_OBJS=wire.o item.o group.o minsky.o port.o operation.o variable.o switchIcon.o godleyTable.o cairoItems.o godleyIcon.o SVGItem.o plotWidget.o canvas.o panopticon.o godleyTableWindow.o ravelWrap.o sheet.o CSVDialog.o selection.o
ENGINE_OBJS=coverage.o derivative.o equationDisplay.o equations.o evalGodley.o evalOp.o flowCoef.o godleyExport.o \
	latexMarkup.o variableValue.o xvector.o node_latex.o node_matlab.o CSVParser.o
SERVER_OBJS=database.o message.o websocket.o databaseServer.o
SCHEMA_OBJS=schema2.o schema1.o schema0.o variableType.o operationType.o a85.o
#schema0.o 
GUI_TK_OBJS=tclmain.o minskyTCL.o
RESTSERVICE_OBJS=RESTService.o

ALL_OBJS=$(MODEL_OBJS) $(ENGINE_OBJS) $(SERVER_OBJS) $(RESTSERVICE_OBJS) $(SCHEMA_OBJS) $(GUI_TK_OBJS)

EXES=gui-tk/minsky $(SERVER_OBJS) RESTService/RESTService 
#EXES=gui-tk/minsky server/server

ifeq ($(OS),Darwin)
FLAGS+=-DENABLE_DARWIN_EVENTS -DMAC_OSX_TK
endif

FLAGS+=-std=c++11 -Ischema -Iengine -Imodel -IRESTService $(OPT) -UECOLAB_LIB -DECOLAB_LIB=\"library\" -Wno-unused-local-typedefs

VPATH= schema model engine gui-tk server RESTService $(ECOLAB_HOME)/include

.h.xcd:
# xml_pack/unpack need to -typeName option, as well as including privates
	$(CLASSDESC) -typeName -nodef -respect_private -I $(CDINCLUDE) \
	-I $(ECOLAB_HOME)/include -I RESTService -i $< xml_pack xml_unpack xsd_generate \
	json_pack json_unpack RESTProcess >$@

# assorted performance profiling stuff using gperftools, or Russell's custom
# timer calipers
ifdef MEMPROFILE
LIBS+=-ltcmalloc
endif
ifdef CPUPROFILE
LIBS+=-lprofiler
endif

TESTS=
ifdef AEGIS
# ensure all exes get built in AEGIS mode
TESTS=tests 
# enable TCL coverage testing
FLAGS+=-DTCL_COV -Werror=delete-non-virtual-dtor
endif

ifdef MXE
BOOST_EXT=-mt
EXE=.exe
else
EXE=
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

LIBS+= -ljson_spirit \
	-lboost_system$(BOOST_EXT) -lboost_regex$(BOOST_EXT) \
	-lboost_date_time$(BOOST_EXT) -lboost_program_options$(BOOST_EXT) \
	-lboost_filesystem$(BOOST_EXT) -lgsl -lgslcblas  

ifdef MXE
LIBS+=-lboost_thread_win32$(BOOST_EXT)
else
LIBS+=-lboost_thread$(BOOST_EXT) 
endif

ifdef CPUPROFILE
OPT+=-g
LIBS+=-lprofiler
endif

# RSVG dependencies calculated here
FLAGS+=$(shell $(PKG_CONFIG) --cflags librsvg-2.0)
LIBS+=$(shell $(PKG_CONFIG) --libs librsvg-2.0)

GUI_LIBS=
#SERVER_LIBS=-lwebsocketpp -lsoci_core 
SERVER_LIBS=-lsoci_core 
# disable a deprecation warning that comes from Wt
FLAGS+=-DBOOST_SIGNALS_NO_DEPRECATION_WARNING

ifndef AEGIS
# just build the Minsky executable
default: gui-tk/minsky$(EXE)
	-$(CHMOD) a+x *.tcl *.sh *.pl
endif

#chmod command is to counteract AEGIS removing execute privelege from scripts
all: $(EXES) $(TESTS) minsky.xsd 
# only perform link checking if online
# linkchecker not currently working! :(
#ifndef TRAVIS
#	if ping -c 1 www.google.com; then linkchecker -f linkcheckerrc gui-tk/library/help/minsky.html; fi
#endif
	-$(CHMOD) a+x *.tcl *.sh *.pl

.PHONY: ecolab
ecolab:
	cd ecolab; $(MAKE) all-without-models

$(ECOLAB_HOME)/include/Makefile.config: ecolab

$(ALL_OBJS): ecolab

include $(ALL_OBJS:.o=.d)

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
GUI_TK_OBJS+=MinskyLogo.o
WINDRES=$(MXE_PREFIX)-windres
endif

MinskyLogo.o: MinskyLogo.rc gui-tk/icons/MinskyLogo.ico
	$(WINDRES) -O coff -i $< -o $@

gui-tk/minsky$(EXE): $(GUI_TK_OBJS) $(MODEL_OBJS) $(ENGINE_OBJS) $(SCHEMA_OBJS)
	$(LINK) $(FLAGS) $^ $(MODLINK) -L/opt/local/lib/db48 -L. $(LIBS) $(GUI_LIBS) -o $@
	-find . \( -name "*.cc" -o -name "*.h" \) -print |etags -
ifdef MXE
# make a local copy the TCL libraries
	rm -rf gui-tk/library/{tcl,tk}
	cp -r $(TCL_LIB) gui-tk/library/tcl
	cp -r $(TK_LIB) gui-tk/library/tk
endif

server/server: tclmain.o $(ENGINE_OBJS) $(SCHEMA_OBJS) $(SERVER_OBJS) $(GUI_OBJS)
	$(LINK) $(FLAGS) $^ $(MODLINK) -L/opt/local/lib/db48 -L. $(LIBS)  $(SERVER_LIBS) -o $@
	-ln -sf `pwd`/GUI/library server

RESTService/RESTService: $(RESTSERVICE_OBJS) $(MODEL_OBJS) $(ENGINE_OBJS) $(SCHEMA_OBJS)
	$(LINK) $(FLAGS) $^ -L/opt/local/lib/db48 -L. $(LIBS) -o $@

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

tests: $(EXES)
	cd test; $(MAKE)

BASIC_CLEAN+=*.xcd

clean:
	find . -name "*,D" -exec rm {} \;
	$(BASIC_CLEAN) minsky.xsd *.gcda *.gcno
	rm -f $(EXES)
	cd test; $(MAKE) clean
	cd gui-tk; $(BASIC_CLEAN)
	cd model; $(BASIC_CLEAN)
	cd engine; $(BASIC_CLEAN)
	cd schema; $(BASIC_CLEAN)
	cd gui-wt; $(BASIC_CLEAN)
	cd server; $(BASIC_CLEAN)
	cd ecolab; $(MAKE) clean

mac-dist: gui-tk/minsky
# create executable in the app package directory. Make it 32 bit only
	mkdir -p minsky.app/Contents/MacOS
	sh -v mkMacDist.sh

minsky.xsd: gui-tk/minsky
	gui-tk/minsky exportSchema.tcl 2

upload-schema: minsky.xsd
	scp minsky.xsd $(SF_WEB)

install: gui-tk/minsky$(EXE)
	mkdir -p $(PREFIX)/bin
	cp gui-tk/minsky$(EXE) $(PREFIX)/bin
	mkdir -p $(PREFIX)/lib/minsky
	cp -r gui-tk/*.tcl gui-tk/accountingRules gui-tk/icons gui-tk/library $(PREFIX)/lib/minsky



# runs the regression tests
sure: all tests
	bash test/runtests.sh

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

# run the regression suite checking for the TCL code coverage
tcl-cov:
	rm -f minsky.cov minsky.cov.{pag,dir} coverage.o
	-env MINSKY_COV=`pwd`/minsky.cov $(MAKE) AEGIS=1 sure
	sh test/run-tcl-cov.sh

MINSKY_VERSION=$(shell git describe)

dist:
	git archive --format=tar.gz --prefix=Minsky-$(MINSKY_VERSION)/ HEAD -o /tmp/Minsky-$(MINSKY_VERSION).tar.gz

lcov:
	$(MAKE) clean
	-$(MAKE) GCOV=1 tests
	lcov -i -c -d . --no-external -o lcovi.info
	-$(MAKE) GCOV=1 sure
	lcov -c -d . --no-external -o lcovt.info
	lcov -a lcovi.info -a lcovt.info -o lcov.info
	genhtml -o coverage lcov.info

compile_commands.json: Makefile
	$(MAKE) clean
	bear $(MAKE)

clang-tidy: compile_commands.json
	run-clang-tidy
