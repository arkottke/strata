#!/bin/sh

mkdir build
cd build

echo $TRAVIS_OS_NAME

if [ "$TRAVIS_OS_NAME" == "linux" ]; then 
    cmake .. \
        -DQWT_ROOT_DIR=$QWT_ROOT_DIR \
        -DGSL_ROOT_DIR=$GSL_ROOT_DIR \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX:STRING=dist
else
    echo "Building with QWT_INCLUDE_DIR variable."
    cmake .. \
        -DQWT_ROOT_DIR=$QWT_ROOT_DIR \
        -DQWT_INCLUDE_DIR=$QWT_INCLUDE_DIR \
        -DGSL_ROOT_DIR=$GSL_ROOT_DIR \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX:STRING=dist
fi

cmake --build . --target install

# Return the build dir
cd $TRAVIS_BUILD_DIR
