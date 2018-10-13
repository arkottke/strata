#!/bin/sh

set -ev

source /opt/qt510/bin/qt510-env.sh
# Install QWT
cd $TRAVIS_BUILD_DIR/..
svn checkout svn://svn.code.sf.net/p/qwt/code/branches/qwt-6.1 qwt
cd qwt
QWT_ROOT_DIR=`pwd`
LD_LIBRARY_PATH=$(readlink -f lib):$LD_LIBRARY_PATH
qmake
make -j$(nproc)
# Install GSL 
GSL_ROOT_DIR=/opt/gsl
cd $TRAVIS_BUILD_DIR/..
git clone https://github.com/ampl/gsl.git
cd gsl
mkdir build && cd build
cmake .. -DGSL_DISABLE_WARNINGS:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_BUILD_TYPE="Release" -DCMAKE_INSTALL_PREFIX:STRING=$GSL_ROOT_DIR
cmake --build . --target install
cd $GSL_ROOT_DIR
LD_LIBRARY_PATH=$(readlink -f lib):$LD_LIBRARY_PATH

