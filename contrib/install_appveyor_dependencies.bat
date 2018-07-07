@ECHO OFF

REM Set up the CPP environment
REM https://www.appveyor.com/docs/lang/cpp/
IF %PLATFORM%==x86 (
    ECHO "Using x86 Environment"
    CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
) ELSE (
    ECHO "Using x64 Environment"
    CALL "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x64 
    CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
)

SET PATH=%QT5%\bin;C:\Program Files (x86)\WiX Toolset v3.11\bin;%PATH%
CD c:\projects

REM Install GSL
IF EXIST gsl (
    CD gsl
    git pull
) ELSE (
    git clone https://github.com/ampl/gsl.git
    CD gsl
)

cmake ^
 -DGSL_DISABLE_WARNINGS:BOOL=ON^
 -DBUILD_SHARED_LIBS:BOOL=ON^
 -DMSVC_RUNTIME_DYNAMIC:BOOL=ON^
 -DCMAKE_BUILD_TYPE="Release"^
 -G "%CMAKE_GENERATOR%"^
 .
cmake --build . --target install

IF %PLATFORM%==x86 (
    SET GSL_ROOT_DIR="%ProgramFiles(x86)%\GSL"
) ELSE (
    SET GSL_ROOT_DIR="%ProgramFiles%\GSL"
)

REM Install Qwt
CD ..
IF EXIST qwt (
    CD qwt
    svn update
) ELSE (
    svn co -r HEAD svn://svn.code.sf.net/p/qwt/code/branches/qwt-6.1 qwt 
    CD qwt
)

qmake
nmake 
set QWT_ROOT_DIR=%CD%
