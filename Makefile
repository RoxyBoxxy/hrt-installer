DEST=$(PWD)/dest

all: build-depends libdebian-installer tools_cdebconf main-menu tools_udpkg anna utils tools_languagechooser tools_ddetect tools_cdrom-detect tools_autopartkit tools_base-installer tools_baseconfig-udeb tools_bterm-unifont tools_bugreporter-udeb tools_cdrom-checker tools_kbd-chooser tools_lvmcfg tools_netcfg tools_partconf tools_partitioner tools_pcmcia-udeb tools_prebaseconfig tools_usb-discover tools_userdevfs retriever_cdrom retriever_choose-mirror retriever_file retriever_floppy retriever_net rootskel

i386: tools_grub-installer tools_lilo-installer

clean:
	rm .stamp-*
	rm -rf $(DEST)

build-depends:
	# all
	sudo apt-get </dev/tty install automake autoconf libtool fakeroot devscripts apt autotools-dev bc bf-utf-source bison d-shlibs dash debhelper dpkg-dev flex genext2fs glibc-pic iso-codes libgtk2.0-dev libnewt-dev libnewt-pic  libparted1.6-dev libperl-dev libtextwrap-dev libtextwrap1 locales mklibs modutils ncurses-base po-debconf sed slang1-utf8-dev slang1-utf8-pic wget

	# !hurd-i386
	sudo apt-get </dev/tty install libbogl-dev

	#i386
	#sudo apt-get </dev/tty install syslinux sysutils dosfstools mtools upx-ucl-beta kernel-image-2.4.22-1-386  kernel-pcmcia-modules-2.4.22-1-386

	# [!s390 !s390x !mips !mipsel]
	sudo apt-get </dev/tty install libdiscover1-pic libdiscover1

	# mips
	#sudo apt-get </dev/tty install tip22

rootskel: .stamp-rootskel
.stamp-rootskel: 
	./compile.sh rootskel . $(DEST)
	touch .stamp-rootskel

libdebian-installer: .stamp-libdebian-installer
.stamp-libdebian-installer:
	cd libdebian-installer; autoreconf -i
	./compile.sh libdebian-installer . $(DEST)
	touch .stamp-libdebian-installer

tools_cdebconf: .stamp-tools_cdebconf
.stamp-tools_cdebconf:
	./compile.sh cdebconf tools $(DEST)
	touch .stamp-tools_cdebconf

main-menu: install-libdebconfclient-dev install-libdebian-installer4-dev .stamp-main-menu
.stamp-main-menu: 
	./compile.sh main-menu . $(DEST)
	touch .stamp-main-menu

tools_udpkg: .stamp-tools_udpkg
.stamp-tools_udpkg:
	./compile.sh udpkg tools $(DEST)
	touch .stamp-tools_udpkg

anna: install-libdebconfclient-dev install-libdebian-installer4-dev .stamp-anna
.stamp-anna:
	./compile.sh anna . $(DEST)
	touch .stamp-anna

utils: install-libdebconfclient-dev install-libdebian-installer4-dev .stamp-utils
.stamp-utils:
	./compile.sh utils . $(DEST)
	touch .stamp-utils

tools_languagechooser: .stamp-tools_languagechooser
.stamp-tools_languagechooser:
	./compile.sh languagechooser tools $(DEST)
	touch .stamp-tools_languagechooser

tools_ddetect: .stamp-tools_ddetect
.stamp-tools_ddetect:
	./compile.sh ddetect tools $(DEST)
	touch .stamp-tools_ddetect

tools_cdrom-detect: .stamp-tools_cdrom-detect
.stamp-tools_cdrom-detect:
	./compile.sh cdrom-detect tools $(DEST)
	touch .stamp-tools_cdrom-detect

tools_autopartkit: install-libdebconfclient-dev .stamp-tools_autopartkit
.stamp-tools_autopartkit:
	cd tools/autopartkit; autoreconf -i
	./compile.sh autopartkit tools $(DEST)
	touch .stamp-tools_autopartkit

tools_base-installer: install-libdebconfclient-dev install-libdebian-installer4-dev .stamp-tools_base-installer
.stamp-tools_base-installer:
	./compile.sh base-installer tools $(DEST)
	touch .stamp-tools_base-installer

tools_baseconfig-udeb: .stamp-tools_baseconfig-udeb
.stamp-tools_baseconfig-udeb:
	./compile.sh baseconfig-udeb tools $(DEST)
	touch .stamp-tools_baseconfig-udeb

tools_bterm-unifont: .stamp-tools_bterm-unifont
.stamp-tools_bterm-unifont:
	./compile.sh bterm-unifont tools $(DEST)
	touch .stamp-tools_bterm-unifont

