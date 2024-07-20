#!/bin/bash

# Var section -------------------------------------------------------------------------
COMPUTER_NAME=`hostname`
NOW_DATE=`date +"%Y-%m-%d"`

SOURCE_DIR=`pwd`

BUILD_CONF=Release
#BUILD_CONF=Debug

# Compiler section --------------------------------------------------------------------
VT_C_COMPILER=/usr/bin/clang
VT_CXX_COMPILER=/usr/bin/clang++
#VT_C_COMPILER=/usr/bin/gcc
#VT_CXX_COMPILER=/usr/bin/g++

VT_CC_BN=`basename $VT_C_COMPILER`
VT_CC_VER=`$VT_C_COMPILER -dumpversion`

# -------------------------------------------------------------------------------------
mkdir -p ${SOURCE_DIR}/bld
pushd ${SOURCE_DIR}/bld

echo -ne "VT_C_COMPILER   : $VT_C_COMPILER\n"
echo -ne "VT_CXX_COMPILER : $VT_CXX_COMPILER\n"
echo -ne "VCPKG_VERSION   : ga-$VT_CC_BN-$VT_CC_VER\n\n"

# -------------------------------------------------------------------------------------
cmake -G "Ninja" \
-DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
-DCMAKE_BUILD_TYPE:STRING="$BUILD_CONF" \
-DCMAKE_CONFIGURATION_TYPES:STRING="$BUILD_CONF" \
-DCMAKE_C_COMPILER:FILEPATH="$VT_C_COMPILER" \
-DCMAKE_CXX_COMPILER:FILEPATH="$VT_CXX_COMPILER" \
-DCMAKE_INSTALL_PREFIX="${SOURCE_DIR}/instd" \
-DFREECAD_LIBPACK_USE=0 \
-DBUILD_GUI=ON \
-DBUILD_DRAWING=ON \
-DBUILD_ASSEMBLY=OFF \
-DFREECAD_USE_EXTERNAL_SMESH=0 \
-DENABLE_DEVELOPER_TESTS=0 \
-DBUILD_DESIGNER_PLUGIN=0 \
%SOURCE_DIR%

popd
