#!/bin/bash
# Testing snapcraft
# 

# cd dist/xdg
# desktop-file-validate e2-sat-editor.desktop
# desktop-file-validate e2se-cli.desktop

./scripts/translations.sh -m
./scripts/icons.sh -x

cd dist/linux

# rm e2-sat-editor_*.snap
# snap list
# snap remove e2-sat-editor

# snapcraft expand-extensions

snapcraft clean
snapcraft --debug

# snap install --devmode e2-sat-editor_*.snap
# snap install --dangerous e2-sat-editor_*.snap
# snap run --shell e2-sat-editor

# e2-sat-editor
# exit

