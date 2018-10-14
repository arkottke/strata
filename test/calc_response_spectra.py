#!/usr/bin/python3

import numpy as np
import pandas as pd

import pyrvt


mag = 7
dist = 50
regions = ['cena', 'wna']


motions = {
    region: pyrvt.motions.SourceTheoryMotion(
        mag, dist, region,
        stress_drop=100 if region == 'wna' else 150,
        peak_calculator='V75')
    for region in regions
}

osc_freqs = np.logspace(-1, 2, num=301)
osc_damping = 0.05

spec_accels = {}
for region, m in motions.items():
    m.calc_fourier_amps()
    spec_accels[region] = m.calc_osc_accels(osc_freqs, osc_damping)

df = pd.DataFrame(spec_accels, index=osc_freqs)
df.to_csv('response_spectra.csv')
