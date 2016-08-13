#!/usr/bin/env python
# encoding: utf-8

import os
import subprocess
import re


def get_version():
    parts = {}
    with open('strata.pro') as fp:
        for l in fp:
            m = re.search(r'VER_(?P<part>\S+) = (?P<number>\d+)', l)
            if m:
                gd = m.groupdict()
                parts[gd['part']] = gd['number']

    # Need to use decode() to convert from bytes
    parts['GIT'] = subprocess.check_output(
        ['git', 'rev-parse', '--short', 'HEAD']).strip().decode()
    if parts:
        ver = '{MAJ}.{MIN}.{PAT}-{GIT}'.format(**parts)
    else:
        raise RuntimeError("Unable to parse strata.pro")
    return ver

archs = ['mingw64', 'mingw32']
instdirs = {
    'mingw64': r'C:\Program Files\Strata',
    'mingw32': r'C:\Program Files (x86)\Strata'
}

orig_path = str(os.environ['PATH'])

for arch in archs:
    os.environ['PATH'] = orig_path
    os.environ['PATH'] += ';C:/msys64/usr/bin;C:/msys64/%s/bin' % arch
    if arch == 'mingw64':
        os.environ['PATH'] += \
            ';C:/msys64/mingw64/lib/gcc/x86_64-w64-mingw32/6.1.0'
    else:
        os.environ['PATH'] += \
            ';C:/msys64/mingw32/lib/gcc/i686-w64-mingw32/6.1.0'

    os.environ['LD_LIBRARY_PATH'] = '/{}/lib'.format(arch)
    os.environ['LIBRARY_PATH'] = '/{}/lib'.format(arch)
    os.environ['CPLUS_INCLUDE_PATH'] = '/{}/include/qwt'.format(arch)

    dirname = 'release_' + arch
    if not os.path.exists(dirname):
        os.makedirs(dirname)

    os.chdir(dirname)
    subprocess.call(['qmake', '../strata.pro'])
    subprocess.call(['make.exe', '-j', '4', 'release'])

    os.chdir('..')
    version = get_version()
    subprocess.call([r'C:\Program Files (x86)\NSIS\makensis.exe',
                     '/DVERSION=' + version,
                     '/DARCH=' + arch,
                     '/DINSTDIR="' + instdirs[arch] + '"',
                     r'installer.nsi'])

    break
