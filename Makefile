# Makefile for MinGW-W64 for Dazed/SoftwareRenderer
# -----------------------------------------------------------------------------
# (C)2021 Raphael Kim

GCC = gcc
GXX = g++

DIR_SRC = src/
DIR_OBJ = obj/
DIR_BIN = bin/

TARGET = $(DIR_BIN)swrndrtest

FLTK_ICFG = $(shell fltk-config --use-images --cxxflags)
FLTK_LCFG = $(shell fltk-config --use-images --ldflags)

#LIBWIN32S  = -lole32 -luuid -lcomctl32 -lwsock32 -lm -lgdi32 -luser32 
#LIBWIN32S += -lkernel32 -lShlwapi -lcomdlg32 -lshcore -loleacc

SRCS = $(wildcard $(DIR_SRC)*.cpp)
OBJS = $(SRCS:$(DIR_SRC)%.cpp=$(DIR_OBJ)%.o)

DEFS += -DCORRECTING_SHADOW

CFLAGS += $(DEFS)
CFLAGS += -mwindows
CLFAGS += -ffast-math
CFLAGS += -fopenmp
CFLAGS += -I$(DIR_SRC)
CFLAGS += -I../fl_imgtk/lib
CFLAGS += $(FLTK_ICFG)

LFLAGS += -static
LFLAGS += -L../fl_imgtk/lib
LFLAGS += -lfl_imgtk_omp
LFLAGS += $(FLTK_LCFG)
LFLAGS += $(LIBWIN32S)
LFLAGS += -O3 -s -fomit-frame-pointer
#LFLAGS += -g

.PHONY: clean prepare

all: prepare $(TARGET)

prepare:
	@mkdir -p $(DIR_OBJ)
	@mkdir -p $(DIR_BIN)

clean:
	@rm -rf $(OBJS)
	@rm -rf $(TARGET)

$(OBJS): $(DIR_OBJ)%.o: $(DIR_SRC)%.cpp
	@echo "Building $@ ... "
	@$(GXX) $(CFLAGS) -c $< -o $@
    
 $(TARGET): $(OBJS)
	@echo "Generating $@ ..."
	@$(GXX) $^ $(CFLAGS) $(LFLAGS) -o $@
	@echo "done."
