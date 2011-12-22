#!/bin/sh
echo Compile NDK part in DEBUG build.

rm -Rf ../obj
ndk-build -B NDK_DEBUG=1 V=0
