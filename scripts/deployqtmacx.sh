#!/bin/bash
# deployqtmacx.sh
# Finalize app bundle like macdeployqt with path resolver
# 
# link: https://github.com/e2se/deployqtmacx
# copyright: e2 SAT Editor Team
# author: Leonardo Laureti
# license: MIT License
# license: GNU GPLv3 License
# 

# private boolean _VERBOSE
_VERBOSE=false
# private boolean _SIMULATE
_SIMULATE=false
# private boolean _PRINT_EXCLUDED
_PRINT_EXCLUDED=false
# private boolean _FORCE_OVERWRITE
_FORCE_OVERWRITE=false
# private boolean _DEPLOY_PLUGINS
_DEPLOY_PLUGINS=true
# private array _EXCLUDE_LIB_PATHS
_EXCLUDE_LIB_PATHS=("/usr/lib" "/System/Library")
# private array _PLUGINS
_PLUGINS=("platforms" "styles")
# private string _QT_PATH
_QT_PATH=""
# private string _FRAMEWORK_PATH
_FRAMEWORK_PATH=""
# private string _PLUGINS_PATH
_PLUGINS_PATH=""
# private string _INPUT_FILE
_INPUT_FILE=""

# private boolean __HASREALPATH
__HASREALPATH=""
# private string __BREWLIBPATH
__BREWLIBPATH=""
# private string __BINFILE
__BINFILE=""
# private array __EPATHS
__EPATHS=()
# private string __ETEMPVAR
__ETEMPVAR=""
# private array __DPATHS
__DPATHS=()
# private string __DTEMPVAR
__DTEMPVAR=""


usage () {
	if [[ -z "$1" ]]; then
		printf "%s\n\n" "Finalize app bundle like macdeployqt with path resolver."
	fi

	printf "%s\n\n" "Usage: bash deployqtmacx.sh AppBundle.app [OPTIONS]"
	printf "%s\n"   "Options:"
	printf "%s\n"   "--verbose                 Verbose"
	printf "%s\n"   "-s --simulate             Simulate actions without write files"
	printf "%s\n"   "-e --print-excluded       Print excluded files"
	printf "%s\n"   "-f --force                Force overwrite of files"
	printf "%s\n"   "-np --no-deploy-plugins   Disallow plugins deploy"
	printf "%s\n"   "-plugins                  Plugins to deploy (platforms styles)"
	printf "%s\n"   "-exclude-lib-paths        Library paths to exclude (/usr/lib /System/Library)"
	printf "%s\n"   "-qt-path                  Set Qt path"
	printf "%s\n"   "-framework-path           Set Qt framework path"
	printf "%s\n"   "-plugins-path             Set Qt plugins path"
	printf "%s\n"   "-h --help                 Display this help and exit"
}

error () {
	local msg="$1"

	# if [[ "$_VERBOSE" == true ]]; then
	# 	echo "$msg"
	# fi

	echo "$msg" >&2
}

has_realpath () {
	if [[ -n "$__HASREALPATH" ]]; then
		echo "$__HASREALPATH"
	fi

	local run=$(type -t realpath)

	if [[ -n "$run" ]]; then
		__HASREALPATH=1
	fi
}

is_exec () {
	local path="$1"

	if [[ "$path" =~ "/MacOS" ]]; then
		echo 1
	fi
}

is_framework () {
	local path="$1"

	if [[ "$path" =~ ".framework" ]]; then
		echo 1
	fi
}

is_plugin () {
	local path="$1"

	if [[ "$path" =~ "/plugins" || "$path" =~ "/PlugIns" ]]; then
		echo 1
	fi
}

path_exclusion () {
	local path="$1"

	for exclude in "${_EXCLUDE_LIB_PATHS[@]}"; do
		if [[ $(dirname "$path") =~ "$exclude" ]]; then
			echo 1

			return 0
		fi
	done
}

