DEBUG=1

# root directory for ecolab include files and libraries
ECOLAB_HOME=../ecolab

ifneq ($(MAKECMDGOALS),clean)
include $(ECOLAB_HOME)/include/Makefile
endif
# link statically to ecolab (needed until all bugs in EcoLab 6 ironed out)
LIBS:=$(subst -lecolab,$(ECOLAB_HOME)/lib/libecolab.a,$(LIBS)) 

VPATH= .. ../schema ../model ../engine ../RESTService ../RavelCAPI/civita ../RavelCAPI $(ECOLAB_HOME)/include

GTESTOBJS=testCSVParser.o testCanvas.o testDerivative.o testExpressionWalker.o testGrid.o testLatexToPango.o testLockGroup.o testMdl.o testMinsky.o testModel.o testPannableTab.o testPhillips.o testPlotWidget.o testPubTab.o testSaver.o testSheet.o testStr.o testTensorOps.o testUnits.o testUserFunction.o testVariable.o testVariablePane.o testXVector.o testZStream.o ticket-1461.o

MINSKYOBJS=localMinsky.o ../libminsky.a
FLAGS:=-I.. -I../RESTService -I../RavelCAPI/civita -I../RavelCAPI $(FLAGS)
FLAGS+=-std=c++20  -DJSON_PACK_NO_FALL_THROUGH_TO_STREAMING -DUSE_UNROLLED -DCLASSDESC_ARITIES=0xf \
  -Wno-unknown-warning-option -Wno-unused-local-typedefs -Wno-unused-command-line-argument -I../model -I../engine -I../schema
LIBS+=-L../RavelCAPI -lravelCAPI -L../RavelCAPI/civita -lcivita -lboost_system -lboost_thread \
	-lboost_regex -lboost_date_time -lboost_filesystem -lclipboard -lxcb -lX11 \
	-lgsl -lssl -lcrypto -lgslcblas  -lxml2 -ltiff -ldl

ifdef MXE
else
LIBS+=-lxcb
endif

ifdef OPENMP
FLAGS+=-fopenmp
LIBS+=-fopenmp
endif

# RSVG dependencies calculated here
FLAGS+=$(shell pkg-config --cflags librsvg-2.0)
LIBS+=$(shell pkg-config --libs librsvg-2.0)

EXES=cmpFp checkSchemasAreSame
#testDatabase testGroup 


ifdef ASAN
FLAGS+=-fsanitize=address -fno-omit-frame-pointer
endif

all: unittests $(EXES)

unittests: gtestUnittests.o $(GTESTOBJS) $(MINSKYOBJS)
	$(CPLUSPLUS) $(FLAGS)  -o $@ $^ $(LIBS) -lgtest

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
include $(UNITTESTOBJS:.o=.d) $(GTESTOBJS:.o=.d) $(EXES:=.d)
endif

BASIC_CLEAN=rm -rf *.o *~ "\#*\#" core *.d *.cd *.xcd *.gcda *.gcno

clean:
	-$(BASIC_CLEAN) unittests gtestUnittests $(EXES)
	cd 00; $(BASIC_CLEAN)
	cd exampleLogs; $(BASIC_CLEAN)
	cd oldSchema; $(BASIC_CLEAN)
