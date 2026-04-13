#!/bin/bash
# Update appimage deploy tools
# 

DATE=$(date +%Y%m%d)
ARCH=$(uname -p)

if [[ "$ARCH" == "arm64" ]]; then
	ARCH="aarch64"
elif [[ "$ARCH" == "aarch64" ]]; then
	ARCH="aarch64"
else
	ARCH="x86_64"
fi

mkdir "deploy-bak-$DATE"
sudo mv /usr/local/bin/linuxdeploy "deploy-bak-$DATE/linuxdeploy-$ARCH.AppImage"
sudo mv /usr/local/bin/linuxdeploy-plugin-qt "deploy-bak-$DATE/linuxdeploy-plugin-qt-$ARCH.AppImage"
sudo mv /usr/local/bin/appimagetool "deploy-bak-$DATE/appimagetool-$ARCH.AppImage"

wget "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-$ARCH.AppImage"
wget "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-$ARCH.AppImage"
wget "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-$ARCH.AppImage"
chmod +x "linuxdeploy-$ARCH.AppImage"
chmod +x "linuxdeploy-plugin-qt-$ARCH.AppImage"
chmod +x "appimagetool-$ARCH.AppImage"

sudo mv "linuxdeploy-$ARCH.AppImage" /usr/local/bin/linuxdeploy
sudo mv "linuxdeploy-plugin-qt-$ARCH.AppImage" /usr/local/bin/linuxdeploy-plugin-qt
sudo mv "appimagetool-$ARCH.AppImage" /usr/local/bin/appimagetool

