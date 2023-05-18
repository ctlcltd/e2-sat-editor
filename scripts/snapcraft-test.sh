#!/bin/bash
# Testing snapcraft
# 

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

