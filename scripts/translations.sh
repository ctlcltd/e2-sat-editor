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
	printf "%s\n"   "-g --generate      Task: generate"
	printf "%s\n"   "-m --make          Task: make"
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

	if [[ -z $(type -t "$LUPDATE") ]]; then
		echo "Qt Linguist lupdate not found."

		exit 1;
	fi
	if [[ -z $(type -t "$LRELEASE") ]]; then
		echo "Qt Linguist lrelease not found."

		exit 1;
	fi
}

i18n_generate () {
	printf "%s\n\n" "generate."

	cwt

	ABSDIR="$PWD/src/gui"

	mkdir -p translations

	$LUPDATE -recursive $ABSDIR -ts translations/e2se_ar.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_bg.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_ca.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_cs.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_da.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_de.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_es.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_fa.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_fi.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_fr.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_gd.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_gl.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_he.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_hr.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_hu.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_it.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_ja.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_ko.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_lt.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_lv.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_nl.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_nn.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_pl.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_pt_BR.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_pt_PT.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_ru.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_sk.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_sl.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_sv.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_tr.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_uk.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_zh_CN.ts -locations none
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_zh_TW.ts -locations none
}

i18n_make () {
	printf "%s\n\n" "make."

	cwt

	mkdir -p res/locale

	$LRELEASE translations/e2se_ar.ts -qm dist/translations/e2se_ar.qm
	$LRELEASE translations/e2se_bg.ts -qm dist/translations/e2se_bg.qm
	$LRELEASE translations/e2se_ca.ts -qm dist/translations/e2se_ca.qm
	$LRELEASE translations/e2se_cs.ts -qm dist/translations/e2se_cs.qm
	$LRELEASE translations/e2se_da.ts -qm dist/translations/e2se_da.qm
	$LRELEASE translations/e2se_de.ts -qm dist/translations/e2se_de.qm
	$LRELEASE translations/e2se_es.ts -qm dist/translations/e2se_es.qm
	$LRELEASE translations/e2se_fa.ts -qm dist/translations/e2se_fa.qm
	$LRELEASE translations/e2se_fi.ts -qm dist/translations/e2se_fi.qm
	$LRELEASE translations/e2se_fr.ts -qm dist/translations/e2se_fr.qm
	$LRELEASE translations/e2se_gd.ts -qm dist/translations/e2se_gd.qm
	$LRELEASE translations/e2se_gl.ts -qm dist/translations/e2se_gl.qm
	$LRELEASE translations/e2se_he.ts -qm dist/translations/e2se_he.qm
	$LRELEASE translations/e2se_hr.ts -qm dist/translations/e2se_hr.qm
	$LRELEASE translations/e2se_hu.ts -qm dist/translations/e2se_hu.qm
	$LRELEASE translations/e2se_it.ts -qm dist/translations/e2se_it.qm
	$LRELEASE translations/e2se_ja.ts -qm dist/translations/e2se_ja.qm
	$LRELEASE translations/e2se_ko.ts -qm dist/translations/e2se_ko.qm
	$LRELEASE translations/e2se_lt.ts -qm dist/translations/e2se_lt.qm
	$LRELEASE translations/e2se_lv.ts -qm dist/translations/e2se_lv.qm
	$LRELEASE translations/e2se_nl.ts -qm dist/translations/e2se_nl.qm
	$LRELEASE translations/e2se_nn.ts -qm dist/translations/e2se_nn.qm
	$LRELEASE translations/e2se_pl.ts -qm dist/translations/e2se_pl.qm
	$LRELEASE translations/e2se_pt_BR.ts -qm dist/translations/e2se_pt_BR.qm
	$LRELEASE translations/e2se_pt_PT.ts -qm dist/translations/e2se_pt_PT.qm
	$LRELEASE translations/e2se_ru.ts -qm dist/translations/e2se_ru.qm
	$LRELEASE translations/e2se_sk.ts -qm dist/translations/e2se_sk.qm
	$LRELEASE translations/e2se_sl.ts -qm dist/translations/e2se_sl.qm
	$LRELEASE translations/e2se_sv.ts -qm dist/translations/e2se_sv.qm
	$LRELEASE translations/e2se_tr.ts -qm dist/translations/e2se_tr.qm
	$LRELEASE translations/e2se_uk.ts -qm dist/translations/e2se_uk.qm
	$LRELEASE translations/e2se_zh_CN.ts -qm dist/translations/e2se_zh_CN.qm
	$LRELEASE translations/e2se_zh_TW.ts -qm dist/translations/e2se_zh_TW.qm
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
		-g|--generate)
			init
			i18n_generate
			shift
			;;
		-m|--make)
			init
			i18n_make
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

