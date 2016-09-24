CC = g++
CFLAGS := -g -Wall $(shell pkg-config libmusicbrainz5 --cflags) -std=c++14
LDFLAGS := $(shell pkg-config libmusicbrainz5 --libs)
ROOT := /home/wieland/dev/emacs

all: musicbrainz.so

musicbrainz.so: musicbrainz.o
	$(CC) $(LDFLAGS) -shared -o $@ $<

%.o: %.cpp
	$(CC) $(CFLAGS) -I$(ROOT)/src -fPIC -c $<

interactive:
	$(ROOT)/src/emacs -Q --eval "(add-to-list 'load-path \"$(CURDIR)\")" -l musicbrainz-interactive.el
