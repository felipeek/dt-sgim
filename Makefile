IDIR=include
CC=gcc
CFLAGS=-I$(IDIR)

SRCDIR=src
OUTDIR=bin
OBJDIR=$(OUTDIR)/obj
LDIR=lib

LIBS=-lm -lglfw -lGLEW -lfreetype -lGL -lpng -lz

_DEPS = camera.h common.h core.h graphics_math.h graphics.h 
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = camera.o core.o graphics_math.o graphics.o main.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(shell mkdir -p $(@D))
	$(CC) -c -o $@ $< $(CFLAGS)

gimmesh: $(OBJ)
	$(shell mkdir -p $(@D))
	$(CC) -o $(OUTDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OUTDIR)