#!/bin/bash

git clone --branch qwt-6.2 https://git.code.sf.net/p/qwt/git qwt
cd qwt
source /opt/qt515/bin/qt515-env.sh
qmake --version
qmake
make -j2
