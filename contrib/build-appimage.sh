#!/bin/bash

set -e
unset SOURCE_DATE_EPOCH

APPDIR="$PWD/wowlet.AppDir"

mkdir -p "$APPDIR"
mkdir -p "$APPDIR/usr/share/applications/"
mkdir -p "$APPDIR/usr/bin"

cp "$PWD/src/assets/org.wowlet.wowlet.desktop" "$APPDIR/usr/share/applications/org.wowlet.wowlet.desktop"
cp "$PWD/src/assets/images/appicons/64x64.png" "$APPDIR/wowlet.png"
cp "$PWD/build/bin/wowlet" "$APPDIR/usr/bin/wowlet"

LD_LIBRARY_PATH=/usr/local/lib /linuxdeployqt/squashfs-root/AppRun wowlet.AppDir/usr/share/applications/org.wowlet.wowlet.desktop -bundle-non-qt-libs

find wowlet.AppDir/ -exec touch -h -a -m -t 202101010100.00 {} \;

# Manually create AppImage (reproducibly)

# download runtime
wget -nc https://github.com/AppImage/AppImageKit/releases/download/12/runtime-x86_64
echo "24da8e0e149b7211cbfb00a545189a1101cb18d1f27d4cfc1895837d2c30bc30 runtime-x86_64" | sha256sum -c

mksquashfs wowlet.AppDir wowlet.squashfs -info -root-owned -no-xattrs -noappend -fstime 0
# mksquashfs writes a timestamp to the header
printf '\x00\x00\x00\x00' | dd conv=notrunc of=wowlet.squashfs bs=1 seek=$((0x8))

rm -f wowlet.AppImage
cat runtime-x86_64 >> wowlet.AppImage
cat wowlet.squashfs >> wowlet.AppImage
chmod a+x wowlet.AppImage
