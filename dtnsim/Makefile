#Name of target to be created(-o option)
TARGET_DIR = .
TARGET_NAME = dtnsim$(D)
TARGET = $(TARGET_NAME)$(EXE_SUFFIX)
TARGET_IMPLIB = $(TARGET_NAME)$(IMPLIB_SUFFIX)
TARGET_IMPDEF = $(TARGET_NAME)$(IMPDEF_SUFFIX)
TARGET_FILES = $(TARGET_DIR)/$(TARGET)

#User interface(uncomment one)(-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(QTENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(QTENV_LIBS)

PYTHON_VERSION=3.11
#C++ include paths(with - I)
INCLUDE_PATH = -I. -I/usr/include/python$(PYTHON_VERSION)

#Additional object and library files to link with
EXTRA_OBJS =

#Additional libraries(-L, -l options)
LIBS =  -lpython$(PYTHON_VERSION)

#Output directory
PROJECT_OUTPUT_DIR = out
PROJECTRELATIVE_PATH =
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

#Object files for local.cc, .msg and.sm files
OBJS = \
    $O/src/central/Central.o \
    $O/src/node/app/App.o \
    $O/src/node/com/Com.o \
    $O/src/node/dtn/Contact.o \
    $O/src/node/dtn/ContactHistory.o \
    $O/src/node/dtn/ContactPlan.o \
    $O/src/node/dtn/CustodyModel.o \
    $O/src/node/dtn/Dtn.o \
    $O/src/node/dtn/SdrModel.o \
    $O/src/node/dtn/SdrStatus.o \
    $O/src/node/dtn/routing/RoutingAntop.o \
    $O/src/node/dtn/routing/RoutingBRUF1T.o \
    $O/src/node/dtn/routing/RoutingCgrModel350.o \
    $O/src/node/dtn/routing/RoutingCgrModel350_2Copies.o \
    $O/src/node/dtn/routing/RoutingCgrModel350_Hops.o \
    $O/src/node/dtn/routing/RoutingCgrModel350_Probabilistic.o \
    $O/src/node/dtn/routing/RoutingCgrModelRev17.o \
    $O/src/node/dtn/routing/RoutingCgrModelYen.o \
    $O/src/node/dtn/routing/RoutingDeterministic.o \
    $O/src/node/dtn/routing/RoutingDirect.o \
    $O/src/node/dtn/routing/RoutingEpidemic.o \
    $O/src/node/dtn/routing/RoutingOpportunistic.o \
    $O/src/node/dtn/routing/RoutingORUCOP.o \
    $O/src/node/dtn/routing/RoutingPRoPHET.o \
    $O/src/node/dtn/routing/RoutingSprayAndWait.o \
    $O/src/node/dtn/routing/RoutingStochastic.o \
    $O/src/node/dtn/routing/RoutingUncertainUniboCgr.o \
    $O/src/node/dtn/routing/RoutingUniboCgr.o \
    $O/src/node/dtn/routing/brufncopies/BRUFNCopies1TOracle.o \
    $O/src/node/dtn/routing/brufncopies/BRUFNCopiesOracle.o \
    $O/src/node/dtn/routing/brufncopies/RoutingBRUFNCopies.o \
    $O/src/node/dtn/routing/cgrbrufpowered/CGRBRUFPowered.o \
    $O/src/node/dtn/routing/unibocgr/core/bundles/bundles.o \
    $O/src/node/dtn/routing/unibocgr/core/cgr/cgr.o \
    $O/src/node/dtn/routing/unibocgr/core/cgr/phase_one.o \
    $O/src/node/dtn/routing/unibocgr/core/cgr/phase_three.o \
    $O/src/node/dtn/routing/unibocgr/core/cgr/phase_two.o \
    $O/src/node/dtn/routing/unibocgr/core/contact_plan/contactPlan.o \
    $O/src/node/dtn/routing/unibocgr/core/contact_plan/contacts/contacts.o \
    $O/src/node/dtn/routing/unibocgr/core/contact_plan/nodes/nodes.o \
    $O/src/node/dtn/routing/unibocgr/core/contact_plan/ranges/ranges.o \
    $O/src/node/dtn/routing/unibocgr/core/library/commonFunctions.o \
    $O/src/node/dtn/routing/unibocgr/core/library/list/list.o \
    $O/src/node/dtn/routing/unibocgr/core/library/log/log.o \
    $O/src/node/dtn/routing/unibocgr/core/library_from_ion/rbt/rbt.o \
    $O/src/node/dtn/routing/unibocgr/core/library_from_ion/scalar/scalar.o \
    $O/src/node/dtn/routing/unibocgr/core/msr/msr.o \
    $O/src/node/dtn/routing/unibocgr/core/msr/msr_utils.o \
    $O/src/node/dtn/routing/unibocgr/core/routes/routes.o \
    $O/src/node/dtn/routing/unibocgr/core/time_analysis/time.o \
    $O/src/node/fault/Fault.o \
    $O/src/node/graphics/Graphics.o \
    $O/src/utils/ContactPlanUtils.o \
    $O/src/utils/Lp.o \
    $O/src/utils/LpUtils.o \
    $O/src/utils/MetricCollector.o \
    $O/src/utils/RouterUtils.o \
    $O/src/utils/Subject.o \
    $O/src/utils/TopologyUtils.o \
    $O/src/dtnsim_m.o

