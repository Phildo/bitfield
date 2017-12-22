CURDIR = $(shell pwd)
SRCDIR = .
OUTFILE = piximg
CC = gcc
DEBUGGER = gdb -q
CFLAGS = -Wall -std=c99
LFLAGS = -Wall -lm -std=c99
DEBUGFLAGS = -ggdb3 -O0
PRODFLAGS = -O3
ALL_HEADERS:=$(wildcard $(CURDIR)/$(SRCDIR)/*.h)
ALL_SOURCES:=$(wildcard $(CURDIR)/$(SRCDIR)/*.c)
HEADERS:=$(ALL_HEADERS)
SOURCES:=$(filter-out %/test.c, $(ALL_SOURCES))

ARGS=-i ~/Desktop/img.bmp -o ~/Desktop/img

$(OUTFILE): $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTFILE) $(LFLAGS)

run: $(OUTFILE)
	./$(OUTFILE) $(ARGS)

$(OUTFILE).dSYM: $(HEADERS) $(SOURCES)
	$(CC) $(DEBUGFLAGS) $(CFLAGS) $(SOURCES) -o $(OUTFILE) $(LFLAGS)

debug: $(OUTFILE).dSYM
	$(DEBUGGER) --args $(OUTFILE) $(ARGS)

clean:
	for f in $(OUTFILE) $(OUTFILE).dSYM ; do if [ -d $$f ] ; then rm -r $$f ; elif [ -f $$f ] ; then rm $$f ; fi ; done

