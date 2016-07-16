======
Strata
======

.. image:: https://img.shields.io/travis/arkottke/strata.svg
    :target: https://travis-ci.org/arkottke/strata
    :alt: Build Status

.. image::  https://img.shields.io/badge/license-GLPv3-blue.svg
    :target: https://github.com/arkottke/strata/blob/master/LICENSE.txt
    :alt: License


Equivalent linear site response with random vibration theory, site property
randomization, and a graphical user interface.



Building
========

Strata requires the following dependencies prior to building:

* `Qt <http://doc.qt.io/>`_ (version 5.5 or later)
* GNU Scientific Library (`GSL <http://www.gnu.org/software/gsl/>`_)
* `Qwt <http://qwt.sourceforge.net/>`_ (version 6.1 or later)
* `FFTW <http://www.fftw.org/>`_ (optional)

See `Building on Windows`_ for installing these dependencies on windows. Once,
these dependencies are installed the *Strata* can build checked out and built
using the following commands::
    $> git clone https://github.com/arkottke/strata.git
    $> cd strata
    $> mkdir build
    $> cd build
    $> qmake ..
    $> make -j2 release

Strata executable is located in: `strata/build/release/strata`. If the build is
unable to find header files and libraries for linking, paths to these files can
be added by modifying the `strata.pro` text file, or by passing the INCLUDEPATH
and LIBS environmental varibles::
    

Building on Linux
-----------------

Depending the distribution, the Qt binaries may or may not be in the package
manager. On Ubuntu Trusty, Qt 5.6 is available from
`ppa:beineri/opt-qt561-trusty`_, which can be installed with the following
steps::
    $> sudo add-apt-repository --yes ppa:beineri/opt-qt561-trusty
    $> sudo apt-get update -qq
    $> sudo apt-get install -qq libgsl0-dev qt56base qt56tools qt56svg qt56script

If Qwt 6.1 is not available in the package manager. Qwt can be built using the
following commands::
    $> source /opt/qt56/bin/qt56-env.sh
    $> cd $HOME/..
    $> svn checkout svn://svn.code.sf.net/p/qwt/code/branches/qwt-6.1 qwt
    $> cd qwt
    $> qmake
    $> make -j2
    $> sudo make install

Here is an example of passing adding paths for Qwt headers and shared
libraries::
    $> LIBS=-L/usr/qwt-6.1.3/lib
    $> INCLUDEPATH=/usr/local/qwt-6.1.3/include
    $> qmake ..

Building on Windows
-------------------

Building on Windows is greatly simplified by using 
`MSYS2 <https://msys2.github.io/>`_. After installing *MSYS2*, the required
dependencies can be installed with the following commands::
    
    $> pacman -Sy
    $> pacman -S \
        mingw-w64-i686-qt5 \
        mingw-w64-x86_64-qwt-qt5 \
        mingw-w64-x86_64-gsl \
        git

Using a *MinGW-w64* shell, execute the commands listed in `Building`_. 
    

Testing
=======

Examples for testing are located in the `example/` directory.
