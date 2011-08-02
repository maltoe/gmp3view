#
# Makefile for gmp3view
#

VERSION=0.4.2alpha1

PREFIX=/usr/local
CC=gcc
CPP=g++
CPPFLAGS=`pkg-config --cflags gtk+-3.0` -Wall -g -D _DEBUG_
LDFLAGS=`pkg-config --libs gtk+-3.0` `pkg-config --libs sqlite3` /usr/lib/libjpeg.so

all: clean gmp3view

gmp3view.o:
	$(CPP) -c $(CPPFLAGS) gmp3view.cpp -o gmp3view.o
	
db.o:
	$(CPP) -c $(CPPFLAGS) db.cpp -o db.o	

base64.o:
	$(CPP) -c $(CPPFLAGS) base64.cpp -o base64.o	
	
collect.o:
	$(CPP) -c $(CPPFLAGS) collect.cpp -o collect.o	

interface.o:
	$(CPP) -c $(CPPFLAGS) interface.cpp -o interface.o
	
jpeg.o:
	$(CPP) -c $(CPPFLAGS) jpeg.cpp -o jpeg.o

extern.o:
	$(CPP) -c $(CPPFLAGS) extern.cpp -o extern.o
	
imageio/jmem_dest.o:
	$(CPP) -c $(CPPFLAGS) imageio/jmem_dest.c -o imageio/jmem_dest.o
	
imageio/jmem_src.o:
	$(CPP) -c $(CPPFLAGS) imageio/jmem_src.c -o imageio/jmem_src.o
	
searchwin.o:
	$(CPP) -c $(CPPFLAGS) searchwin.cpp -o searchwin.o
	
	
gmp3view: gmp3view.o db.o base64.o collect.o interface.o imageio/jmem_dest.o imageio/jmem_src.o jpeg.o extern.o searchwin.o
	$(CPP) $(CPPFLAGS) $(LDFLAGS) *.o imageio/*.o -o gmp3view 
	
clean:
	rm -f *.o imageio/*.o gmp3view
	
install: gmp3view
	/usr/bin/install -g 0 -o 0 --mode=755 gmp3view $(PREFIX)/bin
	mkdir -p --mode=755 $(PREFIX)/share/gmp3view
	/usr/bin/install -g 0 -o 0 --mode=644 img/*png $(PREFIX)/share/gmp3view
