#!/bin/bash
# Testing appimage
# 

# cd dist/linux
# python -m venv .pythonenv
# source ./.pythonenv/bin/activate
# python -m pip install appimage-builder
# deactivate
# cd ../..

# cd src
# cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DWITHOUT_CLI=ON -B build
# make -C build
# make -C build install DESTDIR=../AppDir

# source ../dist/linux/.pythonenv/bin/activate
appimage-builder --recipe ../dist/linux/AppImageBuilder.yaml
# deactivate

# mv "e2 SAT Editor-0.7.0-x86_64.AppImage" e2-sat-editor-0.7.0-x86_64.AppImage
# mv "e2 SAT Editor-0.7.0-x86_64.AppImage.zsync" e2-sat-editor-0.7.0-x86_64.AppImage.zsync
# ./e2-sat-editor-0.7.0-x86_64.AppImage

