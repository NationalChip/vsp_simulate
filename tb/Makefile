#
# Voice Signal Preprocess
# Copyright (C) 1991-2017 Nationalchip Co., Ltd
#
# Makefile: Dsp tb Makefile
#
#=================================================================================#
# Toolchain Configurations

AR              = xt-ar
CC              = xt-xcc
CPP             = xt-xc++
OBJDUMP         = xt-objdump --disassemble
READELF         = xt-readelf -a
DUMPELF         = xt-dumpelf
STRIP           = xt-strip
GENLDSCRIPTS    = xt-genldscripts
REGENLSPS       = xt-regenlsps

RM              = rm -f
SHELL           = /bin/bash


#=================================================================================#
# Flags Configurations

XTENSA_CORE 	= Hifi4_bd7_20161020_M
XTENSA          = $(HOME)/xtensa

XTENSA_SYSTEM   = $(XTENSA)/XtDevTools/install/builds/RF-2016.4-linux/$(XTENSA_CORE)/config

LSP_FLAGS       = -mlsp=./output/lsp-sim

CORE_FLAGS      = --xtensa-system=$(XTENSA_SYSTEM) --xtensa-core=$(XTENSA_CORE)
LDFLAGS         = $(CORE_FLAGS) $(LSP_FLAGS) -lstdc++ -Wl,-gc-sections -mlongcalls
CFLAGS          = $(CORE_FLAGS) -std=gnu99 -Wall -ffunction-sections -mlongcalls -g -O2
CPPFLAGS        = $(CORE_FLAGS) -std=gnu++98 -Wall -ffunction-sections -mlongcalls -g -O2

INCLUDE_DIR     = ./include
LIB_DIR         = ./lib

LIBS            = -lhifi4-vfpu -lm

INCLUDE_FLAGS   = $(addprefix -I , $(INCLUDE_DIR))
CFLAGS         += $(INCLUDE_FLAGS)
CPPFLAGS       += $(INCLUDE_FLAGS)

LIB_FLAGS       = $(addprefix -L , $(LIB_DIR))
LDFLAGS        += $(LIBS) $(LIB_FLAGS)


#---------------------------------------------------------------------------------#
# Source List
-include src/Makefile
-include common/Makefile
-include algorithm/Makefile

target          = ./output/dsp_tb.elf
all_objs        = $(src_objs) $(algorithm_objs) $(common_objs)
all_deps        = $(subst .o,.d,$(all_objs))

#=================================================================================#
# Build Rules

.PHONY: clean export help


all: build
build: prepare $(target)
prepare:
	@echo "[building lsp-sim]"
	@$(REGENLSPS) -f -srcbase ./arch/xtensa/ -dstbase  ./output/
	@$(GENLDSCRIPTS) $(CORE_FLAGS) -b ./output/lsp-sim

$(target): $(all_objs)
	@echo "[$(CC) Linking $@]"
	@$(CC) -o $@ $(all_objs) $(LDFLAGS)
	@xt-ar -r ./output/libalgorithm.a $(algorithm_objs)

profile:
	@xt-run  ${CORE_FLAGS} --client_cmds="profile misses.out" $(target) ./audio_data
	@xt-gprof $(target) misses.out > dsp_tb.profile
	#@xt-run  ${CORE_FLAGS} --client_cmds="profile --instructions misses.out" $(target) ./audio_data
	#@xt-gprof -l -A -f $(target)  > dsp_tb.profile

#---------------------------------------------------------------------------------#
# Include dependance

ifeq ($(MAKECMDGOALS), clean)
    NOT_INCLUDE_DEP = true
endif
ifeq ($(MAKECMDGOALS), help)
    NOT_INCLUDE_DEP = true
endif
ifneq ($(NOT_INCLUDE_DEP), true)
    -include $(all_deps)
endif

#---------------------------------------------------------------------------------#
# Common Build Rules

%.d:%.cc
	@echo [$(CPP) creating $@]
	@$(CPP) $(CPPFLAGS) -M $< > $@.$$$$;\
	sed 's,\(.*\)\.o[ :]*,$(shell dirname $@)/\1.o $@ : ,g' <$@.$$$$ > $@;\
	$(RM) $@.$$$$

%.o:%.cc
	@echo [$(CPP) compiling $@]
	@$(CPP) $(CPPFLAGS) -c $< -o $@

%.d:%.c
	@echo [$(CC) creating $@]
	@$(CC) $(CFLAGS) -M $< > $@.$$$$;\
	sed 's,\(.*\)\.o[ :]*,$(shell dirname $@)/\1.o $@ : ,g' <$@.$$$$ > $@;\
	$(RM) $@.$$$$

%.o:%.c
	@echo [$(CC) compiling $@]
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(RM) $(all_objs) $(all_deps)
	@$(RM) -r $(target)
	@$(RM) -r ./output/lsp-sim
	@$(RM) -r dsp_tb.profile
	@echo "[clean done]"

help:
	@echo 'Generic Makefile for C/C++ Programs (GNU/linux makefile) version 0.1'  
	@echo 'Copyright (C) 2017'  
	@echo   
	@echo 'Usage: make [target]'  
	@echo 'TARGETS:'  
	@echo '  all       compile and create elf files(.elf)'
	@echo '  clean     clean all objects and the executable file.'  
	@echo '  profile   build profile.'  
	@echo '  help      print this message.'  
	@echo
