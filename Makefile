####### openwrt-xburst path

BASEPATH      = /home/fcarello/Projects/openwrt-xburst
TARGETPATH    ?= $(BASEPATH)/staging_dir/target-mipsel_uClibc-0.9.32

####### Compiler, tools and options

CC            = mipsel-openwrt-linux-gcc
STRIP         = mipsel-openwrt-linux-strip
CFLAGS        := $(CFLAGS) -pipe -O2 -fomit-frame-pointer -mips32 -mtune=mips32 -funit-at-a-time -fhonour-copts -msoft-float -Wall -W -D_REENTRANT
INCPATH       = -I$(TARGETPATH)/usr/include/ -I$(TARGETPATH)/usr/include/SDL -I.
LIBS          = -L$(TARGETPATH)/usr/lib/ -lSDL -lSDL_image -lSDL_gfx -lSDL_ttf -lfreetype -ldirectfb -ldirect -lfusion -lz -lpthread

SOURCE1       = sdl-imageviewer.c
TARGET1       = imgv

####### Build rules

all:
	$(CC) $(CFLAGS) $(INCPATH) $(SOURCE1) -o $(TARGET1) $(LIBS)
	$(STRIP) $(TARGET1)
clean:
	rm -f $(TARGET1) 
 
