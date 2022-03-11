#!/bin/bash
# Install pyftpdlib in a Python venv to simulate FTP

if [[ -n $(type -t python3) ]]; then
	PYTHON="python3"
elif [[ -n $(type -t py) ]]; then
	PYTHON="py"
elif [[ -n $(type -t python) ]]; then
	PYTHON="python"
fi
if [[ -z $PYTHON ]]; then
	echo "python3 not found.";
	exit 1;
fi
PYTHON_VER=$($PYTHON --version)
if [[ ${PYTHON_VER::7} -ge 3 ]]; then
	echo "python version mismatch.";
	exit 1;
fi

echo "installing python venv ..."
$PYTHON -m venv .pythonenv
echo "installing pyftpdlib (pip) ..."
$PYTHON -m pip install pyftpdlib
echo "done."
