#!/bin/bash

source /opt/qt515/bin/qt515-env.sh

echo "Cloning Qwt"
git clone --branch qwt-6.2 https://git.code.sf.net/p/qwt/git qwt
cd qwt
echo "Configuring Qwt"
qmake
echo "Building Qwt"
make -j2

QWT_ROOT_DIR=`pwd`
LD_LIBRARY_PATH=$(readlink -f lib):$LD_LIBRARY_PATH

cd ..

mkdir build
cd build
echo "Configuring Strata"
cmake .. -DQWT_ROOT_DIR=$QWT_ROOT_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:STRING=dist

echo "Building Strata"
cmake --build . --target install

cp ../qwt/lib/libqwt.so dist/usr/bin/

echo "Built $(./dist/usr/bin/strata -v)"
