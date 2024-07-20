#!/bin/bash

# Var section -------------------------------------------------------------------------
COMPUTER_NAME=`hostname`
NOW_DATE=`date +"%Y-%m-%d"`

SOURCE_DIR=`pwd`

BUILD_CONF=Release
#BUILD_CONF=Debug

# Compiler section --------------------------------------------------------------------
FC_C_COMPILER=$MSYS_ROOT/clang64/bin/clang.exe
FC_CXX_COMPILER=$MSYS_ROOT/clang64/bin/clang++.exe
FC_RC_COMPILER=$MSYS_ROOT/clang64/bin/windres.exe

FC_CC_BN=`basename $FC_C_COMPILER`
FC_CC_VER=`$FC_C_COMPILER -dumpversion`
FC_BLD_STR=$FC_CC_BN-$FC_CC_VER
FC_BLD_DIR=$SOURCE_DIR/.chacha

# -------------------------------------------------------------------------------------
mkdir -p $FC_BLD_DIR
pushd $FC_BLD_DIR

echo -ne "MSYS_ROOT       : $MSYS_ROOT\n"
echo -ne "BUILD_CONF      : $BUILD_CONF\n"
echo -ne "FC_C_COMPILER   : $FC_C_COMPILER\n"
echo -ne "FC_CXX_COMPILER : $FC_CXX_COMPILER\n"
echo -ne "FC_BLD_STR      : $FC_BLD_STR\n"
echo -ne "FC_BLD_DIR      : $FC_BLD_DIR\n"
echo -ne "\n"

# -------------------------------------------------------------------------------------
cmake -Wno-dev -G "Ninja" \
-DCMAKE_PREFIX_PATH="$MSYS_ROOT/clang64" \
-DCMAKE_SYSTEM_PREFIX_PATH="$MSYS_ROOT/clang64" \
-DCMAKE_BUILD_TYPE:STRING="$BUILD_CONF" \
-DCMAKE_CONFIGURATION_TYPES:STRING="Debug;Release" \
-DCMAKE_C_COMPILER:FILEPATH="$FC_C_COMPILER" \
-DCMAKE_CXX_COMPILER:FILEPATH="$FC_CXX_COMPILER" \
-DCMAKE_RC_COMPILER:FILEPATH="$FC_RC_COMPILER" \
-DCMAKE_INSTALL_PREFIX="$SOURCE_DIR/instd" \
-DFREECAD_RELEASE_PDB=1 \
-DFREECAD_LIBPACK_USE=0 \
-DBUILD_ENABLE_CXX_STD="C++20" \
-DCMAKE_VERBOSE_MAKEFILE=0 \
-DBUILD_FEM=0 \
-DBUILD_MESH=0 \
-DBUILD_GUI=1 \
-DBUILD_DRAWING=1 \
-DBUILD_ASSEMBLY=0 \
-DFREECAD_USE_EXTERNAL_SMESH=0 \
-DENABLE_DEVELOPER_TESTS=0 \
-DBUILD_DESIGNER_PLUGIN=0 \
-DFREECAD_USE_PCH=1 \
-DFREECAD_USE_OCC_VARIANT="SYSTEM" \
-DFREECAD_BLD_STR="$FC_BLD_STR" \
-DPACKAGE_WCURL="https://github.com/wcdnail/FreeCAD" \
$SOURCE_DIR

popd
