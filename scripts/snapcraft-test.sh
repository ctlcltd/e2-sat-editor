#!/bin/bash
# Testing snapcraft
# 

# apt-get install snapcraft

# apt-get install qt6-i10n-tools qmake6
## BUG-1964763 https://askubuntu.com/a/1460243

# git clone https://github.com/ctlcltd/e2-sat-editor.git
# git checkout v1.7.0
# cd e2-sat-editor

# cd dist/xdg
# desktop-file-validate e2-sat-editor.desktop
# desktop-file-validate e2se-cli.desktop
# cd ../..

./scripts/translations.sh -m
./scripts/icons.sh -x

cd dist/linux

# rm e2-sat-editor_*.snap
# snap list
# snap remove e2-sat-editor

# snapcraft expand-extensions

snapcraft clean
snapcraft --debug

# cd ../..
# mkdir build
# git init
# cp dist/linux/snapcraft.yaml build/
# cd build

# snapcraft remote-build --build-for=amd64
# snapcraft remote-build --build-for=arm64

# snap install --devmode e2-sat-editor_*.snap
# snap install --dangerous e2-sat-editor_*.snap
# snap run --shell e2-sat-editor

# e2-sat-editor
# exit