path_resolver () {
	local path="$1"
	local parentpath="$2"

	local _path="$path"

	if [[ "${path:0:1}" == "@" ]]; then
		_pos="${path%%\/*}"
		path="${path:${#_pos}}"

		path="$(dirname "$parentpath")$path"

		if [[ ! -e "$path" ]]; then
			path="$_path"

			if [[ $(is_plugin "$path") ]]; then
				parentpath=$(plugins_path)
			elif [[ $(is_framework "$path") ]]; then
				parentpath=$(framework_path)
			fi

			_pos="${path%%\/*}"
			path="${path:${#_pos}}"

			path="${parentpath}${path}"
		fi
	fi

	if [[ ! -e "$path" && $(has_realpath) ]]; then
		_pos="${_path%%\/*}"
		path="${_path:${#_pos}}"

		path="$(dirname "$parentpath")$path"
		path=$(realpath "$path")
	fi

	if [[ -e "$path" ]]; then
		echo "$path"
	fi
}

path_abspath () {
	local path="$1"
	local basepath="$2"
	local recursive="$3"

	local abspath="$path"

	if [[ "$recursive" == true ]]; then
		if [[ $(is_plugin "$path") && ! -d "$path" ]]; then
			abspath=$(dirname "$path")
		elif [[ $(is_framework "$path") ]]; then
			_len="${path%%.framework\/*}"
			abspath="${path:0:${#_len}}.framework"
		fi

		path="$abspath"
	fi
	if [[ -n "$basepath" ]]; then
		if [[ $(is_plugin "$path") ]]; then
			local parentpath=$(plugins_path)
			path="${path/$parentpath/}"
			path="${path:1}"
		elif [[ $(is_framework "$path") ]]; then
			local parentpath=$(framework_path)
			path="${path/$parentpath/}"
			path="${path:1}"
		else
			path=$(basename "$path")
		fi

		abspath="${basepath}/${path}"
	fi

	echo "$abspath"
}

path_source () {
	local path="$1"

	echo $(path_abspath "$path" "" true)
}

path_target () {
	local path="$1"

	local basepath="$_INPUT_FILE/Contents"

	if [[ $(is_plugin "$path") ]]; then
		basepath="${basepath}/PlugIns"
	else
		basepath="${basepath}/Frameworks"
	fi

	echo $(path_abspath "$path" "$basepath" true)
}

path_rpath () {
	local path="$1"

	local rpath=$(path_abspath "$path")

	rpath="${rpath/$_INPUT_FILE\/Contents/}"
	rpath="${rpath/\/Frameworks/}"
	rpath="${rpath/\/PlugIns/}"
	rpath="${rpath:1}"

	echo "$rpath"
}

path_exists () {
	local path="$1"

	__ETEMPVAR=""

	if [[ "$_SIMULATE" == false ]]; then
		if [[ -e "$path" ]]; then
			__ETEMPVAR=1
		fi
	else
		for value in "${__EPATHS[@]}"; do
			if [[ "$value" == "$path" ]]; then
				__ETEMPVAR=1

				break
			fi
		done

		if [[ ! "$__ETEMPVAR" ]]; then
			__EPATHS+=("$path")
		fi
	fi
}

retr_path_exists () {
	echo "$__ETEMPVAR"

	__ETEMPVAR=""
}

dependency_done () {
	local path="$1"

	__DTEMPVAR=""

	for value in "${__DPATHS[@]}"; do
		if [[ "$value" == "$path" ]]; then
			__DTEMPVAR=1

			break
		fi
	done

	if [[ ! "$__DTEMPVAR" ]]; then
		__DPATHS+=("$path")
	fi
}

retr_dependency_done () {
	echo "$__DTEMPVAR"

	__DTEMPVAR=""
}

relname () {
	local path="$1"

	_pos="${path%%Contents\/*}"
	_pos="${#_pos}"
	_pos=$_pos+9
	path="${path:$_pos}"

	echo "$path"
}

trim () {
	local value="$1"

	value=$(echo "$value" | xargs)

	echo "$value"
}

brew_lib_path () {
	if [[ -n "$__BREWLIBPATH" ]]; then
		echo "$__BREWLIBPATH"
	fi

	local run=$(which brew)

	local brew_path=$run

	if [[ -z "$brew_path" ]]; then
		return 0
	fi

	local brew_lib_path

	if [[ ! "$brew_path" =~ "/usr/local" ]]; then
		brew_lib_path="/opt/homebrew/lib"
	else
		brew_lib_path="/usr/local/lib"
	fi

	__BREWLIBPATH="$brew_lib_path"
}

