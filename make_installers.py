#!/usr/bin/env python
# encoding: utf-8

import os
import subprocess

archs = ['mingw64', 'mingw32']
instdirs = {
    'mingw64': r'C:\Program Files\Strata',
    'mingw32': r'C:\Program Files (x86)\Strata'
}

orig_path = str(os.environ['PATH'])
#arch = os.environ['MSYSTEM'].lower()

for arch in archs:
    os.environ['PATH'] = orig_path + ';C:/msys64/usr/bin;C:/msys64/%s/bin' % arch
    if arch == 'mingw64':
        os.environ['PATH'] += ';C:/msys64/mingw64/lib/gcc/x86_64-w64-mingw32/5.3.0'
    else:
        os.environ['PATH'] += ';C:/msys64/mingw32/lib/gcc/i686-w64-mingw32/5.3.0'

    dirname = 'build_' + arch

    if not os.path.exists(dirname):
        os.makedirs(dirname)

    os.chdir(dirname)
    subprocess.call(['C:/msys64/%s/bin/qmake.exe' % arch, '..'])
    subprocess.call(['make.exe', 'release'])
    os.chdir('..')
    subprocess.call([r'C:\Program Files (x86)\NSIS\makensis.exe',
                     '/DARCH=' + arch,
                     '/DINSTDIR="' + instdirs[arch] + '"',
                     r'installer.nsi'])
