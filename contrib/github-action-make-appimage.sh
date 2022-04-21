#!/bin/bash

source /opt/qt515/bin/qt515-env.sh

# Permissions reset on downloading artifact
chmod +x dist/usr/bin/strata
# Create the AppImage
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage

./linuxdeployqt-continuous-x86_64.AppImage dist/usr/share/applications/strata.desktop -appimage -no-translations

# Rename and upload the AppImage
VERSION=$(sed -ne 's/.*VERSION "\([0-9.]\+\)".*/\1/p' CMakeLists.txt)
GITHASH=$(git rev-parse --short HEAD)
IMAGE=Strata-v$VERSION-$GITHASH-x86_64.AppImage

mv Strata-*.AppImage $IMAGE
