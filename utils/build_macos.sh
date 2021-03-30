#!/usr/bin/env bash
# Used for macos buildbot
HASH="$1"
echo "[+] hash: $HASH"

export DRONE=true
echo "[+] Building"

rm ~/wowlet.zip 2>&1 >/dev/null
cd ~/wowlet
git fetch
git reset --hard "$HASH"
git submodule update --init --depth 120 monero
git submodule update --init --depth 120 --recursive monero

cp "/Users/administrator/tor/libevent-2.1.7.dylib" "/Users/administrator/wowlet/src/assets/exec/libevent-2.1.7.dylib"
CMAKE_PREFIX_PATH="~/Qt/5.15.1/clang_64" TOR_BIN="/Users/administrator/tor/tor" make -j3 mac-release

if [[ $? -eq 0 ]]; then
    echo "[+] WOWlet built OK"
    cd ~/wowlet/build/bin
    zip -qr ~/wowlet.zip wowlet.app
else
    echo "[+] Error!"
    exit 1;
fi
