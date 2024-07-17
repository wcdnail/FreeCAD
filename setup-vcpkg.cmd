@echo off

:: SET curDir=%~dp0

pushd %VCPKG_ROOT%
echo.
echo Using VCPKG root directory: %VCPKG_ROOT%

if NOT exist vcpkg.exe (
    echo.
    call bootstrap-vcpkg.bat -disableMetrics
)

:: "qt3d[vulkan]" ^

echo.
vcpkg install --disable-metrics ^
qtbase[gui,opengl,widgets] ^
vtk[qt,opengl,python,utf8] ^
yaml-cpp ^
xerces-c ^
opencascade[vtk,freetype,freeimage,rapidjson] ^
salome-medcoupling ^
boost-program-options ^
qt5compat ^
coin[simage,zlib,bzip2,freetype] ^
boost-python[python3] ^
boost-signals2 ^
boost-iostreams ^
boost-format ^
boost-uuid ^
boost-polygon ^
boost-interprocess ^
boost-statechart ^
boost-convert ^
boost-geometry ^
boost-assign ^
ms-gsl ^
netgen[occ] ^
--recurse

IF '%ERRORLEVEL%'=='0' GOTO VCPKG_OK
set RV=%ERRORLEVEL%
echo ERRORLEVEL: %RV%
pause
popd
exit /B %RV%

:VCPKG_OK
popd
