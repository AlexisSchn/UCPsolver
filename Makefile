
#@brief   Makefile for UCP solver




#-----------------------------------------------------------------------------
# paths
#-----------------------------------------------------------------------------

SCIPDIR      	:=      /home/schlegel/scipoptsuite-7.0.2/scip
CPLEXDIR      	:=  	/opt/ibm/ILOG/CPLEX_Studio201/cplex
CONCERTDIR    	:= 		/opt/ibm/ILOG/CPLEX_Studio201/concert

#-----------------------------------------------------------------------------
# include default project Makefile from SCIP
#-----------------------------------------------------------------------------
include $(SCIPDIR)/make/make.project

# ---------------------------------------------------------------------
# Compiler options for Cplex
# ---------------------------------------------------------------------

CCOPT = -O0  -m64 -O -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD
COPT  = -O0 -m64 -fPIC -fno-strict-aliasing



# ---------------------------------------------------------------------
# CPLEX Link options and libraries
# ---------------------------------------------------------------------

SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic

CPLEXBINDIR   = $(CPLEXDIR)/bin/$(BINDIST)
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CCLNDIRS  = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR)

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include

CPLEXLNFLAGS = -lconcert -lilocplex -lcplex -lm -lpthread 

CCCPLEXFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)
CCPLEXFLAGS  = $(COPT)  -I$(CPLEXINCDIR)			



#-----------------------------------------------------------------------------
# Main Program
#-----------------------------------------------------------------------------

MAINNAME	=	ucp.exe


# Add all the objects here
MAINOBJ	=	main.o \
			\
			DataClasses/InstanceUCP.o \
			DataClasses/ProductionPlan.o \
			\
			OtherResolution/FormulationCompact.o \
			OtherResolution/FormulationLinearRelaxation.o \
			\
			Decomposition/FormulationMaster.o \
			Decomposition/FormulationPricer.o \
			Decomposition/VariableMaster.o \
			\
			UnitDecomposition2/FormulationMasterUnitDecomposition2.o \
			UnitDecomposition2/FormulationPricerUnitDecomposition2.o \
			UnitDecomposition2/ObjPricerUCPUnitDecomposition2.o \
			\
			UnitDecomposition3/FormulationMasterUnitDecomposition3.o \
			UnitDecomposition3/FormulationPricerUnitDecomposition3.o \
			UnitDecomposition3/ObjPricerUCPUnitDecomposition3.o \
			\
			TimeDecomposition/FormulationMasterTimeDecomposition.o \
			TimeDecomposition/FormulationPricerTimeDecomposition.o \
			TimeDecomposition/ObjPricerUCPTimeDecomposition.o \
			\
			TimeDecomposition2/FormulationMasterTimeDecomposition2.o \
			TimeDecomposition2/FormulationPricerTimeDecomposition2.o \
			TimeDecomposition2/ObjPricerUCPTimeDecomposition2.o \
			\
			TimeDecomposition2pmin/FormulationMasterTimeDecomposition2pmin.o \
			TimeDecomposition2pmin/FormulationPricerTimeDecomposition2pmin.o \
			TimeDecomposition2pmin/ObjPricerUCPTimeDecomposition2pmin.o \
			\
			TimeDecomposition2pmax/FormulationMasterTimeDecomposition2pmax.o \
			TimeDecomposition2pmax/FormulationPricerTimeDecomposition2pmax.o \
			TimeDecomposition2pmax/ObjPricerUCPTimeDecomposition2pmax.o \
			\
			SwitchDecomposition/FormulationMasterSwitchDecomposition.o \
			SwitchDecomposition/FormulationPricerSwitchDecomposition.o \
			SwitchDecomposition/ObjPricerUCPSwitchDecomposition.o \
			\
			SwitchDecomposition2/FormulationMasterSwitchDecomposition2.o \
			SwitchDecomposition2/FormulationPricerSwitchDecomposition2.o \
			SwitchDecomposition2/ObjPricerUCPSwitchDecomposition2.o \
			\
			OverlappingDecomposition/FormulationMasterOverlappingDecomposition.o\
			OverlappingDecomposition/FormulationPricerOverlappingDecompositionUnit.o \
			OverlappingDecomposition/FormulationPricerOverlappingDecompositionTime.o \
			OverlappingDecomposition/ObjPricerUCPOverlappingDecomposition.o

									
