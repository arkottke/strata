"""Convert BT pars files into JSON format."""

import json
import pathlib


def load_pars(fpath):
    with fpath.open() as fp:
        for _ in range(3):
            next(fp)

        names = next(fp).split()
        rows = [[float(part) for part in line.split()] for line in fp]

        rows = [row for row in rows if row]

        d = {n: [row[i] for row in rows] for i, n in enumerate(names)}
        return d


for fpath in pathlib.Path('../resources/data').glob('*.pars'):
    pars = load_pars(fpath)
    with fpath.with_suffix('.json').open('wt') as fp:
        json.dump(pars, fp, indent=4)
