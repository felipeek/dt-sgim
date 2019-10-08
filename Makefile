IDIR=include
CC=gcc
CCXX=g++
CFLAGS=-I$(IDIR) -g

SRCDIR=src
OUTDIR=bin
OBJDIR=$(OUTDIR)/obj
VENDORDIR=$(SRCDIR)/vendor
LDIR=lib

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LIBS=-framework OpenGL -lm -lglfw -lglew
else
	LIBS=-lm -lglfw -lGLEW -lGL -lpng -lz
endif

_DEPS = camera.h common.h core.h domain_transform.h filter.h gim.h graphics_math.h graphics.h menu.h obj.h parametrization.h util.h
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = camera.o core.o domain_transform.o filter.o gim.o graphics_math.o graphics.o main.o menu.o obj.o parametrization.o util.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

_VENDOR = imgui.o imgui_demo.o imgui_draw.o imgui_impl_glfw.o imgui_impl_opengl3.o imgui_widgets.o
VENDOR = $(patsubst %,$(OBJDIR)/%,$(_VENDOR))

all: gimmesh

$(OBJDIR)/%.o: $(VENDORDIR)/%.cpp $(DEPS)
	$(shell mkdir -p $(@D))
	$(CCXX) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(shell mkdir -p $(@D))
	$(CCXX) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(shell mkdir -p $(@D))
	$(CC) -c -o $@ $< $(CFLAGS)

gimmesh: $(OBJ) $(VENDOR)
	$(shell mkdir -p $(@D))
	$(CCXX) -o $(OUTDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -r $(OUTDIR)
