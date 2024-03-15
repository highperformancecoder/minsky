DEBUG=1

# root directory for ecolab include files and libraries
ECOLAB_HOME=../ecolab

ifneq ($(MAKECMDGOALS),clean)
include $(ECOLAB_HOME)/include/Makefile
endif

VPATH= .. ../schema ../model ../engine ../RESTService ../RavelCAPI/civita ../RavelCAPI $(ECOLAB_HOME)/include

UNITTESTOBJS=main.o testCSVParser.o testCanvas.o testDerivative.o testExpressionWalker.o testGrid.o testLatexToPango.o testLockGroup.o testMdl.o testMinsky.o testModel.o testPannableTab.o testPhillips.o testPlotWidget.o testPubTab.o testSaver.o testStr.o testTensorOps.o testUnits.o testUserFunction.o testVariable.o testVariablePane.o testXVector.o ticket-1461.o

MINSKYOBJS=$(filter-out ../tclmain.o ../minskyTCL.o ../RESTService.o ../httpd.o ../addon.o ../dummy-addon.o ../itemTemplateInstantiations.o ../typescriptAPI.o,$(wildcard ../*.o)) localMinsky.o
FLAGS:=-I.. -I../RESTService -I../RavelCAPI/civita -I../RavelCAPI $(FLAGS)
FLAGS+=-std=c++20  -DJSON_PACK_NO_FALL_THROUGH_TO_STREAMING -DUSE_UNROLLED -DCLASSDESC_ARITIES=0xf \
  -Wno-unknown-warning-option -Wno-unused-local-typedefs -Wno-unused-command-line-argument -I../model -I../engine -I../schema
LIBS+=-L../RavelCAPI -lravelCAPI -L../RavelCAPI/civita -lcivita -lboost_system -lboost_thread \
	-lboost_regex -lboost_date_time -lboost_filesystem -lclipboard -lxcb -lX11 \
	-lUnitTest++ -lgsl -lssl -lcrypto -lgslcblas  -lxml2 -ltiff -ldl

ifdef MXE
else
LIBS+=-lxcb
endif


# RSVG dependencies calculated here
FLAGS+=$(shell pkg-config --cflags librsvg-2.0)
LIBS+=$(shell pkg-config --libs librsvg-2.0)

EXES=cmpFp checkSchemasAreSame
#testDatabase testGroup 

ifdef AEGIS
#EXES+=tcl-cov
endif

ifdef ASAN
FLAGS+=-fsanitize=address -fno-omit-frame-pointer
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
