#!/bin/bash

source /opt/qt*/bin/qt*-env.sh

set -o errexit   # abort on nonzero exitstatus
set -o nounset   # abort on unbound variable
set -o pipefail  # don't hide errors within pipes

# Build Qwt
echo "Cloning Qwt"
git clone --branch qwt-6.2 https://git.code.sf.net/p/qwt/git qwt
cd qwt
echo "Configuring Qwt"
qmake
echo "Building Qwt"
make -j2

QWT_ROOT_DIR=`pwd`

# Build Strata
cd ..
mkdir build
cd build
echo "Configuring Strata"
cmake .. -DQWT_ROOT_DIR=$QWT_ROOT_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:STRING=dist

echo "Building Strata"
cmake --build . --target install

mv ../qwt/lib dist/usr/lib

cd dist/usr/bin

echo "Built $(./dist/usr/bin/strata -v)"
