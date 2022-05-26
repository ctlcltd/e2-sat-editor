#!/bin/bash
# Convert icons
# - iconutil (macos)
# - straight copy (linux)
# - IMagick (windows)

mkdir -p dist/macos/icons.iconset
cp res/e2-sat-editor/icon-{512,256,128,64}.png dist/macos/icons.iconset
mv dist/macos/icons.iconset/icon-512.png dist/macos/icons.iconset/icon_512x512.png
mv dist/macos/icons.iconset/icon-256.png dist/macos/icons.iconset/icon_256x256.png
mv dist/macos/icons.iconset/icon-128.png dist/macos/icons.iconset/icon_128x128.png
mv dist/macos/icons.iconset/icon-64.png dist/macos/icons.iconset/icon_64x64.png
iconutil -c icns dist/macos/icons.iconset -o dist/macos/e2-sat-editor.icns
rm -R dist/macos/icons.iconset

mkdir -p dist/linux/icons/{scalable,128x128,64x64}
cp res/e2-sat-editor/icon.svg dist/linux/icons/scalable/e2-sat-editor.svg
cp res/e2-sat-editor/icon-128.png dist/linux/icons/128x128/e2-sat-editor.png
cp res/e2-sat-editor/icon-64.png dist/linux/icons/64x64/e2-sat-editor.png

convert res/e2-sat-editor/icon-256.png res/e2-sat-editor/icon-128.png res/e2-sat-editor/icon-64.png dist/windows/e2-sat-editor.ico