qt_path () {
	if [[ -n "$_QT_PATH" ]]; then
		echo "$_QT_PATH"
	fi

	local run=$(which brew)

	local brew_path=$run

	if [[ -z "$brew_path" ]]; then
		error "$(printf "Error Homebrew path not found: %s\n" "$brew_path")"

		return 1
	fi

	local qt_path

	if [[ ! "$brew_path" =~ "/usr/local" ]]; then
		qt_path="/opt/homebrew"
	else
		qt_path="/usr/local"
	fi
	qt_path="${qt_path}/opt/qt"

	if [[ ! -e "$qt_path" ]]; then
		error "$(printf "Error Qt path not found: %s\n" "$qt_path")"

		return 1
	fi

	_QT_PATH="$qt_path"
}

framework_path () {
	if [[ -n "$_FRAMEWORK_PATH" ]]; then
		echo "$_FRAMEWORK_PATH"
	fi

	local qt_path=$(qt_path)

	local framework_path="${qt_path}/lib"

	if [[ ! -e "$framework_path" ]]; then
		error "$(printf "Error Qt framework path not found: %s\n" "$framework_path")"

		return 1
	fi

	_FRAMEWORK_PATH="$framework_path"
}

plugins_path () {
	if [[ -n "$_PLUGINS_PATH" ]]; then
		echo "$_PLUGINS_PATH"
	fi

	local qt_path=$(qt_path)

	local plugins_path="${qt_path}/share/qt/plugins"

	if [[ ! -e "$plugins_path" ]]; then
		error "$(printf "Error Qt plugins path not found: %s\n" "$plugins_path")"

		return 1
	fi

	_PLUGINS_PATH="$plugins_path"
}

recurse_dependency () {
	local path="$1"

	if [[ "$_VERBOSE" == true ]]; then
		local filename=$(basename "$path")
		printf "dependency: %s\n" "$filename"
	fi

	if [[ ! -e "$path" ]]; then
		error "$(printf "Error dependency file not found: %s\n" "$path")"

		return 1
	fi

	local run=$(dyld_info -dependents "$path" | tail -n +4)

	if [[ -z "$run" ]]; then
		error "$(printf "Error command [dyld_info]: %s\n" "$path")"

		return 1
	fi

	local parentpath="$path"

	IFS=$'\n'
	local deps=$run

	for line in $deps; do
		path=$(trim "$line")

		if [[ -z "$path" || $(path_exclusion "$path") ]]; then
			if [[ -n "$path" && "$_VERBOSE" == true && "$_PRINT_EXCLUDED" == true ]]; then
				printf "exclude: %s\n" "$path"
			fi

			continue
		fi

		local depname="$path"
		local _path="$path"

		path=$(path_resolver "$path" "$parentpath")

		if [[ -n "$path" ]]; then
			local run=$(otool -D "$path" | tail -n +2)

			_path="$path"
			path=$run

			if [[ -z "$run" ]]; then
				error "$(printf "Error command [otool]: %s (%s)  from: %s\n" "$_path" "$path" "$depname")"

				continue
			fi

			if [[ "$_VERBOSE" == true ]]; then
				printf "resolved: %s  from: %s\n" "$path" "$depname"
			fi


			local srcpath=$(path_source "$path")
			local dstpath=$(path_target "$path")

			local overwrite=false

			path_exists "$dstpath"

			if [[ "$_FORCE_OVERWRITE" == true || ! $(retr_path_exists) ]]; then
				overwrite=true
			elif [[ "$_VERBOSE" == true ]]; then
				local srcfilename=$(basename "$srcpath")
				local dstfilename=$(relname "$dstpath")
				printf "no overwrite: %s  already at: %s\n" "$srcfilename" "$dstfilename"
			fi

			if [[ "$overwrite" == true ]]; then
				copy_dependency "$srcpath" "$dstpath"
			fi


			local basepath="$_INPUT_FILE/Contents"

			if [[ $(is_plugin "$parentpath") ]]; then
				basepath="${basepath}/PlugIns"
			else
				basepath="${basepath}/Frameworks"
			fi

			local toppath="$parentpath"

			if [[ $(is_exec "$toppath") ]]; then
				toppath=$(path_abspath "$toppath")
			else
				toppath=$(path_abspath "$toppath" "$basepath")
			fi

			local libpath=$(path_abspath "$path" "$basepath")
			local rpath=$(path_rpath "$libpath")

			fix_dependency "$toppath" "$depname" "$rpath"

			if [[ "$overwrite" == true ]]; then
				fix_dependency "$libpath" "$depname" "$rpath"

				if [[ ! $(is_exec "$path") ]]; then
					remove_sign "$libpath"
				fi

				recurse_dependency "$path"

				if [[ ! "$_VERBOSE" ]]; then
					local relpath=$(relname "$libpath")
					echo "$relpath\n"
				fi
			fi
		else
			error "$(printf "Error not resolved: %s (%s)  from: %s\n" "$_path" "$path" "$depname")"
		fi
	done


	local basepath="$_INPUT_FILE/Contents"

	if [[ $(is_plugin "$parentpath") ]]; then
		basepath="${basepath}/PlugIns"
	else
		basepath="${basepath}/Frameworks"
	fi

	local toppath="$parentpath"

	if [[ $(is_exec "$toppath") ]]; then
		toppath=$(path_abspath "$toppath")
	else
		toppath=$(path_abspath "$toppath" "$basepath")
	fi

	dependency_done "$toppath"

	if [[ ! $(retr_dependency_done) ]]; then
		clean_dependency "$toppath" "$parentpath"

		if [[ $(is_exec "$toppath") || $(is_plugin "$toppath") || $(is_framework "$toppath") ]]; then
			complete_dependency "$toppath"
		fi
	fi
}

