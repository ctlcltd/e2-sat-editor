#!/bin/bash
# Testing flatpak
# 

# cd dist/xdg
# flatpak run org.freedesktop.appstream-glib validate io.github.ctlcltd.e2se.xml
# desktop-file-validate e2-sat-editor.desktop
# desktop-file-validate e2se-cli.desktop

./scripts/translations.sh -m
./scripts/icons.sh -x

mkdir src/build
flatpak-builder src/build dist/linux/io.github.ctlcltd.e2se.yaml --force-clean
flatpak-builder src/build dist/linux/io.github.ctlcltd.e2se.yaml --force-clean --user --install

# flatpak run io.github.ctlcltd.e2se

