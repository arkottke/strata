# Strata

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Build Status](https://travis-ci.org/arkottke/strata.svg?branch=master)](https://travis-ci.org/arkottke/strata)
[![Build status](https://ci.appveyor.com/api/projects/status/cpgr2vsh1re8c35x/branch/master?svg=true)](https://ci.appveyor.com/project/arkottke/strata/branch/master)
[![DOI](https://zenodo.org/badge/49243972.svg)](https://zenodo.org/badge/latestdoi/49243972)


Equivalent linear site response with random vibration theory, site property
randomization, and a graphical user interface.

## Binaries

Pre-built binaries for Windows are available from the [Github releases
page](https://github.com/arkottke/strata/releases).

## Building

Compiling Strata from the source code requires the following dependencies prior
to building:

-   [Qt](http://doc.qt.io/) (version 5.5 or later)
-   GNU Scientific Library ([GSL](http://www.gnu.org/software/gsl/))
-   [Qwt](http://qwt.sourceforge.net/) (version 6.1 or later)
-   [FFTW](http://www.fftw.org/) (optional)
-   [CMake](https://cmake.org/) (version 3.2 or later)

See [Building on Windows](#building-on-windows) for installing these
dependencies on windows.  Once, these dependencies are installed the Strata can
build checked out and built using the following commands:

    $> git clone https://github.com/arkottke/strata.git
    $> cd strata
    $> mkdir build
    $> cd build
    $> cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:STRING=dist
    $> cmake --build . --target install

Strata executable is located in: `strata/build/dist`. If the build is
unable to find header files and libraries for linking, paths to these files can
be added by providing paths to the GSL and Qwt root directories such as:

    $> cmake .. -DQWT_ROOT_DIR=$QWT_ROOT_DIR -DGSL_ROOT_DIR=$GSL_ROOT_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:STRING=dist

where the variables `$QWT_ROOT_DIR` and `$GSL_ROOT_DIR` correspond to the root
directories of Qwt and GSL, respectively.


### Building on Linux

Depending the distribution, the Qt binaries may or may not be in the package
manager. On Ubuntu Trusty, Qt 5.6 is available from
[ppa:beineri/opt-qt562-trusty] [1], which can be installed with the following
steps:

    $> sudo add-apt-repository --yes ppa:beineri/opt-qt-5.10.1-trusty
    $> sudo apt-get update -qq
    $> sudo apt-get install -qq libgsl0-dev qt510base qt510tools qt510svg cmake

If Qwt 6.1 is not available in the package manager. Qwt can be built using the
following commands:

    $> source /opt/qt510/bin/qt510-env.sh
    $> cd \$HOME/..
    $> svn checkout <svn://svn.code.sf.net/p/qwt/code/branches/qwt-6.1> qwt
    $> cd qwt
    $> qmake
    $> make -j2
    $> sudo make install

### Building on Windows

Building on Windows is greatly simplified by using
[MSYS2](https://msys2.github.io/). After installing MSYS2, the required
dependencies can be installed with the following commands:

    $> pacman -Sy
    $> pacman -S \
        mingw-w64-i686-qt5 \
        mingw-w64-x86_64-qwt-qt5 \
        mingw-w64-x86_64-gsl \
        git

Using a MinGW-w64 shell, execute the commands listed in [Building](#building).

## Testing

Examples for testing are located in the example/ directory.

[1]: https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.10.1-trusty
