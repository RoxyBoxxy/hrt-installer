ifndef TARGETS
TARGETS=netcfg-dynamic netcfg-static
endif

LDOPTS=-ldebconf -ldebian-installer
PREFIX=$(DESTDIR)/usr/
CFLAGS=-Wall  -Os

ifneq (,$(findstring debug,$(DEB_BUILD_OPTIONS)))
CFLAGS += -g
STRIP = /bin/true
else
CFLAGS += -fomit-frame-pointer
STRIPTOOL=strip
STRIP = $(STRIPTOOL) --remove-section=.note --remove-section=.comment
endif

INSTALL=install

all: $(TARGETS)

netcfg-dynamic: netcfg-dynamic.o netcfg.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCS) $(LDOPTS)
	$(STRIP) $@

netcfg-static: netcfg-static.o netcfg.o
	$(CC) $(CFLAGS) -o $@ $^ $(INCS) $(LDOPTS)
	$(STRIP) $@

%.o: %.c %.h netcfg.h
	$(CC) -c $(CFLAGS) $(INCS) -o $@ $<

clean:
	rm -f netcfg-dhcp netcfg-static *.o 
