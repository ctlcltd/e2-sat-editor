#!/bin/bash
# Testing appimage
# 

# wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
# wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
# chmod +x linuxdeploy*.AppImage
# mv linuxdeploy-x86_64.AppImage /usr/local/bin/linuxdeploy
# mv linuxdeploy-plugin-qt-x86_64.AppImage /usr/local/bin/linuxdeploy-plugin-qt

./scripts/translations.sh -r
./scripts/icons.sh -x

cd src
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DWITHOUT_CLI=ON -G Ninja -B build
ninja -C build
DESTDIR=../AppDir ninja -C build install
mv AppDir/usr/share/applications/e2-sat-editor.desktop AppDir/usr/share/applications/io.github.ctlctld.e2se.desktop
mv AppDir/usr/share/metainfo/io.github.ctlctld.e2se.metainfo.xml AppDir/usr/share/applications/io.github.ctlctld.e2se.appdata.xml
mkdir -p AppDir/usr/share/doc/e2-sat-editor
mv AppDir/usr/share/e2-sat-editor/COPYING AppDir/usr/share/doc/e2-sat-editor/copyright

export QMAKE=qmake6

linuxdeploy --appdir AppDir --plugin qt --output appimage

mv e2\ SAT\ Editor*.AppImage e2-sat-editor-0.7.0-x86_64.AppImage

# ./e2-sat-editor-0.7.0-x86_64.AppImage

