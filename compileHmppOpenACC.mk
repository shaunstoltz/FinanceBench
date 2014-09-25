#
# Copyright 2008 - 2010 CAPS entreprise. All rights reserved.
#
ATI_DISPLAY    = DISPLAY=:0.0

GCC            = gcc
ICC            = icc
HMPP           = hmpp #--nvcc-options -arch,sm_10

# if running on GPU of compute capacity 1.0, put the following at the end of the previous line
# --nvcc-options -arch,sm_10

CFLAGS         = -O2 -p -w
CPPFLAGS       = #-I../../../../common
LDFLAGS        =
HMPPFLAGS      = --codelet-required

VPATH          = ../../../../common

CLEANFILES     = *.exe *.so *.so.* *.cu *.cu.* *.linkinfo

BIN	       = $(EXE_NAME)

## choose your compiler here ##
CC	       = $(GCC)

#ifeq($(CC),$(GCC))
FLAGS  = $(GCC_FLAGS)
#else
#FLAGS  = $(ICC_FLAGS)
#endif

.DEFAULT: all

all:
	$(HMPP) $(HMPPFLAGS) $(DFLAGS) $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $(EXE_NAME).exe $(MAIN_FILE)

#exe: $(BIN).exe

#run: exe
#	$(ATI_DISPLAY) ./$(BIN).exe

#verbose:
#	$(MAKE) $(BIN).exe DFLAGS="-k -d"
#	$(MAKE) run HMPP_VERBOSITY=9
#	$(GPROF) ./$(BIN).exe

#%.exe: %.c $(DEPS)
#	$(HMPP) $(HMPPFLAGS) $(DFLAGS) $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $(EXE_NAME).exe $(MAIN_FILE)

#%.o: %.c
#	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	rm -f $(CLEANFILES)

help: 	
	@echo "#"
	@echo "# Copyright 2008 - 2010 CAPS entreprise. All rights reserved."
	@echo "#"
	@echo ""
	@echo "Targets available are :"
	@echo ""
	@echo " make all     : compiles and runs the application"
	@echo ""
	@echo " make exe     : compiles the application"
	@echo ""
	@echo " make run     : runs the execution"
	@echo ""
	@echo " make verbose : compiles and launches the execution with the higher level of verbosity"
	@echo ""
	@echo " make clean   : deletes files generated by make"
	@echo ""
