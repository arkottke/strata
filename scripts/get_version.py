#!/usr/bin/env python
# encoding: utf-8

from __future__ import print_function

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

if __name__ == '__main__':
    print(get_version())
