########################################################################
# Strata 
# Copyright (C) 2011   Albert R. Kottke
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GPL License, Version 3.0
########################################################################

########################################################################
# Enable advanced options that should not be included in the version for
# standard users. If only the basic features are needed, comment out the
# following line.
########################################################################
DEFINES += ADVANCED_OPTIONS

########################################################################
# Build type. For most cases this should be release, however during
# development of the software using the debug configuration can be
# beneficial.
########################################################################
#CONFIG += debug
#CONFIG += release
CONFIG += debug_and_release

########################################################################
# Compiler warning messages.
########################################################################
CONFIG += warn_on

########################################################################
# Enable console for debug versions
########################################################################
CONFIG(debug, debug|release) {
    CONFIG += console
}

########################################################################
# Setup of for the various libraries. This is most important on Windows.
########################################################################
unix {
    LIBS += -lm \
        -lfftw3 \
        -lgsl \
        -lgslcblas \
        -L$$PWD/../../qwt-6.1/src
        -lqwt
    INCLUDEPATH += $$PWD/../../qwt-6.1/src

}
win32 { 
    LIBS += -lm \
        -lfftw3-3 \
        -L"C:/devel/fftw-3.3.4" \
        -lgsl \
        -lgslcblas \
        -L"C:/devel/GnuWin32/bin"
    INCLUDEPATH += . \
        "C:/devel/fftw-3.3.4" \
        "C:/devel/qwt-6.1/src" \
        "C:/devel/GnuWin32/include"
    RC_FILE = strata.rc
    CONFIG(debug, debug|release ) {
        LIBS += -lqwtd \
            -L"C:/devel/qwt-6.1/lib"
    } else {
        LIBS += -lqwt \
            -L"C:/devel/qwt-6.1/lib"
    }
}
