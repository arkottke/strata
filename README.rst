======
Strata
======

.. image:: https://img.shields.io/travis/arkottke/pygmm.svg
    :target: https://travis-ci.org/arkottke/pygmm
    :alt: Build Status

.. image::  https://img.shields.io/badge/license-GLPv3-blue.svg
    :target: https://github.com/arkottke/pygmm/blob/master/LICENSE.txt
    :alt: License


Equivalent linear site response with random vibration theory, site property
randomization, and a graphical user interface.



Building
--------

Strata requires the following dependencies prior to building:

* `Qt <http://doc.qt.io/>`_ (version 5.4 or later)
* GNU Scientific Library (`GSL <http://www.gnu.org/software/gsl/>`_)
* `Qwt <http://qwt.sourceforge.net/>`_ (version 6.1 or later)
* `FFTW <http://www.fftw.org/>`_ (optional)
* `cmake <https://cmake.org/>`_

See `Building on Windows`_ for installing these dependencies on windows. Once,
these dependencies are installed the *Strata* can build checked out and built
using the following commands::

    $> git clone https://github.com/arkottke/strata.git strata
    $> cd strata
    $> mkdir build
    $> cd build
    $> cmake ..
    $> make

The resulting executable is `strata/build/strata`.

Calling *cmake* may require specification of the generator name. On Windows with
MSYS the *cmake* call would be::
    
    $> cmake -G "MSYS Makefiles" ..


Building on Windows
...................

Building on Windows is greatly simplified by using 
`MSYS2 <https://msys2.github.io/>`_. After installing *MSYS2*, the required
dependencies can be installed with the following commands::
    
    $> pacman -Sy
    $> pacman -S \
        mingw-w64-i686-qt5 \
        mingw-w64-x86_64-qwt-qt5 \
        mingw-w64-x86_64-cmake \ 
        mingw-w64-x86_64-gsl \
        git


Testing
-------

Examples for testing are located in the `example/` directory.
