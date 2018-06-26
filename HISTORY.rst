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
