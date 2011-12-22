#!/bin/sh
echo Compile NDK part in RELEASE build.

rm -Rf ../obj
ndk-build -B NDK_DEBUG=0 V=0
