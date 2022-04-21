#!/bin/bash

source /opt/qt*/bin/qt*-env.sh

set -o errexit   # abort on nonzero exitstatus
set -o nounset   # abort on unbound variable
set -o pipefail  # don't hide errors within pipes

QWT_ROOT_DIR=$(readlink -f deps/qwt)

# Build Strata
mkdir build
cd build
echo "Configuring Strata"
cmake .. -DQWT_ROOT_DIR=$QWT_ROOT_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:STRING=dist

echo "Building Strata"
cmake --build . --target install

cp -ax $QWT_ROOT_DIR/lib dist/usr/lib

cd dist/usr/bin
echo "Built $(./dist/usr/bin/strata -v)"
