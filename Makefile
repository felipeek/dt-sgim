IDIR=include
CC=gcc
CFLAGS=-I$(IDIR) -g

SRCDIR=src
OUTDIR=bin
OBJDIR=$(OUTDIR)/obj
LDIR=lib

LIBS=-lm -lglfw -lGLEW -lfreetype -lGL -lpng -lz

_DEPS = camera.h common.h core.h domain_transform.h filter.h gim.h graphics_math.h graphics.h util.h
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = camera.o core.o domain_transform.o filter.o gim.o graphics_math.o graphics.o main.o util.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(shell mkdir -p $(@D))
	$(CC) -c -o $@ $< $(CFLAGS)

gimmesh: $(OBJ)
	$(shell mkdir -p $(@D))
	$(CC) -o $(OUTDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -r $(OUTDIR)
