#!/bin/sh

echo $QWT_ROOT_DIR $GSL_ROOT_DIR

ls $QWT_ROOT_DIR

find $QWT_ROOT_DIR -iname "*qwt_plot.h"

mkdir build
cd build

cmake .. \
    -DQWT_ROOT_DIR=$QWT_ROOT_DIR \
    -DQWT_INCLUDE_DIR=$QWT_INCLUDE_DIR \
    -DGSL_ROOT_DIR=$GSL_ROOT_DIR \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX:STRING=dist

cmake --build . --target install

# Return the build dir
cd $TRAVIS_BUILD_DIR
