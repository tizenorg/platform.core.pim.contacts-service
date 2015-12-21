CC = gcc

REQUIRED_PKG =
CFLAGS = -g -Wall #-fprofile-arcs -ftest-coverage
LDFLAGS =
ifdef REQUIRED_PKG
	CFLAGS += `pkg-config --cflags $(REQUIRED_PKG)`
	LDFLAGS += `pkg-config --libs $(REQUIRED_PKG)`
endif

SRCS = $(wildcard *.c)
OBJECTS = $(SRCS:.c=.o)
TARGETS = $(OBJECTS:.o=)

all: $(TARGETS)

% : %.o
	$(CC) $(LDFLAGS) -o $@ $<

clean:
	rm -rf $(OBJECTS) $(TARGETS)

