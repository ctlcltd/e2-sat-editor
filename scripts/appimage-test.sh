#!/bin/bash
# Testing appimage
# 

## Ubuntu 18.04 LTS
## GCC g++ 7.5.0 (GCC 9 needed)
##
##
##
## wget https://releases.ubuntu.com/18.04.6/ubuntu-18.04.6-live-server-amd64.iso
##

## Ubuntu 20.04 LTS
## GCC g++ 9.4.0 (GCC 9 satisfied)
##
##

# wget https://releases.ubuntu.com/20.04.6/ubuntu-20.04.6-live-server-amd64.iso

# dpkg-reconfigure tzdata
# apt-get update
# apt-get upgrade

# apt-get install build-essential cmake ninja-build python3 wget
# cd $HOME
# wget https://github.com/Kitware/CMake/releases/download/v3.26.4/cmake-3.26.4-linux-x86_64.sh
# wget https://master.qt.io/archive/qt/6.5/6.5.0/single/qt-everywhere-src-6.5.0.tar.xz
# wget https://curl.se/download/curl-8.1.0.tar.xz
# chmod +x cmake-3.26.4-linux-x86_64.sh
# ./cmake-3.26.4-linux-x86_64.sh --prefix=/usr/local
# tar -xf qt-everywhere-src-6.5.0.tar.xz
# tar -xf curl-8.1.0.tar.xz
# cd $HOME/qt-everywhere-src-6.5.0
# apt-get install libgl1-mesa-dev libglvnd-dev libdrm-dev libfontconfig-dev libfreetype-dev libharfbuzz-dev libgbm-dev libvulkan-dev libwayland-dev libgles-dev libglu1-mesa-dev libgl-dev libinput-dev libxcb-xinput-dev libxcb-xinerama0-dev libxcb-randr0-dev libxcb-shape0-dev libxcb-sync-dev libxcb-util-dev libxcb-xfixes0-dev lib-xext-dev libxcb-xkb-dev libxkbcommon-x11-dev libx11-xcb-dev libxcb-cursor-dev libxcb-keysyms1-dev libxcb-icccm4-dev libxcb-glx0-dev
# apt-get install libatspi2.0-dev libglib2.0-dev libicu-dev libdouble-conversion-dev libgtk-3-dev libjpeg-dev libpng-dev libmd4c-dev libpcre2-dev libts-dev libudev-dev libssl-dev libpq-dev libproxy-dev libsctp-dev libsystemd-dev libb2-dev libdbus-1-dev libcups2-dev dh-exec
# apt-get install libkrb5-dev liblttng-ust-dev default-libmysqlclient-dev firebird-dev libsqlite3-dev unixodbc-dev
# apt-get install libzstd-dev zlib1g-dev libmtdev-dev
# df -h
# cmake -DCMAKE_BUILD_TYPE=None -G Ninja . -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF -DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=ON -DCMAKE_VERBOSE_MAKEFILE=ON -DBUILD_EXAMPLES=OFF -DFEATURE_mimetype_database=OFF -DFEATURE_dbus_linked=ON -DFEATURE_accessibility=ON -DFEATURE_doubleconversion=ON -DFEATURE_glib=ON -DFEATURE_icu=ON -DFEATURE_pcre2=ON -DFEATURE_system_pcre2=ON -DFEATURE_zlib=ON -DFEATURE_ssl=ON -DFEATURE_libproxy=ON -DFEATURE_system_proxies=ON -DFEATURE_cups=ON -DFEATURE_fontconfig=ON -DFEATURE_freetype=ON -DFEATURE_harfbuzz=ON -DFEATURE_xcb=ON -DFEATURE_system_xcb_xinput=ON -DFEATURE_gtk=ON -DFEATURE_directfb=OFF -DFEATURE_sql_odbc=ON -DFEATURE_sql_mysql=ON -DFEATURE_sql_psql=ON -DFEATURE_sql_sqlite=ON -DFEATURE_system_sqlite=ON -DFEATURE_jpeg=ON -DFEATURE_system_jpeg=ON -DFEATURE_png=ON -DFEATURE_system_png=ON -DFEATURE_system_libb2=ON -DFEATURE_rpath=OFF -DFEATURE_relocatable=OFF -DFEATURE_sql_ibase=ON -DFEATURE_sctp=ON
# cat config.summary
# cmake --build . --parallel -t qtbase -t qtimageformats -t qtsvg -t qttools -t qtwayland
# cd qtbase
# cmake --install .
# cd ../qttools
# cmake --install .
# cd .../qtwayland
# cmake --install .
# cd
# export LD_LIBRARY_PATH=/usr/local/Qt-6.5.0/lib:$LD_LIBRARY_PATH
# export PATH=/usr/local/Qt-6.5.0/bin:$PATH
# qmake --version
# cd $HOME/curl-8.1.0
# apt-get install libssl-dev
# df -h
# ./configure --prefix=/usr/local --with-openssl --enable-versioned-symbols
# make -j 2
# make install
# ldconfig
# curl --version

# apt-get install build-essential cmake ninja-build git
# apt-get install libgl1-mesa-dev libglvnd-dev libxcb-xkb-dev libxkbcommon-x11-dev libvulkan-dev
# export LD_LIBRARY_PATH=/usr/local/Qt-6.5.0/lib:$LD_LIBRARY_PATH
# export PATH=/usr/local/Qt-6.5.0/bin:$PATH
# ldconfig
# cd $HOME


# wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
# wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
# wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
# chmod +x linuxdeploy*.AppImage
# chmod +x appimagetool*.AppImage
# mv linuxdeploy-x86_64.AppImage /usr/local/bin/linuxdeploy
# mv linuxdeploy-plugin-qt-x86_64.AppImage /usr/local/bin/linuxdeploy-plugin-qt
# mv appimagetool-x86_64.AppImage /usr/local/bin/appimagetool

# git clone https://github.com/ctlcltd/e2-sat-editor.git
# git checkout v0.7.0
# cd e2-sat-editor

./scripts/translations.sh -m
./scripts/icons.sh -x

cd src
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DWITHOUT_CLI=ON -G Ninja -B build
ninja -C build
DESTDIR=../AppDir ninja -C build install
mv AppDir/usr/share/applications/e2-sat-editor.desktop AppDir/usr/share/applications/io.github.ctlcltd.e2se.desktop
mv AppDir/usr/share/metainfo/io.github.ctlcltd.e2se.metainfo.xml AppDir/usr/share/metainfo/io.github.ctlcltd.e2se.appdata.xml
mkdir -p AppDir/usr/share/doc/e2-sat-editor
mv AppDir/usr/share/e2-sat-editor/COPYING AppDir/usr/share/doc/e2-sat-editor/copyright

# export QMAKE=qmake
linuxdeploy --appdir AppDir --plugin qt --output appimage

chmod +x e2_SAT_Editor*.AppImage
mv e2_SAT_Editor*.AppImage e2-sat-editor-0.7.0-x86_64.AppImage

# ./e2-sat-editor-0.7.0-x86_64.AppImage


# apt-get install nano
# nano /etc/resolv.conf
# systemctl restart systemd-resolved

appimagetool AppDir --sign --sign-key $MY_SIGN_KEY

# ./e2-sat-editor-0.7.0-x86_64.AppImage --appimage-signature

