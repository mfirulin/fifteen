CC = g++
SRCS = main.cpp
OBJS = main.o
BINS = fifteen.out
INCLUDES = -I/usr/local/include
LIBS = -L/usr/local/lib
COMPILER_OPTS = -Og
LINKER_OPTS = -lSDL2 -lSDL2_image
INSTALL = install
PREFIX = /usr/local
INSTALL_OPTS = -s -m 755 -o 0 -g 0

.PHONY = all clean install

all: $(BINS)

$(BINS): $(OBJS)
	$(CC) -o $(BINS) $(OBJS) $(LINKER_OPTS) $(LIBS)

$(OBJS): $(SRCS)
	$(CC) -c $(SRCS) $(COMPILER_OPTS) $(INCLUDES)

clean:
	rm -f $(BINS)
	rm -f $(OBJS)
	rm -f *~

install: all
	for $i in $(BINS) ; do \
		$(INSTALL) $(INSTALL_OPTS) $$i $(PREFIX)/bin ; \
	done
