@echo off

rem Var section -------------------------------------------------------------------------

rem VCPKG_ROOT must be set in system
rem set VCPKG_ROOT=c:\_\wcpkg

set COMPUTER_NAME=%ComputerName%

set TMP_NOWDATE_FN=%CD%\PS_TS_NOW_DAAATE27.TEMPTS
PowerShell -Command "get-date" -format "yyyy-MM-dd" > %TMP_NOWDATE_FN%
set /P NOW_DATE=< %TMP_NOWDATE_FN%
del %TMP_NOWDATE_FN% 2>NUL

set SOURCE_DIR=%CD%
set BUILD_DIR=%SOURCE_DIR%\bld

set MSVC_ROOT=c:\Program Files\Microsoft Visual Studio\2022\Community
set MSVC_TOOLS_ROOT=%MSVC_ROOT%\VC\Tools
set MSVC_KIT_ROOT=c:\Program Files (x86)\Windows Kits

rem CRT sources here
set MSVC_TOOLS_ROOT_DIR=%MSVC_TOOLS_ROOT%\MSVC\14.39.33519

set MSVC_CMAKE_GENERATOR="Visual Studio 17 2022"
set MSVC_C_COMPILER=%MSVC_TOOLS_ROOT%\MSVC\14.39.33519\bin\Hostx64\x64\cl.exe
set MSVC_CXX_COMPILER=%MSVC_C_COMPILER%

set MSVC_ARM_C_COMPILER=%MSVC_TOOLS_ROOT%\MSVC\14.28.29333\bin\Hostx64\arm\cl.exe
set MSVC_ARM_CXX_COMPILER=%MSVC_ARM_C_COMPILER%
set MSVC_ARM_ASM_COMPILER=%MSVC_ARM_C_COMPILER%

set CLANG_CMAKE_GENERATOR="NMake Makefiles"
set CLANG_C_COMPILER=%MSVC_TOOLS_ROOT%\Llvm\x64\bin\clang.exe
set CLANG_CXX_COMPILER=%MSVC_TOOLS_ROOT%\Llvm\x64\bin\clang++.exe

set MSVC_RC_COMPILER=%MSVC_KIT_ROOT%\10\bin\10.0.22621.0\x64\rc.exe

set PATH=%PATH%;"%MSVC_TOOLS_ROOT%\MSVC\14.39.33519\bin\Hostx64\x64"
set PATH=%PATH%;%VCPKG_ROOT%\downloads\tools\ninja\1.10.2-windows
set PATH=%PATH%;%VCPKG_ROOT%\installed\x64-windows\tools\openssl

if "%BUILD_CONF%" == "Debug" goto :DebugConf

set RELEASE_ENV=1
set PATH=%PATH%;%VCPKG_ROOT%\installed\x64-windows\bin
goto :ConfDone

:DebugConf
set RELEASE_ENV=0
set PATH=%PATH%;%VCPKG_ROOT%\installed\x64-windows\debug\bin

:ConfDone