#Message files
MSGFILES = \
    src/dtnsim.msg

#SM files
SMFILES =

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

#Pull in OMNeT++ configuration(Makefile.inc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
CONFIGFILE = $(shell opp_configfilepath)
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

#Simulation kernel and user interface libraries
OMNETPP_LIBS = $(OPPMAIN_LIB) $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS) $(IMPORT_DEFINES)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

#we want to recompile everything if COPTS changes,
#so we store COPTS into $COPTS_FILE(if COPTS has changed since last build)
#and make the object files depend on it
COPTS_FILE = $O/.last-copts
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
  $(shell $(MKPATH) "$O")
  $(file >$(COPTS_FILE),$(COPTS))
endif

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
#User - supplied makefile fragment(s)
-include makefrag

#-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

#Main target
all: $(TARGET_FILES)

$(TARGET_DIR)/% :: $O/%
	@mkdir -p $(TARGET_DIR)
	$(Q)$(LN) $< $@
ifeq ($(TOOLCHAIN_NAME),clang-msabi)
	-$(Q)-$(LN) $(<:%.dll=%.lib) $(@:%.dll=%.lib) 2>/dev/null

$O/$(TARGET_NAME).pdb: $O/$(TARGET)
endif

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile $(CONFIGFILE)
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET) $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders smheaders

#disabling all implicit rules
.SUFFIXES :
.PRECIOUS : %_m.h %_m.cc

$O/%.o: %.cc $(COPTS_FILE) | msgheaders smheaders
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc -MD -MP -MF $O/$(basename $<)_m.h.d $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning $(TARGET)
	$(Q)-rm -rf $O
	$(Q)-rm -f $(TARGET_FILES)
	$(Q)-rm -f $(call opp_rwildcard, . , *_m.cc *_m.h *_sm.cc *_sm.h)

cleanall:
	$(Q)$(CLEANALL_COMMAND)
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

help:
	@echo "$$HELP_SYNOPSYS"
	@echo "$$HELP_TARGETS"
	@echo "$$HELP_VARIABLES"
	@echo "$$HELP_EXAMPLES"

#include all dependencies
-include $(OBJS:%=%.d) $(MSGFILES:%.msg=$O/%_m.h.d)

external/lib/build/libantop.a:
	cd external/lib && mkdir -p build && cd build && cmake .. && make

$(TARGET_FILES): external/lib/build/libantop.a