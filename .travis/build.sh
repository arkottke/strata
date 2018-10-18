#!/bin/sh

# Exit script if you try to use an uninitialized variable.
set -o nounset
# Exit script if a statement returns a non-true return value.
set -o errexit
# Use the error status of the first failure, rather than that of the last item in a pipeline.
set -o pipefail

mkdi build
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
