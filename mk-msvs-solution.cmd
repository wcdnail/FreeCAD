@echo off

rem Var section -------------------------------------------------------------------------

:: set BUILD_CONF=Release
set BUILD_CONF=Debug

call %CD%\mk-vars.cmd
call %CD%\mk-trace.cmd

rem Compiler section --------------------------------------------------------------------
set VT_CMAKE_GENERATOR=%MSVC_CMAKE_GENERATOR%
set VT_C_COMPILER=%MSVC_C_COMPILER%
set VT_CXX_COMPILER=%MSVC_CXX_COMPILER%

rem -------------------------------------------------------------------------------------
pushd %SOURCE_DIR%
mkdir %BUILD_DIR% 2>NUL
cd %BUILD_DIR%

rem -------------------------------------------------------------------------------------
:: -DNGLIB_INCLUDE_DIR="%VCPKG_ROOT%\installed\x64-windows\include" ^
:: -DCMAKE_CONFIGURATION_TYPES:STRING="%BUILD_CONF%" ^

chcp 65001 >NUL
cmake -Wno-dev -G %VT_CMAKE_GENERATOR% ^
-DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
-DCMAKE_BUILD_TYPE:STRING="%BUILD_CONF%" ^
-DCMAKE_C_COMPILER:FILEPATH="%VT_C_COMPILER%" ^
-DCMAKE_CXX_COMPILER:FILEPATH="%VT_CXX_COMPILER%" ^
-DCMAKE_RC_COMPILER:FILEPATH="%MSVC_RC_COMPILER%" ^
-DCMAKE_INSTALL_PREFIX="instd" ^
-DFREECAD_LIBPACK_USE=0 ^
-DBUILD_GUI=ON ^
-DBUILD_DRAWING=ON ^
-DBUILD_ASSEMBLY=OFF ^
-DFREECAD_USE_EXTERNAL_SMESH=0 ^
-DENABLE_DEVELOPER_TESTS=0 ^
-DBUILD_DESIGNER_PLUGIN=0 ^
-DFREECAD_USE_SHIBOKEN=OFF ^
-DFREECAD_USE_PYSIDE=OFF ^
-DFREECAD_USE_PCH=1 ^
-DPACKAGE_WCURL="https://github.com/wcdnail/FreeCAD" ^
%SOURCE_DIR%

IF '%ERRORLEVEL%'=='0' GOTO CMAKE_OK
set RV=%ERRORLEVEL%
echo ERRORLEVEL: %RV%
pause
popd
exit /B %RV%

rem -------------------------------------------------------------------------------------
:CMAKE_OK
:: start "WFreeCAD" FreeCAD.sln
popd
