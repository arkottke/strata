#!/bin/sh

cd build

if [ "$TRAVIS_OS_NAME" == "linux" ]; then 
    # Create the AppImage
    wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
    chmod a+x linuxdeployqt-continuous-x86_64.AppImage
    ARCH=x86_64 ./linuxdeployqt-continuous-x86_64.AppImage dist/strata.desktop -appimage -no-translations
    # Rename and upload the AppImage
    VERSION=$(sed -ne 's/.*VERSION "\([0-9.]\+\)".*/\1/p' ../CMakeLists.txt)
    GITHASH=$(git rev-parse --short HEAD)
    IMAGE=Strata-v$VERSION-$GITHASH-x86_64.AppImage
    mv Strata-x86_64.AppImage $IMAGE
    curl --upload-file $IMAGE https://transfer.sh/$IMAGE
else
    ls
fi

# Return the build dir
cd $TRAVIS_BUILD_DIR
