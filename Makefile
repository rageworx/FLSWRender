# Makefile for POSIX, for FLTK software rednerer test
# -----------------------------------------------------------------------------
# (C)2021 Raphael Kim

ARCH_S = $(shell uname -s)
ARCH_M = $(shell uname -m)
ARCH_R = $(shell uname -r | cut -d . -f1)

GCC = gcc
GXX = g++
AR  = ar
RL  = ranlib

DIR_SRC = src/
DIR_OBJ = obj/
DIR_LIB = lib/
DIR_BIN = bin/

NAME_B = flswrender
TARGET = $(DIR_LIB)lib$(NAME_B).a
TESTBIN = $(DIR_BIN)test

FLTK_ICFG = $(shell fltk-config --use-images --cxxflags)
FLTK_LCFG = $(shell fltk-config --use-images --ldflags)

SRCS = $(wildcard $(DIR_SRC)*.cpp)
OBJS = $(SRCS:$(DIR_SRC)%.cpp=$(DIR_OBJ)%.o)

DEFS += -DCORRECTING_SHADOW
OPTS  =
LOPTS =

# Architecture reconfiguration ...
ifeq ($(ARCH_S),Darwin)
	GCC = llvm-gcc
	CXX = llvm-g++

	ifeq ($(shell test $(ARCH_R) -gt 19; echo $$?),0)
		OPTS += -arch x86_64 -arch arm64
	endif
	OPTS  += -std=c++11
else ifeq ($(ARCH_S),Linux)
	OPTS  += -std=c++11
	LOPTS += -s
else
	ARCH_SS = $(shell echo $(ARCH_S) | cut -d _ -f1)
	ifeq ($(ARCH_SS),MINGW64)
		OPTS  += -mwindows 
		LOPTS += -s -static
	endif
endif

CFLAGS += $(DEFS)
CFLAGS += $(OPTS)
CLFAGS += -ffast-math
CFLAGS += -I$(DIR_SRC)
CFLAGS += -I../fl_imgtk/lib
CFLAGS += $(FLTK_ICFG)

LFLAGS += $(LOPTS)
LFLAGS += -L../fl_imgtk/lib
LFLAGS += -lfl_imgtk
LFLAGS += $(FLTK_LCFG)
LFLAGS += -Os -fomit-frame-pointer
#LFLAGS += -g

.PHONY: clean prepare test cleantest

all: prepare $(TARGET)

prepare:
	@mkdir -p $(DIR_OBJ)
	@mkdir -p $(DIR_LIB)
	@mkdir -p $(DIR_BIN)

clean:
	@rm -rf $(OBJS)
	@rm -rf $(TARGET)

cleantest:
	@rm -rf $(TESTBIN)

$(OBJS): $(DIR_OBJ)%.o: $(DIR_SRC)%.cpp
	@echo "Building $@ ... "
	@$(GXX) $(CFLAGS) -c $< -o $@
    
$(TARGET): $(OBJS)
	@echo "Generating $@ ..."
	@$(AR) -cr $@ $^
	@$(RL) $@
	@cp -f src/FLSWRenderer.H lib
	@cp -f src/FLSWRenderMath.H lib
	@echo "done"
				 
test: $(TARGET)
	@echo "Generating test ..."
	@$(GXX) test/main.cpp $(CFLAGS) $(LFLAGS) -Llib -l$(NAME_B) -o $(TESTBIN)
