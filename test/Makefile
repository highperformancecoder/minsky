DEBUG=1

# root directory for ecolab include files and libraries
ECOLAB_HOME=../ecolab

ifneq ($(MAKECMDGOALS),clean)
include $(ECOLAB_HOME)/include/Makefile
endif

VPATH= .. ../schema ../model ../engine ../tensor ../RESTService $(ECOLAB_HOME)/include

UNITTESTOBJS=main.o testModel.o testMinsky.o testLatexToPango.o testVariable.o testDerivative.o testUnits.o testXVector.o testLockGroup.o testCSVParser.o testTensorOps.o testStr.o

MINSKYOBJS=$(filter-out ../tclmain.o ../RESTService.o,$(wildcard ../*.o))
FLAGS:=-I.. -I../RESTService -I../tensor $(FLAGS)
FLAGS+=-std=c++11  -Wno-unused-local-typedefs -I../model -I../engine -I../schema
LIBS+=-ljson_spirit -lsoci_core -lboost_system -lboost_thread \
	-lboost_regex -lboost_date_time -lboost_filesystem \
	-lUnitTest++ -lgsl -lssl -lcrypto -lgslcblas  -lxml2 -ltiff

# RSVG dependencies calculated here
FLAGS+=$(shell pkg-config --cflags librsvg-2.0)
LIBS+=$(shell pkg-config --libs librsvg-2.0)

EXES=cmpFp checkSchemasAreSame
#testDatabase testGroup 

ifdef AEGIS
#EXES+=tcl-cov
endif

all: unittests $(EXES)

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

checkSchemasAreSame: checkSchemasAreSame.o $(MINSKYOBJS)
	$(CPLUSPLUS) $(FLAGS) -o $@ $^ $(LIBS)

tcl-cov: tcl-cov.o $(MINSKYOBJS)
	$(CPLUSPLUS) $(FLAGS) -o $@ $^ $(LIBS)

ifneq ($(MAKECMDGOALS),clean)
include $(UNITTESTOBJS:.o=.d) $(EXES:=.d)
endif

BASIC_CLEAN=rm -rf *.o *~ "\#*\#" core *.d *.cd *.xcd *.gcda *.gcno

clean:
	-$(BASIC_CLEAN) unittests $(EXES)
	cd 00; $(BASIC_CLEAN)
	cd exampleLogs; $(BASIC_CLEAN)
	cd oldSchema; $(BASIC_CLEAN)
