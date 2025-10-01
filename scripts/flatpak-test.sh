#!/bin/bash
# Testing flatpak
# 

# git clone https://github.com/ctlcltd/e2-sat-editor.git
# git checkout $COMMIT
# cd e2-sat-editor

# cd dist/xdg
# flatpak run org.freedesktop.appstream-glib validate io.github.ctlcltd.e2se.metainfo.xml
# desktop-file-validate e2-sat-editor.desktop
# desktop-file-validate e2se-cli.desktop
# cd ../..

# flatpak uninstall io.github.ctlcltd.e2se
# flatpak install org.kde.Sdk
# flatpak install org.kde.Platform

./scripts/translations.sh -m
./scripts/icons.sh -x

mkdir src/build
cat dist/linux/io.github.ctlcltd.e2se.yaml

flatpak-builder src/build dist/linux/io.github.ctlcltd.e2se.yaml --force-clean
flatpak-builder src/build dist/linux/io.github.ctlcltd.e2se.yaml --force-clean --user --install

# flatpak run io.github.ctlcltd.e2se --debug

