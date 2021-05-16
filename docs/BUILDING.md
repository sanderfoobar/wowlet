## Buildbot builds

The docker build bins can be found here: https://build.wownero.org/files/

## Docker static builds

Static builds via Docker are done in 3 steps:

1. Cloning this repository (+submodules)
2. Creating a base Docker image
3. Using the base image to compile a build

### Linux (reproducible)

The docker image for reproducible Linux static builds uses Ubuntu 16.04 and compiles the required libraries statically 
so that the resulting `wowlet` binary is static. For more information, check the Dockerfile: `Dockerfile`.

#### 1. Clone

```bash
git clone --branch master --recursive https://git.wownero.com/wowlet/wowlet.git
cd wowlet
```

Replace `master` with the desired version tag (e.g. `beta-4`) to build the release binary.

#### 2. Base image

```bash
docker build --tag wowlet:linux --build-arg THREADS=6 .
```

Building the base image takes a while. You only need to build the base image once.

#### 3. Build

##### Standalone binary

```bash
docker run --rm -it -v $PWD:/wowlet -w /wowlet wowlet:linux sh -c 'make release-static -j6'
```

If you're re-running a build make sure to `rm -rf build/` first.

The resulting binary can be found in `build/bin/wowlet`.

##### AppImage

First create the standalone binary using the Docker command in the previous step.

```bash
docker run --rm -it -v $PWD:/wowlet -w /wowlet wowlet:linux contrib/build-appimage.sh
```

### Windows (reproducible)

#### 1. Clone

```bash
git clone --branch master --recursive https://git.wownero.com/wowlet/wowlet.git
cd wowlet
```

Replace `master` with the desired version tag (e.g. `beta-4`) to build the release binary.

#### 2. Base image


```bash
docker build -f Dockerfile.windows --tag wowlet:win --build-arg THREADS=6 .
```

Building the base image takes a while. You only need to build the base image once.

#### 3. Build

```bash
docker run --rm -it -v $PWD:/wowlet -w /wowlet wowlet:win sh -c 'make windows root=/depends target=x86_64-w64-mingw32 tag=win-x64 -j6'
```

If you're re-running a build make sure to `rm -rf build/` first.

The resulting binary can be found in `build/x86_64-w64-mingw32/release/bin/wowlet.exe`.

## macOS

For MacOS it's easiest to leverage [brew](https://brew.sh) to install the required dependencies. 

```bash
HOMEBREW_OPTFLAGS="-march=core2" HOMEBREW_OPTIMIZATION_LEVEL="O0" \
    brew install boost zmq openssl libpgm miniupnpc libsodium expat libunwind-headers protobuf libgcrypt qrencode ccache cmake pkgconfig git
```

Clone the repository.

```bash
git clone --recursive https://git.wownero.com/wowlet/wowlet.git
``` 

Get the latest LTS from here: https://www.qt.io/offline-installers and install.

Build WOWlet.

```bash
CMAKE_PREFIX_PATH=~/Qt5.15.1/5.15.1/clang_64 make mac-release
```

The resulting Mac OS application can be found `build/bin/wowlet.app` and will **not** have Tor embedded.
