#!/bin/sh

# Install dependencies
#  - Qt 5.9
#  - GSL
sudo add-apt-repository --yes ppa:beineri/opt-qt593-trusty
sudo apt-get update -qq
sudo apt-get install -qq libgsl0-dev qt59base qt59tools qt59svg

ls -l /opt/qt59/bin
source /opt/qt59/bin/qt59-env.sh

# Instal QWT
cd $TRAVIS_BUILD_DIR/..
svn checkout svn://svn.code.sf.net/p/qwt/code/branches/qwt-6.1 qwt
cd qwt
QWT_ROOT_DIR=`pwd`
LD_LIBRARY_PATH=$(readlink -f lib):$LD_LIBRARY_PATH
qmake
make -j$(nproc)