tools_bugreporter-udeb: .stamp-tools_bugreporter-udeb
.stamp-tools_bugreporter-udeb:
	./compile.sh bugreporter-udeb tools $(DEST)
	touch .stamp-tools_bugreporter-udeb

tools_cdrom-checker: install-libdebconfclient-dev install-libdebian-installer4-dev .stamp-tools_cdrom-checker
.stamp-tools_cdrom-checker:
	./compile.sh cdrom-checker tools $(DEST)
	touch .stamp-tools_cdrom-checker

tools_grub-installer: .stamp-tools_grub-installer
.stamp-tools_grub-installer:
	./compile.sh grub-installer tools $(DEST)
	touch .stamp-tools_grub-installer

tools_kbd-chooser: install-libdebconfclient-dev install-libdebian-installer4-dev .stamp-tools_kbd-chooser
.stamp-tools_kbd-chooser:
	./compile.sh kbd-chooser tools $(DEST)
	touch .stamp-tools_kbd-chooser

tools_lilo-installer: .stamp-tools_lilo-installer
.stamp-tools_lilo-installer:
	./compile.sh lilo-installer tools $(DEST)
	touch .stamp-tools_lilo-installer

tools_lvmcfg: .stamp-tools_lvmcfg
.stamp-tools_lvmcfg:
	./compile.sh lvmcfg tools $(DEST)
	touch .stamp-tools_lvmcfg

tools_netcfg: install-libdebconfclient-dev install-libdebian-installer4-dev .stamp-tools_netcfg
.stamp-tools_netcfg:
	./compile.sh netcfg tools $(DEST)
	touch .stamp-tools_netcfg

tools_partconf: install-libdebconfclient-dev install-libdebian-installer4-dev .stamp-tools_partconf
.stamp-tools_partconf:
	./compile.sh partconf tools $(DEST)
	touch .stamp-tools_partconf

tools_partitioner: install-libdebconfclient-dev install-libdebian-installer4-dev .stamp-tools_partitioner
.stamp-tools_partitioner:
	./compile.sh partitioner tools $(DEST)
	touch .stamp-tools_partitioner

tools_pcmcia-udeb: .stamp-tools_pcmcia-udeb
.stamp-tools_pcmcia-udeb:
	./compile.sh pcmcia-udeb tools $(DEST)
	touch .stamp-tools_pcmcia-udeb

tools_prebaseconfig: .stamp-tools_prebaseconfig
.stamp-tools_prebaseconfig:
	./compile.sh prebaseconfig tools $(DEST)
	touch .stamp-tools_prebaseconfig

tools_usb-discover: .stamp-tools_usb-discover
.stamp-tools_usb-discover:
	./compile.sh usb-discover tools $(DEST)
	touch .stamp-tools_usb-discover

tools_userdevfs: .stamp-tools_userdevfs
.stamp-tools_userdevfs:
	./compile.sh userdevfs tools $(DEST)
	touch .stamp-tools_userdevfs

retriever_cdrom: .stamp-retriever_cdrom
.stamp-retriever_cdrom:
	./compile.sh cdrom retriever $(DEST)
	touch .stamp-retriever_cdrom

retriever_choose-mirror: install-libdebconfclient-dev .stamp-retriever_choose-mirror
.stamp-retriever_choose-mirror:
	./compile.sh choose-mirror retriever $(DEST)
	touch .stamp-retriever_choose-mirror

retriever_file: .stamp-retriever_file
.stamp-retriever_file:
	./compile.sh file retriever $(DEST)
	touch .stamp-retriever_file

retriever_floppy: .stamp-retriever_floppy
.stamp-retriever_floppy:
	./compile.sh floppy retriever $(DEST)
	touch .stamp-retriever_floppy

retriever_net: .stamp-retriever_net
.stamp-retriever_net:
	./compile.sh net retriever $(DEST)
	touch .stamp-retriever_net

install-libdebconfclient-dev: .stamp-tools_cdebconf .stamp-install-libdebconfclient-dev
.stamp-install-libdebconfclient-dev:
	sudo dpkg -i $(DEST)/cdebconf/libdebconfclient0*.deb
	touch .stamp-install-libdebconfclient-dev

install-libdebian-installer4-dev: .stamp-libdebian-installer .stamp-install-libdebian-installer4-dev
.stamp-install-libdebian-installer4-dev:
	sudo dpkg -i $(DEST)/libdebian-installer/libdebian-installer4*.deb
	touch .stamp-install-libdebian-installer4-dev
