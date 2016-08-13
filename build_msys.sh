#!/bin/sh

# Run in top-level directory
cd `dirname "$0"`
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

version=`python get_version.py`
if [ "$ARCH" = x86_64 ]; then
  bits=64
  archsuffix=64
  instdir='/c/Program\ Files/Strata'
else
  bits=32
  archsuffix=86
  instdir='/c/Program Files (x86)/Strata'
fi

echo Using ARCH=$ARCH

export PATH=/mingw$bits/bin:$PATH
# If there is a version of make.exe here, it is mingw32-make which won't work
rm -f /mingw$bits/bin/make.exe

# Add library paths
export LD_LIBRARY_PATH="/mingw$bits/lib"
export LIBRARY_PATH="/mingw$bits/lib"
export CPLUS_INCLUDE_PATH="/mingw$bits/include/qwt"

qmake 
make -j2 release

./release/strata -b example/*.strata
./release/strata -b example/*.json

# Compile the NSIS script
/c/'Program Files (x86)'/NSIS/makensis.exe \
    -DVERSION="$version" \
    -DSTRATA_PATH="release/strata.exe" \
    -DARCH="mingw$bits" \
    -DINSTDIR=`cygpath -w $instdir` \
    installer.nsi
