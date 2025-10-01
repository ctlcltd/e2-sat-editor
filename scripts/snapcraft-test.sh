#!/bin/bash
# Testing snapcraft
# 

# snap install snapcraft --classic

# git clone https://github.com/ctlcltd/e2-sat-editor.git
# git checkout $COMMIT
# cd e2-sat-editor

# cd dist/xdg
# desktop-file-validate e2-sat-editor.desktop
# desktop-file-validate e2se-cli.desktop
# cd ../..

# snap remove e2-sat-editor

# apt-get install qt6-i10n-tools qmake6
./scripts/translations.sh -m
./scripts/icons.sh -x

mkdir build
git init

cp dist/linux/snapcraft.yaml build/
cd build
cat snapcraft.yaml

# snapcraft expand-extensions
snapcraft clean
snapcraft --debug

# snapcraft remote-build --build-for=amd64
# snapcraft remote-build --build-for=arm64

# snap install --devmode e2-sat-editor_*.snap
# snap install --dangerous e2-sat-editor_*.snap
# snap run --shell e2-sat-editor

# e2-sat-editor --debug
# exit

