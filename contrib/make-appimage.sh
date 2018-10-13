#!/bin/sh

cd $TRAVIS_BUILD_DIR
mkdir build
cd build
cmake .. -DQWT_ROOT_DIR=$QWT_ROOT_DIR -DGSL_ROOT_DIR=$GSL_ROOT_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:STRING=dist
cmake --build . --target install
