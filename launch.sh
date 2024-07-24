#!/bin/bash

export MINGW_PREFIX=$MSYS_ROOT/clang64
export FC_ROOT=c:/_/FreeCAD/.chacha-debug
export FC_PYTHON_STDLIB=$(python -c "import sysconfig; print(sysconfig.get_path('stdlib'))")

# Log: PythonSearchPath = C:/msys/clang64/lib/python311.zip;C:/msys/clang64/lib/python3.11;D:/a/msys64/clang64/lib/python3.11/lib-dynload
#export PYTHONPATH=${FCROOT}/bin;${FCROOT}/Mod/Start

pushd $FC_ROOT

mkdir ${FC_ROOT}/config 2> /dev/null

./bin/FreeCAD.exe --log-file ${FC_ROOT}/log.txt -u ${FC_ROOT}/config/user.cfg ${FC_ROOT}/config/system.cfg

cat log.txt

popd
