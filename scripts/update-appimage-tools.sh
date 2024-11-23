#!/bin/bash
# Update appimage deploy tools
# 

D=$(date +%Y%m%d)

mkdir "deploy-bak-${D}"
sudo mv /usr/local/bin/linuxdeploy "deploy-bak-${D}/linuxdeploy-x86_64.AppImage"
sudo mv /usr/local/bin/linuxdeploy-plugin-qt "deploy-bak-${D}/linuxdeploy-plugin-qt-x86_64.AppImage"
sudo mv /usr/local/bin/appimagetool "deploy-bak-${D}/appimagetool-x86_64.AppImage"

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage

sudo mv linuxdeploy-x86_64.AppImage /usr/local/bin/linuxdeploy
sudo mv linuxdeploy-plugin-qt-x86_64.AppImage /usr/local/bin/linuxdeploy-plugin-qt
sudo mv appimagetool-x86_64.AppImage /usr/local/bin/appimagetool

