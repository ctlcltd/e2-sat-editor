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

	magick res/e2-sat-editor/icon-{256,128,64,48,32}.png dist/windows/e2-sat-editor.ico
}

icons_macx () {
	printf "%s\n\n" "icons-macx."

	cwt

	mkdir -p dist/macos/icons.iconset

	for I in 512 256 128 64 32; do
		mv "res/e2-sat-editor/icon-$I.png" "dist/macos/icons.iconset/icon_${I}x${I}.png"
	done

	iconutil -c icns dist/macos/icons.iconset -o dist/macos/e2-sat-editor.icns

	rm -R dist/macos/icons.iconset
}

icons_ux () {
	printf "%s\n\n" "icons-ux."

	cwt

	mkdir -p dist/xdg/icons/{scalable,512x512,256x256,128x128,64x64,48x48,32x32}

	for I in 0 512 256 128 64 48 32; do
		I3=$([[ $i -eq 0]] && echo "scalable" || echo "${I}x${I}")
		IW=$([[ $i -eq 0]] || echo "-$I")
		IX=$([[ $i -eq 0]] && echo "svg" || echo "png")
		cp "res/e2-sat-editor/icon$IW.$IX" "dist/xdg/icons/$I3/e2-sat-editor.$IX"
	done

	for I in 0 64; do
		I3=$([[ $i -eq 0]] && echo "scalable" || echo "${I}x${I}")
		IW=$([[ $i -eq 0]] || echo "-$I")
		IX=$([[ $i -eq 0]] && echo "svg" || echo "png")
		cp "res/e2-sat-editor/icon-cli$IW.$IX" "dist/xdg/icons/$I3/e2se-cli.$IX"
	done
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

