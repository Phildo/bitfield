CURDIR = $(shell pwd)
SRCDIR = .
OUTFILE = bf
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

$(OUTFILE): $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) $(LFLAGS) $(SOURCES) -o $(OUTFILE)

run: $(OUTFILE)
	./$(OUTFILE) -i ~/Desktop/font.bmp -o ~/Desktop/font.48x64bf_latest

$(OUTFILE).dSYM: $(HEADERS) $(SOURCES)
	$(CC) $(DEBUGFLAGS) $(CFLAGS) $(LFLAGS) $(SOURCES) -o $(OUTFILE)

debug: $(OUTFILE).dSYM
	$(DEBUGGER) $(OUTFILE)

clean:
	for f in $(OUTFILE) $(OUTFILE).dSYM ; do if [ -d $$f ] ; then rm -r $$f ; elif [ -f $$f ] ; then rm $$f ; fi ; done

