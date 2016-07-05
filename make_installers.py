#!/usr/bin/env python
# encoding: utf-8

import os
import subprocess
import re


def get_version():
    with open('source/main.cpp') as fp:
        for l in fp:
            m = re.search(r'setApplicationVersion\("(\S+)"\)', l)
            if m:
                return m.group(1)

archs = ['mingw64', 'mingw32']
instdirs = {
    'mingw64': r'C:\Program Files\Strata',
    'mingw32': r'C:\Program Files (x86)\Strata'
}

orig_path = str(os.environ['PATH'])
version = get_version()

for arch in archs:
    os.environ['PATH'] = orig_path
    os.environ['PATH'] += ';C:/msys64/usr/bin;C:/msys64/%s/bin' % arch
    if arch == 'mingw64':
        os.environ['PATH'] += \
            ';C:/msys64/mingw64/lib/gcc/x86_64-w64-mingw32/5.3.0'
    else:
        os.environ['PATH'] += \
            ';C:/msys64/mingw32/lib/gcc/i686-w64-mingw32/5.3.0'

    dirname = 'release_' + arch
    if not os.path.exists(dirname):
        os.makedirs(dirname)

    os.chdir(dirname)
    # subprocess.call(['C:/msys64/%s/bin/cmake.exe' % arch,
    #                  'Release', '-G', 'MSYS Makefiles', '..'])
    # subprocess.call(['make.exe', '-j', '4'])

    subprocess.call(['qmake', '../strata.pro'])
    subprocess.call(['make.exe', '-j', '4', 'release'])

    os.chdir('..')
    subprocess.call([r'C:\Program Files (x86)\NSIS\makensis.exe',
                     '/DVERSION=' + version,
                     '/DARCH=' + arch,
                     '/DINSTDIR="' + instdirs[arch] + '"',
                     r'installer.nsi'])
