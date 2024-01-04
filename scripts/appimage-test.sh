#!/bin/bash
# Testing appimage
# 

## Ubuntu 22.04 LTS
## https://releases.ubuntu.com/jammy/ubuntu-22.04.3-live-server-amd64.iso
## 
##

# dpkg-reconfigure tzdata
# apt update
# apt upgrade

# apt install build-essential
# apt install cmake ninja-build
# cmake --version
# ninja --version
# ldd --version


# apt install curl libcurl4-openssl-dev
# apt install qt6-base-dev qt6-translations-l10n qt6-l10n-tools
# curl --version
# qmake6 --version


# apt install wget
# cd
# apt install libssl-dev
# wget https://curl.se/download/curl-8.1.2.tar.xz
# tar -xf curl-8.1.2.tar.xz
# cd curl-8.1.2
# ./configure --prefix=/usr/local --with-openssl --enable-versioned-symbols
# make -j 2
# make install
# ldconfig
# curl --version
# cd
# apt install libgl1-mesa-dev libglvnd-dev libdrm-dev libfontconfig-dev libfreetype-dev libharfbuzz-dev libgbm-dev libvulkan-dev libwayland-dev libgles-dev libglu1-mesa-dev libgl-dev libinput-dev libxcb-xinput-dev libxcb-xinerama0-dev libxcb-randr0-dev libxcb-shape0-dev libxcb-sync-dev libxcb-util-dev libxcb-xfixes0-dev libxext-dev libxcb-xkb-dev libxkbcommon-x11-dev libx11-xcb-dev libxcb-cursor-dev libxcb-keysyms1-dev libxcb-icccm4-dev libxcb-glx0-dev
# apt install libatspi2.0-dev libglib2.0-dev libicu-dev libdouble-conversion-dev libgtk-3-dev libjpeg-dev libpng-dev libmd4c-dev libmd4c-html0-dev libpcre2-dev libts-dev libudev-dev libssl-dev libpq-dev libproxy-dev libsctp-dev libsystemd-dev libb2-dev libdbus-1-dev libcups2-dev dh-exec
# apt install libkrb5-dev liblttng-ust-dev default-libmysqlclient-dev firebird-dev libsqlite3-dev unixodbc-dev
# apt install libzstd-dev zlib1g-dev libmtdev-dev
# apt install llvm clang clang-tools libclang-dev
# df -h
# wget https://master.qt.io/archive/qt/6.6/6.6.1/single/qt-everywhere-src-6.6.1.tar.xz
# tar -xf qt-everywhere-src-6.6.1.tar.xz
# cd qt-everywhere-src-6.6.1
# cmake -DCMAKE_BUILD_TYPE=None -DCMAKE_INSTALL_PREFIX=/usr/local -G Ninja . -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF -DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=ON -DCMAKE_VERBOSE_MAKEFILE=ON -DBUILD_EXAMPLES=OFF -DFEATURE_mimetype_database=OFF -DFEATURE_dbus_linked=ON -DFEATURE_accessibility=ON -DFEATURE_doubleconversion=ON -DFEATURE_glib=ON -DFEATURE_icu=ON -DFEATURE_pcre2=ON -DFEATURE_system_pcre2=ON -DFEATURE_zlib=ON -DFEATURE_ssl=ON -DFEATURE_libproxy=ON -DFEATURE_system_proxies=ON -DFEATURE_cups=ON -DFEATURE_fontconfig=ON -DFEATURE_freetype=ON -DFEATURE_harfbuzz=ON -DFEATURE_xcb=ON -DFEATURE_system_xcb_xinput=ON -DFEATURE_gtk=ON -DFEATURE_directfb=OFF -DFEATURE_sql_odbc=ON -DFEATURE_sql_mysql=ON -DFEATURE_sql_psql=ON -DFEATURE_sql_sqlite=ON -DFEATURE_system_sqlite=ON -DFEATURE_jpeg=ON -DFEATURE_system_jpeg=ON -DFEATURE_png=ON -DFEATURE_system_png=ON -DFEATURE_system_libb2=ON -DFEATURE_rpath=OFF -DFEATURE_relocatable=OFF -DFEATURE_sql_ibase=ON -DFEATURE_sctp=ON
# cat config.summary | less
# cmake --build . --parallel -t qtbase -t qtimageformats -t qtsvg -t qttools -t qtwayland
# cd qtbase
# cmake --install .
# cd qtimageformats
# cmake --install .
# cd qtsvg
# cmake --install .
# cd ../qttools
# cmake --install .
# cd ../qtwayland
# cmake --install .
# ldconfig
# cd
# qmake --version

