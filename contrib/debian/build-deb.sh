#!/bin/bash
set -e

# Make directories

DEBDIR="$PWD/wowlet.DebDir"
mkdir -p "$DEBDIR"
mkdir -p "$DEBDIR/DEBIAN/"
mkdir -p "$DEBDIR/debian/"
mkdir -p "$DEBDIR/usr/bin/"
mkdir -p "$DEBDIR/usr/share/doc/wowlet/"
mkdir -p "$DEBDIR/usr/share/metainfo/"
mkdir -p "$DEBDIR/usr/share/applications/"
mkdir -p "$DEBDIR/usr/share/man/man1/"
mkdir -p "$DEBDIR/usr/share/icons/hicolor/scalable/apps/"
mkdir -p "$DEBDIR/usr/share/icons/hicolor/48x48/apps/"
mkdir -p "$DEBDIR/usr/share/icons/hicolor/64x64/apps/"
mkdir -p "$DEBDIR/usr/share/icons/hicolor/96x96/apps/"
mkdir -p "$DEBDIR/usr/share/icons/hicolor/128x128/apps/"
mkdir -p "$DEBDIR/usr/share/icons/hicolor/192x192/apps/"
mkdir -p "$DEBDIR/usr/share/icons/hicolor/256x256/apps/"

# Copy over assets

cp "$PWD/contrib/debian/control" "$DEBDIR/DEBIAN/control"
cp "$PWD/contrib/debian/copyright" "$DEBDIR/debian/copyright"
cp "$PWD/contrib/debian/copyright" "$DEBDIR/usr/share/doc/wowlet/copyright"
cp "$PWD/contrib/debian/changelog.gz" "$DEBDIR/usr/share/doc/wowlet/changelog.gz"
cp "$PWD/src/assets/org.wowlet.wowlet.metainfo.xml" "$DEBDIR/usr/share/metainfo/org.wowlet.wowlet.metainfo.xml"
cp "$PWD/README.md" "$DEBDIR/usr/share/doc/wowlet/README"
cp "$PWD/docs/SECURITY.md" "$DEBDIR/usr/share/doc/wowlet/SECURITY"
cp "$PWD/build/bin/wowlet" "$DEBDIR/usr/bin/wowlet"
cp "$PWD/src/assets/org.wowlet.wowlet.desktop" "$DEBDIR/usr/share/applications/org.wowlet.wowlet.desktop"
cp "$PWD/src/assets/wowlet.1.gz" "$DEBDIR/usr/share/man/man1/wowlet.1.gz"
cp "$PWD/src/assets/images/appicons/wowlet.svg" "$DEBDIR/usr/share/icons/hicolor/scalable/apps/wowlet.svg"
cp "$PWD/src/assets/images/appicons/48x48.png" "$DEBDIR/usr/share/icons/hicolor/48x48/apps/wowlet.png"
cp "$PWD/src/assets/images/appicons/64x64.png" "$DEBDIR/usr/share/icons/hicolor/64x64/apps/wowlet.png"
cp "$PWD/src/assets/images/appicons/96x96.png" "$DEBDIR/usr/share/icons/hicolor/96x96/apps/wowlet.png"
cp "$PWD/src/assets/images/appicons/128x128.png" "$DEBDIR/usr/share/icons/hicolor/128x128/apps/wowlet.png"
cp "$PWD/src/assets/images/appicons/192x192.png" "$DEBDIR/usr/share/icons/hicolor/192x192/apps/wowlet.png"
cp "$PWD/src/assets/images/appicons/256x256.png" "$DEBDIR/usr/share/icons/hicolor/256x256/apps/wowlet.png"

# Build deb package

dpkg-deb --build $DEBDIR
mv wowlet.DebDir.deb wowlet_2.1_amd64.deb