copy_dependency () {
	local srcpath="$1"
	local dstpath="$2"

	if [[ "$_VERBOSE" == true ]]; then
		local srcfilename=$(basename "$srcpath")
		local dstfilename=$(relname "$dstpath")
		printf "copy: %s  to: %s\n" "$srcfilename" "$dstfilename"
	fi

	if [[ "$_SIMULATE" == false ]]; then
		if [[ -d "$srcpath" ]]; then
			if [[ $(is_framework "$srcpath") ]]; then
				srcpath="${srcpath%\/}"
				dstpath="${dstpath%\/}"

				local filename=$(basename "$srcpath")
				filename="${filename%%.framework}"

				mkdir -p "$dstpath/Versions/A"

				cp "$srcpath/Versions/A/$filename" "$dstpath/Versions/A/$filename"
				ln -s "$dstpath/Versions/A/$filename" "$dstpath/$filename"
				ln -s "$dstpath/Versions/A" "$dstpath/Versions/Current"
			else
				cp -R "$srcpath" "$dstpath"
			fi
		else
			cp "$srcpath" "$dstpath"
		fi
	elif [[ "$_VERBOSE" == true ]]; then
		local _type="file"
		if [[ -d "$srcpath" ]]; then
			_type="directory"
		fi

		printf "  copy %s from \"%s\" to \"%s\"\n" "$_type" "$srcpath" "$dstpath"
	fi
}

fix_dependency () {
	local path="$1"
	local depname="$2"
	local rpath="$3"

	rpath="@rpath/$rpath"

	if [[ "$_VERBOSE" == true ]]; then
		local filename=$(basename "$path")
		printf "fix: %s  with: %s\n" "$filename" "$rpath"
	fi

	if [[ ! $(is_exec "$path") && $(basename "$path") == $(basename "$rpath") ]]; then
		if [[ "$_SIMULATE" == false ]]; then
			install_name_tool -id $rpath "$path"
		elif [[ "$_VERBOSE" == true ]]; then
			printf "  change dylib id to %s on \"%s\"\n" "$rpath" "$path"
		fi
	fi
	if [[ "$_SIMULATE" == false ]]; then
		install_name_tool -change $depname $rpath "$path"
	elif [[ "$_VERBOSE" == true ]]; then
		printf "  change rpath from %s to %s on \"%s\"\n" "$depname" "$rpath" "$path"
	fi
}

