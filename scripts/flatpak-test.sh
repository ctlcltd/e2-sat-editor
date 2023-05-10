#!/bin/bash
# Testing flatpak
# 

mkdir src/build
flatpak-builder src/build dist/linux/io.github.ctlcltd.e2-sat-editor.yaml --force-clean
flatpak-builder src/build dist/linux/io.github.ctlcltd.e2-sat-editor.yaml --force-clean --user --install

flatpak run io.github.ctlcltd.e2-sat-editor

