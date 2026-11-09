# Builds Minsky as a WebAssembly module (jsminsky.js + jsminsky.wasm)
# using Emscripten. Adapted from the Ravel wasm Makefile.
#
# Prerequisites:
#   . /emsdk/emsdk_env.sh
#   libraries (boost, gsl) installed into $(EMLIBS) (default /usr/emscripten)
#   a native classdesc (ecolab/classdesc/classdesc), which is built
#   here if missing
#
# usage: make -C wasm [-j8] [DEBUG=1]
#        make -C wasm test        run the node smoke test
#        make -C wasm clean

TOP:=$(abspath ..)
BUILD:=$(abspath build)
EMLIBS?=/usr/emscripten
ECOLAB_HOME=$(TOP)/ecolab
CLASSDESC=$(ECOLAB_HOME)/classdesc/classdesc

CXX=em++

# Flags matching those of the native build, as far as relevant
CFLAGS=-DCLASSDESC -DUSE_UNROLLED -DCLASSDESC_ARITIES=0xf \
	-DJSON_PACK_NO_FALL_THROUGH_TO_STREAMING -DCIVITA_ALLOCATOR=civita::LibCAllocator \
	-DCAIRO -DZLIB --std=c++20 -fwasm-exceptions
# stubs must come first, as they shadow headers that need cairo/pango/python
CFLAGS+=-I$(BUILD) -I$(TOP)/wasm/stubs -I$(TOP)/schema -I$(TOP)/engine -I$(TOP)/model \
	-I$(TOP)/mansoura/src -I$(TOP)/RESTService -I$(TOP)/RavelCAPI/civita -I$(TOP)/RavelCAPI \
	-I$(ECOLAB_HOME)/include -I$(ECOLAB_HOME)/graphcode -I$(ECOLAB_HOME)/classdesc \
	-I$(ECOLAB_HOME)/classdesc/json5_parser/json5_parser -I$(TOP) -I$(EMLIBS)/include
CFLAGS+=-sUSE_ZLIB=1
# clobber unnecessary warnings
CFLAGS+=-Wno-deprecated-declarations -Wno-unknown-warning-option -Wno-macro-redefined \
	-Wno-attributes -Wno-pragmas -Wno-enum-constexpr-conversion -Wno-deprecated-builtins

ifdef DEBUG
OPT=-g -sASSERTIONS=1
else
OPT=-O2 -DNDEBUG
endif

LDFLAGS=-sALLOW_MEMORY_GROWTH=1 -sMODULARIZE=1 -sEXPORT_NAME=createMinsky \
	-sEXPORTED_RUNTIME_METHODS=FS -sFORCE_FILESYSTEM=1 -sSTACK_SIZE=1048576
LIBS=-L$(EMLIBS)/lib -L$(EMLIBS)/lib64 -lgsl -lgslcblas

# the object lists mirror MODEL_OBJS etc in the top level Makefile,
# less what needs a native window system (windowInformation, addon...)
MODEL_OBJS=autoLayout cairoItems canvas CSVDialog dataOp equationDisplay godleyIcon godleyTable \
	godleyTableEditor group item intOp lasso lock minsky operation operationRS operationRS1 \
	operationRS2 phillipsDiagram plotWidget port pubTab ravelWrap renderNativeWindow selection \
	sheet switchIcon userFunction userFunction_units variableInstanceList variable wire
ENGINE_OBJS=clipboard databaseIngestor derivative equationDisplayRender equations evalGodley \
	evalOp flowCoef godleyExport latexMarkup valueId variableValue node_latex node_matlab \
	CSVParser minskyTensorOps mdlReader saver rungeKutta
SCHEMA_OBJS=schema3 schema2 schema1 schema0 schemaHelper variableType operationType a85
RESTSERVICE_OBJS=minskyRS RESTMinsky
RAVEL_OBJS=ravelState CSVTools dynamicRavelCAPI hypercube index interpolateHypercube tensorOp xvector
MANSOURA_OBJS=plot SVGItem mansouraHTMLCanvas mansouraRecording mansouraNoCairo textMarkup
WASM_OBJS=cairoStub windowInformationStub
# objects that need an entry point of their own
MAIN_OBJS=jsminsky

