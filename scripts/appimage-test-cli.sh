#!/bin/bash
# Testing appimage e2se-cli
# 

## Ubuntu 24.04.4 LTS
## https://releases.ubuntu.com/noble/ubuntu-24.04.4-live-server-amd64.iso
## 
##

ARCH=$(uname -p)

if [[ "$ARCH" == "arm64" ]]; then
	ARCH="aarch64"
elif [[ "$ARCH" == "aarch64" ]]; then
	ARCH="aarch64"
else
	ARCH="x86_64"
fi

# dpkg-reconfigure tzdata
# apt update
# apt upgrade

# apt install build-essential
# apt install cmake ninja-build
# apt-get install wget
# cmake --version
# ninja --version
# ldd --version


# apt install fuse
# wget "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-$ARCH.AppImage"
# wget "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-$ARCH.AppImage"
# chmod +x "linuxdeploy-$ARCH.AppImage"
# chmod +x "appimagetool-$ARCH.AppImage"
# mv "linuxdeploy-$ARCH.AppImage" /usr/local/bin/linuxdeploy
# mv "appimagetool-$ARCH.AppImage" /usr/local/bin/appimagetool


# cd
# git clone https://github.com/ctlcltd/e2-sat-editor.git
# git checkout $TAG
# cd e2-sat-editor

cd src/cli
rm -R build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -G Ninja -B build
ninja -C build
DESTDIR=../AppDir ninja -C build install


mkdir -p AppDir/usr/share/applications
cp ../../dist/xdg/e2se-cli.desktop AppDir/usr/share/applications/io.github.ctlcltd.e2se-cli.desktop
cp ../../dist/xdg/io.github.ctlcltd.e2se-cli.metainfo.xml AppDir/usr/share/metainfo/io.github.ctlcltd.e2se-cli.appdata.xml
mkdir -p AppDir/usr/share/doc/e2se-cli
mv AppDir/usr/share/e2se-cli/Readme.txt AppDir/usr/share/doc/e2se-cli/readme
mv AppDir/usr/share/e2se-cli/License.txt AppDir/usr/share/doc/e2se-cli/copyright

linuxdeploy --appdir AppDir --output appimage

chmod +x e2se-cli-$ARCH.AppImage

# ./e2se-cli-$ARCH.AppImage


# apt-get install nano
# nano /etc/resolv.conf
# systemctl restart systemd-resolved

./e2se-cli-$ARCH.AppImage --appimage-extract

appimagetool squashfs-root --sign --sign-key $MY_SIGN_KEY

mv "e2se-cli-$ARCH.AppImage e2se-cli-$VERSION-$ARCH.AppImage

# ./e2se-cli-$VERSION-$ARCH.AppImage --appimage-signature

