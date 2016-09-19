.SUFFIXES: .xcd $(SUFFIXES)

# location of minsky executable when building mac-dist
MAC_DIST_DIR=minsky.app/Contents/MacOS

# Use the TCL stub libraries to be consistent with TkTable

# location of TCL and TK libraries 
TCL_PREFIX=$(shell grep TCL_PREFIX $(call search,lib*/tclConfig.sh) | cut -f2 -d\')
TCL_VERSION=$(shell grep TCL_VERSION $(call search,lib*/tclConfig.sh) | cut -f2 -d\')
TCL_LIB=$(dir $(shell find $(TCL_PREFIX) -name init.tcl -path "*/tcl$(TCL_VERSION)*" -print))
TK_LIB=$(dir $(shell find $(TCL_PREFIX) -name tk.tcl -path "*/tk$(TCL_VERSION)*" -print))

# root directory for ecolab include files and libraries
ifdef MXE
ECOLAB_HOME=$(HOME)/usr/mxe/ecolab
else
ifeq ($(shell ls $(HOME)/usr/ecolab/include/ecolab.h),$(HOME)/usr/ecolab/include/ecolab.h)
ECOLAB_HOME=$(HOME)/usr/ecolab
else
ECOLAB_HOME=/usr/local/ecolab
endif
endif

include $(ECOLAB_HOME)/include/Makefile

# override the install prefix here
PREFIX=/usr/local


# override MODLINK to remove tclmain.o, which allows us to provide a
# custom one that picks up its scripts from a relative library
# directory
MODLINK=$(LIBMODS:%=$(ECOLAB_HOME)/lib/%)
GUI_OBJS=minskyTCL.o minsky.o godley.o portManager.o wire.o \
	variable.o variableManager.o variableValue.o \
	operation.o plotWidget.o cairoItems.o SVGItem.o equationDisplayItem.o \
	godleyIcon.o groupIcon.o inGroupTest.o opVarBaseAttributes.o \
	switchIcon.o
MODEL_OBJS=wire.o item.o group.o minsky.o port.o operation.o variable.o switchIcon.o godley.o cairoItems.o godleyIcon.o SVGItem.o plotWidget.o equationDisplayItem.o
ENGINE_OBJS=coverage.o derivative.o equationDisplay.o equations.o evalGodley.o evalOp.o flowCoef.o godleyExport.o \
	latexMarkup.o variableValue.o 
SERVER_OBJS=database.o message.o websocket.o databaseServer.o
SCHEMA_OBJS=schema1.o variableType.o operationType.o
#schema0.o 
GUI_TK_OBJS=tclmain.o minskyTCL.o minskyCairoItem.o

ALL_OBJS=$(MODEL_OBJS) $(ENGINE_OBJS) $(SERVER_OBJS) $(SCHEMA_OBJS) $(GUI_TK_OBJS)

EXES=gui-tk/minsky $(SERVER_OBJS)
#EXES=gui-tk/minsky server/server

ifeq ($(OS),Darwin)
FLAGS+=-DENABLE_DARWIN_EVENTS
endif

# TODO - remove dependency on GUI directory here
FLAGS+=-std=c++11 -Ischema -Iengine -Imodel $(OPT) -UECOLAB_LIB -DECOLAB_LIB=\"library\"

VPATH= schema model engine gui-tk server $(ECOLAB_HOME)/include

.h.xcd:
# xml_pack/unpack need to -typeName option, as well as including privates
	$(CLASSDESC) -typeName -nodef -respect_private -I $(CDINCLUDE) \
	-I $(ECOLAB_HOME)/include  -i $< xml_pack xml_unpack xsd_generate json_pack \
	json_unpack >$@

TESTS=
ifdef AEGIS
# ensure all exes get built in AEGIS mode
TESTS=tests 
# enable TCL coverage testing
FLAGS+=-DTCL_COV
endif

LIBS:=-L$(HOME)/usr/lib $(DIRS:%=-L%/lib/Tktable2.11) -lTktable2.11 $(LIBS)

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

LIBS+=	-ljson_spirit \
	-lboost_system$(BOOST_EXT) -lboost_regex$(BOOST_EXT) \
	-lboost_date_time$(BOOST_EXT) -lboost_program_options$(BOOST_EXT) \
	-lboost_filesystem$(BOOST_EXT) -lgsl -lgslcblas  

ifndef MXE
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
	if ping -c 1 www.google.com; then linkchecker gui-tk/library/help/minsky.html; fi
	-$(CHMOD) a+x *.tcl *.sh *.pl


include $(ALL_OBJS:.o=.d)

ifdef MXE
ifndef DEBUG
# This option removes the black window, but this also prevents being
# able to    type TCL commands on the command line, so only use it for
# release builds. Doesn't seemm to work on MXE, though - see ticket #456
FLAGS+=-DCONSOLE
FLAGS+=-mwindows
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

gui-tk/helpRefDb.tcl: doc/minsky/labels.pl
	rm -f $@
	perl makeRefDb.pl $< >$@

doc/minsky/labels.pl: $(wildcard doc/*.tex)
	cd doc; sh makedoc.sh

gui-tk/library/help: doc/minsky/labels.pl doc/minsky.html
	rm -rf $@/*
	mkdir -p $@/minsky
	find doc/minsky \( -name "*.html" -o -name "*.css" -o -name "*.png" \) -exec cp {} $@/minsky \;
	cp -r -f doc/minsky.html $@
	linkchecker $@/minsky.html

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

# we want to build this target always when under AEGIS, otherwise only
# when non-existing
ifdef AEGIS
.PHONY: minskyVersion.h
endif

minskyVersion.h:
	rm -f minskyVersion.h
ifdef AEGIS
	echo '#define MINSKY_VERSION "'$(version)'"' >minskyVersion.h
else
	echo '#define MINSKY_VERSION "unknown"' >minskyVersion.h
endif

mac-dist: gui-tk/minsky
# create executable in the app package directory. Make it 32 bit only
	mkdir -p minsky.app/Contents/MacOS
#	$(LINK) $(FLAGS) $(MODLINK) tclmain.o $(GUI_OBJS) $(ENGINE_OBJS) $(SCHEMA_OBJS) -L$(HOME)/usr/lib -L/opt/local/lib -L$(HOME)/usr/ecolab/lib -L/usr/X11/lib -lecolab /opt/local/lib/libpangocairo-1.0.a /opt/local/lib/libcairo.a /opt/local/lib/libpng16.a /opt/local/lib/libz.a /opt/local/lib/libpangoft2-1.0.a -lharfbuzz /opt/local/lib/libpango-1.0.a -Wl,-framework,Carbon -Wl,-framework,Foundation /opt/local/lib/librsvg-2.a /opt/local/lib/libgio-2.0.a -Wl,-framework,Foundation -Wl,-framework,Carbon -lgdk_pixbuf-2.0 /opt/local/lib/libgmodule-2.0.a /opt/local/lib/libgobject-2.0.a  /opt/local/lib/libcairo.a /opt/local/lib/libffi.a /opt/local/lib/libglib-2.0.a /opt/local/lib/libintl.a /opt/local/lib/libiconv.a -Wl,-framework,Carbon -Wl,-framework,Foundation /opt/local/lib/libpixman-1.a -lfontconfig /opt/local/lib/libxml2.a /opt/local/lib/liblzma.a -lcroco-0.6 /opt/local/lib/libexpat.a /opt/local/lib/libfreetype.a /opt/local/lib/libz.a /opt/local/lib/libbz2.a /opt/local/lib/libpng16.a /opt/local/lib/libX11-xcb.a /opt/local/lib/libxcb-render.a /opt/local/lib/libXrender.a /opt/local/lib/libXext.a /opt/local/lib/libX11.a /opt/local/lib/libxcb.a /opt/local/lib/libXau.a /opt/local/lib/libXdmcp.a -lboost_regex-mt -lboost_filesystem-mt /opt/local/lib/libboost_system-mt.a -lTkTable2.11 /opt/local/lib/libexpat.a -ljson_spirit -ltk -ltcl -L/usr/X11R6/lib -lX11 -Wl,-weak-lXss -lXext -L/usr/X11/lib -lXft -lX11 -L/usr/X11/lib -lfreetype -L/usr/X11/lib -lfontconfig -L/usr/X11/lib -lXrender -lX11  -framework CoreFoundation   -framework CoreFoundation -Wl,-framework,Carbon -Wl,-framework,IOKit -Wl,-framework,CoreServices -Wl,-framework,ApplicationServices /opt/local/lib/libXrender.a  /opt/local/lib/libX11.a -lresolv -lm /opt/local/lib/libgsl.a  /opt/local/lib/libgslcblas.a /opt/local/lib/libz.a /opt/local/lib/libreadline.a /opt/local/lib/libncurses.a -lpcre -pthread -o $(MAC_DIST_DIR)/minsky
	sh -v mkMacDist.sh

minsky.xsd: gui-tk/minsky
	gui-tk/minsky exportSchema.tcl

upload-schema: minsky.xsd
	scp minsky.xsd hpcoder@web.sourceforge.net:/home/project-web/minsky/htdocs

install: GUI/minsky$(EXE)
	mkdir -p $(PREFIX)/bin
	cp GUI/minsky$(EXE) $(PREFIX)/bin
	mkdir -p $(PREFIX)/lib/minsky
	cp -r GUI/*.tcl GUI/accountingRules GUI/icons GUI/library $(PREFIX)/lib/minsky



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
	rsync -r -z --progress --delete doxydoc hpcoder@web.sourceforge.net:/home/project-web/minsky/htdocs

# run the regression suite checking for the TCL code coverage
tcl-cov:
	rm -f minsky.cov minsky.cov.{pag,dir} coverage.o
	-env MINSKY_COV=`pwd`/minsky.cov $(MAKE) AEGIS=1 sure
	sh test/run-tcl-cov.sh

MINSKY_VERSION=$(shell git describe)

dist:
	git archive --format=tar.gz --prefix=Minsky-$(MINSKY_VERSION)/ HEAD -o /tmp/Minsky-$(MINSKY_VERSION).tar.gz
