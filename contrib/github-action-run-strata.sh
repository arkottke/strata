#!/bin/bash

source /opt/qt*/bin/qt*-env.sh

set -o errexit   # abort on nonzero exitstatus

LD_LIBRARY_PATH=$(readlink -f dist/usr/lib):$LD_LIBRARY_PATH

./dist/usr/bin/strata -b "$1"
