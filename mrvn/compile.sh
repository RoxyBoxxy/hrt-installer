#!/bin/sh -ex

# called from makefile
# compile.sh NAME SRC-DIR DEST-DIR
# Example:
#  compile.sh libdebian-installer . dest

NAME="$1"
DIR="$2/$NAME"
DEST="$3/$NAME"

mkdir -p "$DEST"
cd "$DIR"
fakeroot dpkg-buildpackage -S -us -uc
cd ..
mv *.dsc *.tar.gz *.changes "$DEST"
cd $DEST
dpkg-source -x *.dsc
cd */
debuild -us -uc
cd ..
rm -rf */
