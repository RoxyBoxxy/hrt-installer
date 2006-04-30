
DEB_HOST_ARCH_OS ?= $(shell dpkg-architecture -qDEB_HOST_ARCH_OS 2>/dev/null)

dir = sbin

files_exec = \
	debian-installer \
	debian-installer-startup \
	shutdown \
	init:init-$(DEB_HOST_ARCH_OS)

include ../../Makefile.inc