MAINSRC	=	$(addprefix $(SRCDIR)/,$(MAINOBJ:.o=.cpp))
MAINDEP		=	$(SRCDIR)/depend.cppmain.$(OPT)
	
MAIN		=	$(MAINNAME).$(BASE).$(LPS)$(EXEEXTENSION)
MAINFILE	=	$(BINDIR)/$(MAIN)
MAINSHORTLINK	=	$(BINDIR)/$(MAINNAME)
MAINOBJFILES	=	$(addprefix $(OBJDIR)/,$(MAINOBJ))


#-----------------------------------------------------------------------------
# include default project Makefile from SCIP (need to do this twice, once to
# find the correct binary, then, after getting the correct flags from the
# binary (which is necessary since the ZIMPL flags differ from the default
# if compiled with the SCIP Optsuite instead of SCIP), we need to set the
# compile flags, e.g., for the ZIMPL library, which is again done in make.project
#-----------------------------------------------------------------------------
include $(SCIPDIR)/make/make.project
SCIPVERSION				:=$(shell $(SCIPDIR)/bin/scip.$(BASE).$(LPS).$(TPI)$(EXEEXTENSION) -v | sed -e 's/$$/@/')
override ARCH			:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* ARCH=\([^@]*\).*/\1/')
override EXPRINT		:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* EXPRINT=\([^@]*\).*/\1/')
override GAMS			:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* GAMS=\([^@]*\).*/\1/')
override GMP			:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* GMP=\([^@]*\).*/\1/')
override SYM			:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* SYM=\([^@]*\).*/\1/')
override IPOPT			:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* IPOPT=\([^@]*\).*/\1/')
override IPOPTOPT		:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* IPOPTOPT=\([^@]*\).*/\1/')
override LPSCHECK		:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* LPSCHECK=\([^@]*\).*/\1/')
override LPSOPT 		:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* LPSOPT=\([^@]*\).*/\1/')
override NOBLKBUFMEM	:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* NOBLKBUFMEM=\([^@]*\).*/\1/')
override NOBLKMEM		:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* NOBLKMEM=\([^@]*\).*/\1/')
override NOBUFMEM		:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* NOBUFMEM=\([^@]*\).*/\1/')
override PARASCIP		:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* PARASCIP=\([^@]*\).*/\1/')
override READLINE		:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* READLINE=\([^@]*\).*/\1/')
override SANITIZE		:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* SANITIZE=\([^@]*\).*/\1/')
override ZIMPL			:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* ZIMPL=\([^@]*\).*/\1/')
override ZIMPLOPT		:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* ZIMPLOPT=\([^@]*\).*/\1/')
override ZLIB			:=$(shell echo "$(SCIPVERSION)" | sed -e 's/.* ZLIB=\([^@]*\).*/\1/')
include $(SCIPDIR)/make/make.project




#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------

ifeq ($(VERBOSE),false)
.SILENT:	$(MAINFILE) $(MAINOBJFILES) $(MAINSHORTLINK)
endif

.PHONY: all
all:            $(SCIPDIR) $(MAINFILE) $(MAINSHORTLINK)


$(MAINSHORTLINK):	$(MAINFILE)
		@rm -f $@
		cd $(dir $@) && ln -s $(notdir $(MAINFILE)) $(notdir $@)

$(OBJDIR):
		@-mkdir -p $(OBJDIR)

$(BINDIR):
		@-mkdir -p $(BINDIR)



$(MAINFILE):	$(BINDIR) $(OBJDIR) $(SCIPLIBFILE) $(LPILIBFILE) $(NLPILIBFILE) $(MAINOBJFILES)
		@echo "-> linking $@"
		$(LINKCXX) $(CCLNDIRS) $(MAINOBJFILES) $(LINKCXXSCIPALL) \
		$(CPLEXFLAGS) $(CPLEXLNFLAGS)    \
		$(LDFLAGS) $(LINKCXX_o)$@


$(OBJDIR)/%.o:	$(SRCDIR)/%.cpp
		@echo "-> compiling $@"
		$(CXX) $(FLAGS) $(OFLAGS) $(BINOFLAGS) $(CXXFLAGS) $(DFLAGS) $(CCCPLEXFLAGS) -g -c $< $(CXX_o)$@



#---- EOF --------------------------------------------------------------------











