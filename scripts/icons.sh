#!/bin/bash
# Convert icons
# - iconutil (macOS)
# - straight copy (*ux)
# - ImageMagick (Windows)
# 

usage () {
	if [[ -z "$1" ]]; then
		printf "%s\n\n" "Convert icons"
	fi

	printf "%s\n\n" "bash icons.sh [OPTIONS]"
	printf "%s\n"   "-w --icons-win     Task: icons-win"
	printf "%s\n"   "-m --icons-macx    Task: icons-macx"
	printf "%s\n"   "-x --icons-ux      Task: icons-ux"
	printf "%s\n"   "-h --help          Display this help and exit."
}

#TODO improve
cwt () {
	if [[ $(basename $PWD) != "e2-sat-editor" ]]; then
		echo "Current working directory differs from \"e2-sat-editor\".";

		exit 1;
	fi
}

icons_win () {
	printf "%s\n\n" "icons-win."

	cwt

	magick res/e2-sat-editor/icon-256.png res/e2-sat-editor/icon-128.png res/e2-sat-editor/icon-64.png res/e2-sat-editor/icon-48.png res/e2-sat-editor/icon-32.png dist/windows/e2-sat-editor.ico
}

icons_macx () {
	printf "%s\n\n" "icons-macx."

	cwt

	mkdir -p dist/macos/icons.iconset
	cp res/e2-sat-editor/icon-{512,256,128,64,32}.png dist/macos/icons.iconset
	mv dist/macos/icons.iconset/icon-512.png dist/macos/icons.iconset/icon_512x512.png
	mv dist/macos/icons.iconset/icon-256.png dist/macos/icons.iconset/icon_256x256.png
	mv dist/macos/icons.iconset/icon-128.png dist/macos/icons.iconset/icon_128x128.png
	mv dist/macos/icons.iconset/icon-64.png dist/macos/icons.iconset/icon_64x64.png
	mv dist/macos/icons.iconset/icon-32.png dist/macos/icons.iconset/icon_32x32.png

	iconutil -c icns dist/macos/icons.iconset -o dist/macos/e2-sat-editor.icns

	rm -R dist/macos/icons.iconset
}

icons_ux () {
	printf "%s\n\n" "icons-ux."

	cwt

	mkdir -p dist/xdg/icons/{scalable,512x512,256x256,128x128,64x64,48x48,32x32}
	cp res/e2-sat-editor/icon.svg dist/xdg/icons/scalable/e2-sat-editor.svg
	cp res/e2-sat-editor/icon-512.png dist/xdg/icons/512x512/e2-sat-editor.png
	cp res/e2-sat-editor/icon-256.png dist/xdg/icons/256x256/e2-sat-editor.png
	cp res/e2-sat-editor/icon-128.png dist/xdg/icons/128x128/e2-sat-editor.png
	cp res/e2-sat-editor/icon-64.png dist/xdg/icons/64x64/e2-sat-editor.png
	cp res/e2-sat-editor/icon-48.png dist/xdg/icons/48x48/e2-sat-editor.png
	cp res/e2-sat-editor/icon-32.png dist/xdg/icons/32x32/e2-sat-editor.png
	cp res/e2-sat-editor/icon-cli.svg dist/xdg/icons/scalable/e2se-cli.svg
	cp res/e2-sat-editor/icon-cli-64.png dist/xdg/icons/64x64/e2se-cli.png
}

complete () {
	printf "\n%s\n" "done."
}


if [[ -z "$@" ]]; then
	usage

	exit 0
fi

for SRG in "$@"; do
	case "$SRG" in
		-w|--icons-win)
			icons_win
			shift
			;;
		-m|--icons-macx)
			icons_macx
			shift
			;;
		-x|--icons-ux)
			icons_ux
			shift
			;;
		-h|--help)
			usage

			exit 0
			;;
		-*)
			[[ "$1" == "-"* ]] && shift
			printf "%s: %s %s\n\n" "$0" "Illegal option" "$2"

			usage 1

			exit 1
			;;
		*)
			[[ "$1" != -* ]] && usage
			;;
	esac
done

complete

