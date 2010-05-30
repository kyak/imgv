####### openwrt-xburst path

BASEPATH      = /home/fcarello/Projects/openwrt-xburst
TARGETPATH    = /staging_dir/target-mipsel_uClibc-0.9.30.1

####### Compiler, tools and options

CC            = mipsel-openwrt-linux-uclibc-gcc
CXX           = mipsel-openwrt-linux-uclibc-g++
STRIP	      = mipsel-openwrt-linux-uclibc-strip
DEFINES       =
CFLAGS        = -pipe -O2 -fomit-frame-pointer -mips32 -mtune=mips32 -funit-at-a-time -fhonour-copts -msoft-float -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -pipe -O2 -fomit-frame-pointer -mips32 -mtune=mips32 -funit-at-a-time -fhonour-copts -msoft-float -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I$(BASEPATH)$(TARGETPATH)/usr/include/ -I$(BASEPATH)$(TARGETPATH)/usr/include/SDL -I.
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = $(SUBLIBS)  -L$(BASEPATH)$(TARGETPATH)/usr/lib/ -lSDL -lSDL_image -lSDL_gfx -ldirectfb -ldirect -lfusion -lz

SOURCE1       = sdl-imageviewer.c
TARGET1       = imgv

####### Build rules

all: Clean Viewer

Clean:
	rm -f $(TARGET1) 
Viewer:
	$(CC) $(CFLAGS) $(INCPATH) $(SOURCE1) -o $(TARGET1) $(LIBS)
	$(STRIP) $(TARGET1)
