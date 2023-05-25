#!/bin/bash
# Testing snapcraft
# 

# cd dist/xdg
# desktop-file-validate e2-sat-editor.desktop
# desktop-file-validate e2se-cli.desktop
# cd ../..

# apt-get install qt6-i10n-tools qmake6
## BUG-1964763 https://askubuntu.com/a/1460243

./scripts/translations.sh -m
./scripts/icons.sh -x

cd dist/linux

# rm e2-sat-editor_*.snap
# snap list
# snap remove e2-sat-editor

# snapcraft expand-extensions

snapcraft clean
snapcraft --debug

# snapcraft remote-build --build-on=amd64,arm64

# snap install --devmode e2-sat-editor_*.snap
# snap install --dangerous e2-sat-editor_*.snap
# snap run --shell e2-sat-editor

# e2-sat-editor
# exit

