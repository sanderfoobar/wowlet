# Wowlet Mobile (Android)

This directory contains an unfinished QML application that will show:

![https://i.imgur.com/yhCsSgj.jpg](https://i.imgur.com/yhCsSgj.jpg)

## Building

Credits go to Monero GUI team for providing the initial work on Qt5+Android.

Build a Docker image:

```bash
docker build --tag wowlet:android --build-arg THREADS=14 --file Dockerfile.android .
```

Building Wowlet for arm64-v8a:

```Bash
docker run --rm -it -v $PWD:/wowlet -w /wowlet -e THREADS=6 wowlet:android
```

Installing the resulting `.apk` on your device:

```bash
adb install build/Android/release/android-build//build/outputs/apk/debug/android-build-debug.apk
```

Viewing debug logs:

```bash
adb logcat | grep --line-buffered "D wowlet"
```

# Development

To show this on desktop, you will need the following CMake definitions:

`-DANDROID_DEBUG=ON -DWITH_SCANNER=ON`

Start wowlet with the `--android-debug` flag:

```bash
./wowlet --android-debug
```