remove_sign () {
	local path="$1"

	if [[ "$_VERBOSE" == true ]]; then
		local filename=$(basename "$path")
		printf "unsign: %s\n" "$filename"
	fi

	if [[ "$_SIMULATE" == false ]]; then
		codesign --force -s - "$path"
	elif [[ "$_VERBOSE" == true ]]; then
		printf "  remove sign on \"%s\"\n" "$path"
	fi
}

clean_dependency () {
	local path="$1"
	local refpath="$2"

	if [[ -z "$refpath" ]]; then
		refpath="$path"
	fi

	if [[ "$_VERBOSE" == true ]]; then
		local filename=$(basename "$path")
		printf "clean: %s\n" "$filename"
	fi

	local run=$(otool -l "$refpath")

	if [[ -z "$run" ]]; then
		error "$(printf "Error command [otool]: %s\n" "$refpath")"

		return 1
	fi

	IFS=$'\n'
	local loads=$run

	local cmd ; local rpath

	for line in $loads; do
		if [[ "$line" =~ "Load command " ]]; then
			cmd=""
			rpath=""
		elif [[ "$line" =~ "cmd " ]]; then
			line=$(trim "$line")

			cmd="${line:4}"
		elif [[ "$line" =~ "path " ]]; then
			line=$(trim "$line")

			rpath="${line:5}"
			_len="${rpath% (*}"
			rpath="${rpath:0:${#_len}}"
		fi

		if [[ -n "$cmd" && -n "$rpath" ]]; then
			if [[ "$cmd" == "LC_RPATH" && "$rpath" =~ "@loader_path" ]]; then
				if [[ "$_SIMULATE" == false ]]; then
					install_name_tool -delete_rpath $rpath "$path"
				elif [[ "$_VERBOSE" == true ]]; then
					printf "  remove rpath %s on \"%s\"\n" "$rpath" "$path"
				fi
			fi

			cmd=""
			rpath=""
		fi
	done
}

complete_dependency () {
	local path="$1"

	if [[ "$_VERBOSE" == true ]]; then
		local filename=$(basename "$path")
		printf "complete: %s\n" "$filename"
	fi

	if [[ $(is_exec "$path") ]]; then
		local brew_lib_path=$(brew_lib_path)

		if [[ "$_SIMULATE" == false ]]; then
			if [[ -n "$brew_lib_path" ]]; then
				install_name_tool -delete_rpath "$brew_lib_path" "$path"
			fi
			install_name_tool -delete_rpath @executable_path/../Frameworks "$path"
			install_name_tool -add_rpath @loader_path/ "$path"
			install_name_tool -add_rpath @executable_path/../MacOS "$path"
			install_name_tool -add_rpath @executable_path/../Frameworks "$path"
		elif [[ "$_VERBOSE" == true ]]; then
			if [[ -n "$brew_lib_path" ]]; then
				printf "  remove rpath %s on \"%s\"\n" "$brew_lib_path" "$path"
			fi
			printf "  remove rpath %s on \"%s\"\n" "@executable_path/../Frameworks" "$path"
			printf "  add rpath %s on \"%s\"\n" "@loader_path/" "$path"
			printf "  add rpath %s on \"%s\"\n" "@executable_path/../MacOS" "$path"
			printf "  add rpath %s on \"%s\"\n" "@executable_path/../Frameworks" "$path"
		fi
	elif [[ $(is_plugin "$path") ]]; then
		if [[ "$_SIMULATE" == false ]]; then
			install_name_tool -add_rpath @executable_path/../Frameworks "$path"
		elif [[ "$_VERBOSE" == true ]]; then
			printf "  add rpath %s on \"%s\"\n" "@executable_path/../Frameworks" "$path"
		fi
	elif [[ $(is_framework "$path") ]]; then
		if [[ "$_SIMULATE" == false ]]; then
			install_name_tool -add_rpath @loader_path/Frameworks "$path"
			install_name_tool -add_rpath @executable_path/../Frameworks "$path"
		elif [[ "$_VERBOSE" == true ]]; then
			printf "  add rpath %s on \"%s\"\n" "@loader_path/Frameworks" "$path"
			printf "  add rpath %s on \"%s\"\n" "@executable_path/../Frameworks" "$path"
		fi
	fi
}

