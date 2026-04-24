# Strata

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Build Status](https://travis-ci.org/arkottke/strata.svg?branch=master)](https://travis-ci.org/arkottke/strata)
[![Build status](https://ci.appveyor.com/api/projects/status/cpgr2vsh1re8c35x/branch/master?svg=true)](https://ci.appveyor.com/project/arkottke/strata/branch/master)
[![DOI](https://zenodo.org/badge/49243972.svg)](https://zenodo.org/badge/latestdoi/49243972)


Equivalent linear site response with random vibration theory, site property
randomization, and a graphical user interface.

The _Strata_ GUI simplifies the process for performing site response analysis, but
limits the flexibility and requires extensive coding to add features. To help
address these shortcomings, I have written
[pyStrata](https://github.com/arkottke/pystrata), which is a Python library for
performing site response. With _pyStrata_, complicated customized workflows can be
developed, or examples using Jupyter notebooks can be used to aid in
understanding of the process.

## Binaries

Pre-built binaries for Windows are available from the [Github releases
page](https://github.com/arkottke/strata/releases).

## Citation

When citing Strata, refer to either the technical manual:

> Kottke, Albert R., and Ellen M. Rathje. (2008). "Technical manual for
> Strata." Report No.: 2008/10. Pacific Earthquake Engineering Research Center,
> University of California, Berkeley.

or the [DOI](https://zenodo.org/badge/latestdoi/49243972) of the release and
this website.

## Building

Strata uses a modernized build system based on **CMake Presets** and **vcpkg** for dependency management. This ensures a consistent, reproducible build environment across Windows, macOS, and Linux.

### Prerequisites
- [CMake](https://cmake.org) (version 3.21 or later)
- A C++17 compatible compiler (MSVC 2022, GCC 11+, or Clang 13+)
- [Ninja](https://ninja-build.org/) (recommended and used by default in presets)

### Build Workflow

The following steps will automatically fetch and build all dependencies (GSL, Qwt, and Qt6) using the included `vcpkg` submodule.

1.  **Clone the repository with submodules:**
    ```bash
    git clone --recursive https://github.com/arkottke/strata.git
    cd strata
    ```

2.  **Bootstrap vcpkg (one-time setup):**
    ```bash
    # Linux/macOS
    ./vcpkg/bootstrap-vcpkg.sh
    # Windows
    .\vcpkg\bootstrap-vcpkg.bat
    ```

3.  **Configure and Build:**
    Choose the preset corresponding to your OS: `linux-release`, `macos-release`, or `windows-release`.
    ```bash
    cmake --preset <preset-name>
    cmake --build --preset <preset-name>
    ```

4.  **Create Installer (Optional):**
    To generate a platform-native installer (NSIS on Windows, DMG on macOS, or Tarball on Linux), run:
    ```bash
    cmake --build --preset <preset-name> --target package
    ```

The compiled executable will be located in `build/<preset-name>/source/strata`.

## Testing

Examples for testing are located in the `example/` directory. Regression tests can be run via CMake:

```bash
cd build/<preset-name>
ctest
```
