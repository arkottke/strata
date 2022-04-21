#!/bin/bash

source /opt/qt*/bin/qt*-env.sh

set -o errexit   # abort on nonzero exitstatus
set -o nounset   # abort on unbound variable
set -o pipefail  # don't hide errors within pipes

mkdir deps
cd deps

# Build Qwt
echo "Cloning Qwt"
git clone --branch qwt-6.2 https://git.code.sf.net/p/qwt/git qwt
cd qwt
echo "Configuring Qwt"
qmake
echo "Building Qwt"
make -j2