deploy_executable () {
	local filename="$1"

	local path="$__BINFILE/$filename"

	if [[ "$_VERBOSE" == true ]]; then
		local binfilename=$(relname "$path")
		printf "executable: %s\n" "$binfilename"
	fi

	recurse_dependency "$path"
}

deploy_plugin () {
	local plugin="$1"

	if [[ "$_VERBOSE" == true ]]; then
		printf "plugin: %s\n" "$plugin"
	fi

	local basepath=$(plugins_path)
	local path="${basepath}/$plugin"

	if [[ ! -e "$path" ]]; then
		error "$(printf "Error plug-in directory not found: %s\n" "$path")"

		return 1
	fi


	local srcpath=$(path_source "$path")
	local dstpath=$(path_target "$path")

	local overwrite=false

	path_exists "$dstpath"

	if [[ "$_FORCE_OVERWRITE" == true || ! $(retr_path_exists) ]]; then
		overwrite=true
	elif [[ "$_VERBOSE" == true ]]; then
		local srcfilename=$(basename "$srcpath")
		local dstfilename=$(relname "$dstpath")
		printf "no overwrite: %s  already at: %s\n" "$srcfilename" "$dstfilename"
	fi

	if [[ "$overwrite" == true ]]; then
		copy_dependency "$srcpath" "$dstpath"

		local run=$(ls -1 "$srcpath")

		if [[ -z "$run" ]]; then
			error "$(printf "Error plug-in files not found: %s\n" "$srcpath")"

			return 1
		fi

		IFS=$'\n'
		local libs=$run

		for filename in $libs; do
			filename=$(trim "$filename")
			local libpath="${srcpath}/$filename"

			recurse_dependency "$libpath"
		done
	fi
}

deploy () {
	local input_file="$_INPUT_FILE"

	if [[ ! -e "$input_file" ]]; then
		error "$(printf "Error input file not found: %s\n" "$input_file")"

		return 1
	fi

	if [[ "$_VERBOSE" == true ]]; then
		printf "bundle: %s\n" "$input_file"
	fi

	local binary_path="${input_file}/Contents/MacOS"
	__BINFILE="$binary_path"

	local run=$(ls -1 "$binary_path")

	if [[ -z "$run" ]]; then
		error "$(printf "Error executables not found: %s\n" "$input_file")"

		return 1
	fi

	has_realpath
	brew_lib_path
	qt_path
	framework_path
	plugins_path

	mkdir -p "${input_file}/Contents/Frameworks"

	IFS=$'\n'
	local execs=$run

	for filename in $execs; do
		filename=$(trim "$filename")

		deploy_executable "$filename"
	done

	if [[ "$_DEPLOY_PLUGINS" == true ]]; then
		mkdir -p "${input_file}/Contents/PlugIns"

		local plugins=("${_PLUGINS[@]}")

		for plugin in "${plugins[@]}"; do
			plugin=$(trim "$plugin")

			deploy_plugin "$plugin"
		done
	fi
}


if [[ -z "$@" ]]; then
	usage

	exit 0
fi

for SRG in "$@"; do
	case "$SRG" in
		--verbose)
			_VERBOSE=true
			shift
			;;
		-s|--simulate)
			_SIMULATE=true
			shift
			;;
		-e|--print-excluded)
			_PRINT_EXCLUDED=true
			shift
			;;
		-f|--force)
			_FORCE_OVERWRITE=true
			shift
			;;
		-np|--no-deploy-plugins)
			_DEPLOY_PLUGINS=false
			shift
			;;
		-plugins*)
			_PLUGINS=("$2")
			shift
			shift
			;;
		-exclude-lib-paths*)
			_EXCLUDE_LIB_PATHS=("$2")
			shift
			shift
			;;
		-qt-path*)
			_QT_PATH="$2"
			shift
			shift
			;;
		-framework-path*)
			_FRAMEWORK_PATH="$2"
			shift
			shift
			;;
		-plugins-path*)
			_PLUGINS_PATH="$2"
			shift
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
			[[ "$1" != -* ]] && _INPUT_FILE="$1"
			;;
	esac
done

deploy

