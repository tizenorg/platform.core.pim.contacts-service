CC = gcc

REQUIRED_PKG = contacts-service
CFLAGS = -g -Wall
LDFLAGS = # -L../ -lefence -pthread
ifdef REQUIRED_PKG
	CFLAGS += `pkg-config --cflags $(REQUIRED_PKG)`
	LDFLAGS += `pkg-config --libs $(REQUIRED_PKG)`
endif

SRCS = contact-test.c phonelog-test.c change-noti-test.c group-test.c vcard2contact-test.c SIMimport-test.c addressbook-test.c
TIMESRC = timetest.c
OBJECTS = $(SRCS:.c=.o)
TIMEOBJ = $(TIMESRC:.c=.o)
TARGETS = $(OBJECTS:.o=)
#A:.c=.o  //A안에 있는 .c를 .o로 바꿔라


all: $(OBJECTS) $(TARGETS)
#-mv test1 testlocal /usr/

$(TARGETS): $(TIMEOBJ)
$(TIMEOBJ): timetest.h

% : %.o
	$(CC) -o $@ $< $(TIMEOBJ) $(LDFLAGS)

clean:
	rm -rf $(OBJECTS) $(TARGETS) $(TIMEOBJ)