LIBOBJS=$(addprefix $(BUILD)/,$(addsuffix .o,$(MODEL_OBJS) $(ENGINE_OBJS) $(SCHEMA_OBJS) \
	$(RESTSERVICE_OBJS) $(RAVEL_OBJS) $(MANSOURA_OBJS) $(WASM_OBJS)))
MAINOBJS=$(addprefix $(BUILD)/,$(addsuffix .o,$(MAIN_OBJS)))

# As in the native build, the Ravel C API sources are plain C++, without classdesc descriptors
RAVEL_PLAIN=$(addprefix $(BUILD)/,ravelState CSVTools dynamicRavelCAPI)
$(addsuffix .o,$(RAVEL_PLAIN)) $(addsuffix .d,$(RAVEL_PLAIN)): CFLAGS:=$(filter-out -DCLASSDESC,$(CFLAGS))

TARGET=jsminsky.js

VPATH=$(TOP)/model $(TOP)/engine $(TOP)/schema $(TOP)/RESTService $(TOP)/RavelCAPI \
	$(TOP)/RavelCAPI/civita $(TOP)/mansoura/src $(TOP)/wasm $(TOP)/wasm/stubs \
	$(ECOLAB_HOME)/include $(ECOLAB_HOME)/classdesc $(TOP)

all: $(TARGET)

# just the objects, ie everything except the Javascript binding
objs: $(LIBOBJS)

$(TARGET): $(MAINOBJS) $(LIBOBJS)
	$(CXX) --bind $(CFLAGS) $(LDFLAGS) $(OPT) $^ $(LIBS) -o $@

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: %.cc | $(BUILD)
	$(CXX) $(CFLAGS) $(OPT) -c $< -o $@

# dependencies. Generated descriptor files that don't exist yet are
# listed (-MG) without a path, so prefix them with the build directory.
$(BUILD)/%.d: %.cc | $(BUILD)
	$(CXX) $(CFLAGS) -MM -MG -MT $(BUILD)/$*.o -MT $@ $< | \
	  sed -E 's# ([A-Za-z0-9_-]+\.[jrxt]?cd)# $(BUILD)/\1#g' >$@

# Classdesc descriptors: same rules as the native build, except that
# they are generated in $(BUILD)
CDINC=-I $(ECOLAB_HOME)/include -I $(TOP)/RESTService

$(BUILD)/%.cd: %.h $(CLASSDESC) | $(BUILD)
	$(CLASSDESC) -typeName -nodef $(CDINC) -i $< pack unpack >$@

$(BUILD)/%.xcd: %.h $(CLASSDESC) | $(BUILD)
	$(CLASSDESC) -typeName -nodef -respect_private $(CDINC) -i $< \
	  xml_pack xml_unpack xsd_generate json_pack json_unpack >$@

$(BUILD)/%.rcd: %.h $(CLASSDESC) | $(BUILD)
	$(CLASSDESC) -typeName -nodef -use_mbr_pointers -onbase -overload -respect_private \
	  $(CDINC) -i $< RESTProcess >$@

$(BUILD)/%.jcd: %.h $(CLASSDESC) | $(BUILD)
	$(CLASSDESC) -typeName -nodef -respect_private -i $< json_pack json_unpack >$@

# classdesc itself must run on the host, so is built natively
$(CLASSDESC):
	$(MAKE) -C $(ECOLAB_HOME)/classdesc

# not found by the dependency scan, as they are hidden by #ifdef in minsky_epilogue.h
$(addprefix $(BUILD)/,$(addsuffix .o,$(MODEL_OBJS))): $(BUILD)/plot.xcd $(BUILD)/signature.xcd

.SECONDARY:

ifneq ($(MAKECMDGOALS),clean)
include $(LIBOBJS:.o=.d) $(MAINOBJS:.o=.d)
endif

test: $(TARGET)
	node test/smoke.js

clean:
	rm -rf $(BUILD) $(TARGET) jsminsky.wasm
