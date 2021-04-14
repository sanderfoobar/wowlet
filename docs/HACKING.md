# Documentation for developers

WOWlet is developed primarily on Linux. It uses Qt 5.15.* and chances are that your 
distro's package manager has a lower version. It is therefore recommended that you install 
Qt manually using the online installer, which can be found here: https://www.qt.io/download 
(under open-source).

## Jetbrains Clion

WOWlet was developed using JetBrains Clion since it integrates nicely 
with CMake and comes with a built-in debugger. To pass CMake flags to CLion, 
go to `File->Settings->Build->CMake`, set Build Type to `Debug` and set your 
preferred CMake options/definitions.

## Man Page

There is a WOWlet's manual page, which can be accessed with: `man wowlet`

If a new option is introduced, please be sure to update the options section in
`src/assets/wowlet.1.md`, the month and year in line 3, and "manify" the document
by running this command: `pandoc wowlet.1.md -s -t man -o wowlet.1 && gzip wowlet.1`

## Requirements

### Ubuntu/Debian

```bash
apt install -y git cmake libqrencode-dev build-essential cmake libboost-all-dev \
miniupnpc libunbound-dev graphviz doxygen libunwind8-dev pkg-config libssl-dev \
libzmq3-dev libsodium-dev libhidapi-dev libnorm-dev libusb-1.0-0-dev libpgm-dev \
libprotobuf-dev protobuf-compiler libgcrypt20-dev
```

## Mac OS

```bash
brew install boost zmq openssl libpgm miniupnpc libsodium expat libunwind-headers \
protobuf libgcrypt qrencode ccache cmake pkgconfig git
```

## CMake

After installing Qt you might have a folder called `/home/$user/Qt/`. You need to pass this to CMake 
via the `CMAKE_PREFIX_PATH` definition.

```
-DCMAKE_PREFIX_PATH=/home/$user/QtNew/5.15.0/gcc_64
```

There are some Wownero/WOWlet related options/definitions that you may pass:

- `-DXMRIG=OFF` - disable XMRig feature
- `-DTOR_BIN=/path/to/tor` - Embed a Tor executable inside WOWlet
- `-DDONATE_BEG=OFF` - disable the dreaded donate requests

And:

```
-DMANUAL_SUBMODULES=1  
-DUSE_DEVICE_TREZOR=OFF 
-DUSE_SINGLE_BUILDDIR=ON 
-DDEV_MODE=ON 
```

If you have OpenSSL installed in a custom location, try:

```
-DOPENSSL_INCLUDE_DIR=/usr/local/lib/openssl-1.1.1g/include 
-DOPENSSL_SSL_LIBRARY=/usr/local/lib/openssl-1.1.1g/libssl.so.1.1 
-DOPENSSL_CRYPTO_LIBRARY=/usr/local/lib/openssl-1.1.1g/libcrypto.so.1.1
```

I prefer also enabling verbose makefiles, which may be useful in some situations.

```
-DCMAKE_VERBOSE_MAKEFILE=ON
```

Enable debugging symbols:

```bash
-DCMAKE_BUILD_TYPE=Debug
```

## Wowlet

It's best to install Tor locally as a service and start `wowlet` with `--use-local-tor`, this 
prevents the child process from starting up and saves time.

#### Ubuntu/Debian

```bash
apt install -y tor
sudo service tor start
```

#### Mac OS

```bash
brew install tor
brew services start tor
```

To skip the wizards and open a wallet directly use `--wallet-file`: 

```bash
./wowlet --use-local-tor --wallet-file /home/user/Wownero/wallets/bla.keys
```

It is recommended that you use `--stagenet` for development. Testnet is also possible, 
but you'll have to provide Wownero a testnet node of your own.
 
