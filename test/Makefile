DEBUG=1

# root directory for ecolab include files and libraries
ifeq ($(shell ls $(HOME)/usr/ecolab/include/ecolab.h),$(HOME)/usr/ecolab/include/ecolab.h)
ECOLAB_HOME=$(HOME)/usr/ecolab
else
ECOLAB_HOME=/usr/local/ecolab
endif

include $(ECOLAB_HOME)/include/Makefile

UNITTESTOBJS=main.o testMinsky.o testGroup.o testGeometry.o testDatabase.o testLatexToPango.o testVariable.o testDerivative.o
MINSKYOBJS=$(filter-out ../tclmain.o ../server-main.o,$(wildcard ../*.o))
FLAGS:=-I.. $(FLAGS)
FLAGS+=-std=c++11 -I../GUI -I../engine -I../schema
LIBS+=-ljson_spirit -lsoci_core -lwebsocketpp -lboost_system -lboost_thread \
	-lboost_regex -lboost_date_time -lboost_filesystem -lboost_signals \
	-lUnitTest++ -lgsl -lgslcblas  \
	-ljpeg -lxml2 -ltiff

# RSVG dependencies calculated here
FLAGS+=$(shell pkg-config --cflags librsvg-2.0)
LIBS+=$(shell pkg-config --libs librsvg-2.0)

EXES=unittests cmpFp testDatabase testLatexToPango testGroup 

ifdef AEGIS
EXES+=tcl-cov
endif

all: $(EXES)

unittests: $(UNITTESTOBJS) $(MINSKYOBJS)
	$(CPLUSPLUS) $(FLAGS)  -o $@ $(UNITTESTOBJS) $(MINSKYOBJS) $(LIBS)

# todo - remove dependency on all MINSKYOBJS
testDatabase: main.o testDatabase.o $(MINSKYOBJS)
	$(CPLUSPLUS) $(FLAGS)  -o $@ $^ $(LIBS)

testLatexToPango: main.o testLatexToPango.o $(MINSKYOBJS)
	$(CPLUSPLUS) $(FLAGS)  -o $@ $^ $(LIBS)

testGroup: main.o testGroup.o $(MINSKYOBJS)
	$(CPLUSPLUS) $(FLAGS)  -o $@ $^ $(LIBS)

cmpFp: cmpFp.o
	$(CPLUSPLUS) $(FLAGS) -o $@ $<

tcl-cov: tcl-cov.o $(MINSKYOBJS)
	$(CPLUSPLUS) $(FLAGS) -o $@ $^ $(LIBS)

include $(UNITTESTOBJS:.o=.d)

clean:
	$(BASIC_CLEAN) $(EXES)
	cd 00; $(BASIC_CLEAN)
	cd exampleLogs; $(BASIC_CLEAN)
	cd oldSchema; $(BASIC_CLEAN)
