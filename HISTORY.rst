v0.7.0 (2019-10-16)
-------------------
- Added: Instructions for command line operation to manual
- Improved: plots of the CompatibleRvtMotion dialog
- Improved: log messages
- Fixed: Crash on clearing of output
- Fixed: Issue with inconsistent time series length
- Fixed: Logic of smooth spectrum in FrequencyDependentCalculator

v0.6.4 (2019-08-06)
-------------------
- Fixed manual included in installers

v0.6.3 (2018-11-01)
-------------------
- Fixed initialize of variables in SourceTheoryRvtMotion
- Fixed bug in ConfigurePlotDialog

v0.6.2 (2018-10-26)
-------------------
- Fixed #11: Bug in auto-discretization.
- Re-created examples files.

v0.6.1 (2018-10-20)
-------------------
- Fixed bug in WT18 peak factor calculation

v0.6.0 (2018-10-20)
-------------------
- Change peak factor model to Wang & Rathje (2018), which based on Vanmarcke
  (1976) with Boore and Thompson (2015) RMS duration modification
- Changed to CMake build system
- Unified GUI for RVT input motion specification

v0.5.10 (2018-06-25)
--------------------
- Changed from cubic spline to log-linear interpolation of nonlinear curves.

v0.5.9 (2017-12-02)
-------------------
- Fixed name of the PCRE DLL.

v0.5.8 (2017-10-17)
-------------------
- Fixed min/max of nonlinear curves.

v0.5.7 (2017-08-18)
-------------------
- Added: Support for alternative PEER header formats

v0.5.6
------
- Added: Interpolation to CompatibleRvtMotion target if specified with fewer
  than 10 points.
- CI: limit Appveyor to build only master

v0.5.5
------
- Fixed: allow reading integers in time series
- Fixed: Added organization (ARKottke) to fix QSettings usage
- Fixed: Removed incomplete example
- Changed: Copy Qt DLLs to archive using windeployqt 

v0.5.4
------
- Fixed: Reading of batch files
