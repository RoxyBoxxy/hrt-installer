
DEB_HOST_ARCH_OS ?= $(shell dpkg-architecture -qDEB_HOST_ARCH_OS 2>/dev/null)

dir = sbin

files_exec = \
	debian-installer \
	debian-installer-startup \
	shutdown \
	init:init-$(DEB_HOST_ARCH_OS)

ifeq ($(DEB_HOST_ARCH_OS),linux)
  files_exec += \
	reopen-console \
	console-type \
	steal-ctty
endif

console-type: console-type.c
	gcc -Os -Wall console-type.c -o console-type

steal-ctty: steal-ctty.c
	gcc -Os -Wall steal-ctty.c -o steal-ctty

build: console-type steal-ctty

clean:
	rm -f console-type steal-ctty

include ../../Makefile.inc
