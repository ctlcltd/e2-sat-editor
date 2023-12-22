#!/bin/bash
# Translation files utility script
# 

usage () {
	if [[ -z "$1" ]]; then
		printf "%s\n\n" "Translation files utility script"
	fi

	printf "%s\n\n" "bash translations.sh [OPTIONS]"
	printf "%s\n"   "-u --lupdate       Qt Linguist lupdate executable."
	printf "%s\n"   "-r --lrelease      Qt Linguist lrelease executable."
	printf "%s\n"   "-c --lconvert      Qt Linguist lconvert executable."
	printf "%s\n"   "-g --generate      Task: generate"
	printf "%s\n"   "-m --make          Task: make"
	printf "%s\n"   "   --merge         Task: merge"
	printf "%s\n"   "-h --help          Display this help and exit."
}

#//TODO improve
cwt () {
	if [[ $(basename $PWD) != "e2-sat-editor" ]]; then
		echo "Current working directory differs from \"e2-sat-editor\".";

		exit 1;
	fi
}

init () {
	if [[ -z "$LUPDATE" ]]; then
		if [[ -n $(type -t lupdate) ]]; then
			LUPDATE="lupdate"
		fi
	fi
	if [[ -z "$LRELEASE" ]]; then
		if [[ -n $(type -t lrelease) ]]; then
			LRELEASE="lrelease"
		fi
	fi
	if [[ -z "$LCONVERT" ]]; then
		if [[ -n $(type -t lconvert) ]]; then
			LCONVERT="lconvert"
		fi
	fi

	if [[ -z $(type -t "$LUPDATE") ]]; then
		echo "Qt Linguist lupdate not found."

		exit 1;
	fi
	if [[ -z $(type -t "$LRELEASE") ]]; then
		echo "Qt Linguist lrelease not found."

		exit 1;
	fi
	if [[ -z $(type -t "$LCONVERT") ]]; then
		echo "Qt Linguist lconvert not found."

		exit 1;
	fi
}

lang_generate () {
	printf "%s\n\n" "generate."

	cwt

	ABSDIR="$PWD/src/gui"

	mkdir -p translations

	for lang in "ar" "bg" "ca" "cs" "da" "de" "es" "fa" "fi" "fr" "gd" "gl" "he" "hr" "hu" "it" "ja" "ko" "lt" "lv" "nl" "nn" "pl" "pt_BR" "pt_PT" "ru" "sk" "sl" "sv" "tr" "uk" "zh_CN" "zh_TW"; do
		$LUPDATE -recursive $ABSDIR -ts translations/e2se_$lang.ts -locations none
	done
}

lang_make () {
	printf "%s\n\n" "make."

	cwt

	mkdir -p dist/translations

	for lang in "ar" "bg" "ca" "cs" "da" "de" "es" "fa" "fi" "fr" "gd" "gl" "he" "hr" "hu" "it" "ja" "ko" "lt" "lv" "nl" "nn" "pl" "pt_BR" "pt_PT" "ru" "sk" "sl" "sv" "tr" "uk" "zh_CN" "zh_TW"; do
		$LRELEASE translations/e2se_$lang.ts -qm dist/translations/e2se_$lang.qm
	done
}

lang_merge () {
	printf "%s\n\n" "merge."

	cwt

	mkdir -p workflow/ts-merge

	for lang in "ar" "bg" "ca" "cs" "da" "de" "es" "fa" "fi" "fr" "gd" "gl" "he" "hr" "hu" "it" "ja" "ko" "lt" "lv" "nl" "nn" "pl" "pt_BR" "pt_PT" "ru" "sk" "sl" "sv" "tr" "uk" "zh_CN" "zh_TW"; do
		printf "%s: %s > %s > %s\n" "merging" "translations/e2se_$lang.ts" "workflow/ts-input/$lang.ts" "workflow/ts-merge/$lang.ts"
		$LCONVERT -i translations/e2se_$lang.ts -i workflow/ts-input/$lang.ts -o workflow/ts-merge/e2se_$lang.ts
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
		-u*|--lupdate*)
			LUPDATE="$2"
			init
			shift
			shift
			;;
		-r*|--lrelease*)
			LRELEASE="$2"
			init
			shift
			shift
			;;
		-c*|--lconvert*)
			LCONVERT="$2"
			init
			shift
			shift
			;;
		-g|--generate)
			init
			lang_generate
			shift
			;;
		-m|--make)
			init
			lang_make
			shift
			;;
		--merge)
			init
			lang_merge
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