# ls /usr/local/translations
# cd
# cd qt-everywhere-src-6.6.1
# cd ../qttranslations/translations
# lrelease
# for tsfile in qt_*.ts; do lrelease $tsfile -qm "${%tsfile.ts}.qm"; done
# for tsfile in qtbase_*.ts; do lrelease $tsfile -qm "${%tsfile.ts}.qm"; done
# mkdir -p /usr/local/translations
# cp -R qt_*.qm /usr/local/translations
# cp -R qtbase_*.qm /usr/local/translations
# rm -R /usr/local/translations/qt_help_*.qm


# apt install fuse wget
# wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
# wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
# wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
# chmod +x linuxdeploy-x86_64.AppImage
# chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
# chmod +x appimagetool-x86_64.AppImage
# mv linuxdeploy-x86_64.AppImage /usr/local/bin/linuxdeploy
# mv linuxdeploy-plugin-qt-x86_64.AppImage /usr/local/bin/linuxdeploy-plugin-qt
# mv appimagetool-x86_64.AppImage /usr/local/bin/appimagetool


# apt install build-essential git
# apt install libgl1-mesa-dev libglvnd-dev libxcb-xkb-dev libxkbcommon-x11-dev libvulkan-dev

# cd
# git clone https://github.com/ctlcltd/e2-sat-editor.git
# git checkout v1.0.0
# cd e2-sat-editor

./scripts/translations.sh -m
./scripts/icons.sh -x

cd src
rm -R build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -G Ninja -B build
ninja -C build
DESTDIR=../AppDir ninja -C build install


# mkdir -p AppDir/usr/share/qt6/translations
# cp /usr/share/qt6/translations/qt_*.qm AppDir/usr/share/qt6/translations
# cp /usr/share/qt6/translations/qtbase_*.qm AppDir/usr/share/qt6/translations


mkdir -p AppDir/usr/share/qt/translations
cp /usr/local/translations/qt_*.qm AppDir/usr/share/qt/translations
cp /usr/local/translations/qtbase_*.qm AppDir/usr/share/qt/translations


rm AppDir/usr/share/applications/e2se-cli.desktop
mv AppDir/usr/share/applications/e2-sat-editor.desktop AppDir/usr/share/applications/io.github.ctlcltd.e2se.desktop
mv AppDir/usr/share/metainfo/io.github.ctlcltd.e2se.metainfo.xml AppDir/usr/share/metainfo/io.github.ctlcltd.e2se.appdata.xml
mkdir -p AppDir/usr/share/doc/e2-sat-editor
mv AppDir/usr/share/e2-sat-editor/COPYING AppDir/usr/share/doc/e2-sat-editor/copyright

# export QMAKE=qmake6
linuxdeploy --appdir AppDir --plugin qt --output appimage

chmod +x e2_SAT_Editor-x86_64.AppImage

# ./e2_SAT_Editor-x86_64.AppImage


# apt-get install nano
# nano /etc/resolv.conf
# systemctl restart systemd-resolved

./e2_SAT_Editor-x86_64.AppImage --appimage-extract

appimagetool squashfs-root --sign --sign-key $MY_SIGN_KEY

mv e2_SAT_Editor-x86_64.AppImage e2-sat-editor-1.0.0-x86_64.AppImage

# ./e2-sat-editor-1.0.0-x86_64.AppImage --appimage-signature

