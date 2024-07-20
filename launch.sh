#!/bin/bash

pushd .chacha

# Log: PythonSearchPath = C:/msys/clang64/lib/python311.zip;C:/msys/clang64/lib/python3.11;D:/a/msys64/clang64/lib/python3.11/lib-dynload

export MINGW_PREFIX=$MSYS_ROOT/clang64
export FCROOT=c:/_/FreeCAD/.chacha
export PYTHONPATH=${FCROOT}/bin;${FCROOT}/Mod/Start

mkdir ${FCROOT}/config 2> /dev/null

./bin/FreeCAD.exe --log-file ${FCROOT}/log.txt -u ${FCROOT}/config/user.cfg ${FCROOT}/config/system.cfg

cat log.txt

popd
