#!/bin/bash
set -e

# Make directories

DEBDIR="$PWD/wowlet.DebDir"
mkdir -p "$DEBDIR"
mkdir -p "$DEBDIR/DEBIAN/"
mkdir -p "$DEBDIR/usr/share/applications/"
mkdir -p "$DEBDIR/usr/share/pixmaps/"
mkdir -p "$DEBDIR/usr/bin"

# Copy over assets

cp "$PWD/src/assets/wowlet.desktop" "$DEBDIR/usr/share/applications/wowlet.desktop"
cp "$PWD/src/assets/images/wowlet.png" "$DEBDIR/usr/share/pixmaps/wowlet.png"
cp "$PWD/build/bin/wowlet" "$DEBDIR/usr/bin/wowlet"
cp "$PWD/src/assets/control" "$DEBDIR/DEBIAN/control"

# Build deb package

dpkg-deb --build $DEBDIR
mv wowlet.DebDir.deb wowlet.deb
