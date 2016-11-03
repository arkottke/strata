#!/bin/sh

# Run in top-level directory
cd `dirname "$0"`/..
# Stop on error
set -e
# Make sure stdin exists (not true on appveyor msys2)
exec < /dev/null

# If ARCH environment variable not set, choose based on uname -m
if [ -z "$ARCH" -a -z "$XC_HOST" ]; then
  export ARCH=`uname -m`
elif [ -z "$ARCH" ]; then
  ARCH=`echo $XC_HOST | sed 's/-w64-mingw32//'`
fi

version=`python scripts/get_version.py`

if [ "$ARCH" = x86_64 ]; then
  bits=64
else
  bits=32
fi

rm -rf deploy
mkdir deploy

cp release/strata.exe deploy/
cp -r example deploy/
cp LICENSE.txt deploy/

# Copy Qt DLLs
windeployqt deploy/strata.exe --compiler-runtime

# Copy other required DLLs
for dll in 
    libbz2-1.dll \
    libfreetype-6.dll \
    libglib-2.0-0.dll \
    libgraphite2.dll \
    libgsl-19.dll \
    libgslcblas-0.dll \
    libharfbuzz-0.dll \
    libiconv-2.dll \
    libicudt57.dll \
    libicuin57.dll \
    libicuuc57.dll \
    libintl-8.dll \
    libpcre-1.dll \
    libpcre16-0.dll \
    libpng16-16.dll \
    qwt.dll \
    zlib1.dll
do
    cp /mingw$bits/bin/$dll deploy/
done

7z a Strata-mingw$bits-v$version.zip ./deploy/*
