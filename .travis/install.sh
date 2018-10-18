#!/bin/sh

if [ "$TRAVIS_OS_NAME" == "linux" ]; then 
    # Install Qt 5.10
    sudo add-apt-repository --yes ppa:beineri/opt-qt-5.10.1-trusty
    sudo apt-get update -qq
    sudo apt-get install -qq libgsl0-dev qt510base qt510tools qt510svg

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
else
    export PATH="/usr/local/opt/qt/bin:$PATH"
fi
