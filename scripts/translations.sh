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

	$LUPDATE -recursive $ABSDIR -ts translations/e2se_ar.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_bg.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_ca.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_cs.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_da.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_de.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_es.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_fa.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_fi.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_fr.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_gd.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_he.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_hr.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_hu.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_it.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_ja.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_ko.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_lv.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_nl.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_nn.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_pl.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_pt_BR.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_ru.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_sk.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_tr.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_uk.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_zh_CN.ts
	$LUPDATE -recursive $ABSDIR -ts translations/e2se_zh_TW.ts
}

i18n_make () {
	printf "%s\n\n" "make."

	cwt

	mkdir -p res/locale

	$LRELEASE translations/e2se_ar.ts -qm res/locale/e2se_ar.qm
	$LRELEASE translations/e2se_bg.ts -qm res/locale/e2se_bg.qm
	$LRELEASE translations/e2se_ca.ts -qm res/locale/e2se_ca.qm
	$LRELEASE translations/e2se_cs.ts -qm res/locale/e2se_cs.qm
	$LRELEASE translations/e2se_da.ts -qm res/locale/e2se_da.qm
	$LRELEASE translations/e2se_de.ts -qm res/locale/e2se_de.qm
	$LRELEASE translations/e2se_es.ts -qm res/locale/e2se_es.qm
	$LRELEASE translations/e2se_fa.ts -qm res/locale/e2se_fa.qm
	$LRELEASE translations/e2se_fi.ts -qm res/locale/e2se_fi.qm
	$LRELEASE translations/e2se_fr.ts -qm res/locale/e2se_fr.qm
	$LRELEASE translations/e2se_gd.ts -qm res/locale/e2se_gd.qm
	$LRELEASE translations/e2se_he.ts -qm res/locale/e2se_he.qm
	$LRELEASE translations/e2se_hr.ts -qm res/locale/e2se_hr.qm
	$LRELEASE translations/e2se_hu.ts -qm res/locale/e2se_hu.qm
	$LRELEASE translations/e2se_it.ts -qm res/locale/e2se_it.qm
	$LRELEASE translations/e2se_ja.ts -qm res/locale/e2se_ja.qm
	$LRELEASE translations/e2se_ko.ts -qm res/locale/e2se_ko.qm
	$LRELEASE translations/e2se_lv.ts -qm res/locale/e2se_lv.qm
	$LRELEASE translations/e2se_nl.ts -qm res/locale/e2se_nl.qm
	$LRELEASE translations/e2se_nn.ts -qm res/locale/e2se_nn.qm
	$LRELEASE translations/e2se_pl.ts -qm res/locale/e2se_pl.qm
	$LRELEASE translations/e2se_pt_BR.ts -qm res/locale/e2se_pt_BR.qm
	$LRELEASE translations/e2se_ru.ts -qm res/locale/e2se_ru.qm
	$LRELEASE translations/e2se_sk.ts -qm res/locale/e2se_sk.qm
	$LRELEASE translations/e2se_tr.ts -qm res/locale/e2se_tr.qm
	$LRELEASE translations/e2se_uk.ts -qm res/locale/e2se_uk.qm
	$LRELEASE translations/e2se_zh_CN.ts -qm res/locale/e2se_zh_CN.qm
	$LRELEASE translations/e2se_zh_TW.ts -qm res/locale/e2se_zh_TW.qm
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

