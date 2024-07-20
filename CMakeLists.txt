# As of January 2023 we require CMake 3.16.3
# see https://forum.freecad.org/viewtopic.php?f=10&t=72173
# for further info why
cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

if (VCPKG_ROOT AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "VCPKG toolchain file")
endif()

# policy CMP0072 was introduced with CMake 3.11
# relates to FindOpenGL module
# and cache variables OPENGL_gl_LIBRARY, OPENGL_glu_LIBRARY
if (POLICY CMP0072)
    set(OpenGL_GL_PREFERENCE LEGACY)
endif(POLICY CMP0072)

# FindPythonInterp and FindPythonLibs modules are deprecated.
# Currently, they are still used by shiboken's CMake files
if (POLICY CMP0148)
    cmake_policy(SET CMP0148 OLD)
endif()

option(FREECAD_USE_CCACHE "Auto detect and use ccache during compilation" ON)

if(FREECAD_USE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)  #This check should occur before project()
    if(CCACHE_PROGRAM)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    endif()
endif()

project(FreeCAD)

if (WIN32 AND NOT VCPKG_ROOT)
    message(STATUS ">>>> MINGW build ${MINGW}")
endif()

set(VCPKG_APPLOCAL_DEPS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(
    -DQT_NO_DEPRECATED_WARNINGS
   #-D_GLIBCXX_USE_CXX11_ABI=0
)

if (VCPKG_ROOT)
    if ("${VCPKG_TARGET_TRIPLET}" STREQUAL "x64-")
        if (MSVS)
            set(VCPKG_TARGET_TRIPLET x64-windows)
        elseif (MSYS OR MINGW)
            set(VCPKG_TARGET_TRIPLET x64-msys-clang)
        else()
            set(VCPKG_TARGET_TRIPLET x64-linux)
        endif()
        message(STATUS "VCPKG triplet setup from CMAKE: ${VCPKG_TARGET_TRIPLET}")
    endif()
    set(VCPKG_INSTALLED_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(VCPKG_INSTALLED_CONF_DIR "${VCPKG_INSTALLED_DIR}/debug")
    else()
        set(VCPKG_INSTALLED_CONF_DIR "${VCPKG_INSTALLED_DIR}")
    endif()
    message(STATUS "VCPKG installed dir: ${VCPKG_INSTALLED_DIR}")
    message(STATUS "VCPKG installed conf dir: ${VCPKG_INSTALLED_CONF_DIR}")
    include_directories(
        ${VCPKG_INSTALLED_DIR}/include
        ${VCPKG_INSTALLED_DIR}/include/Qt6/QtCore5Compat
    )
endif()

#set(FC_BUILD_DEBUG_ODIR ${CMAKE_CURRENT_BINARY_DIR}/${VCPKG_TARGET_TRIPLET}-debug)
#set(FC_BUILD_RELEASE_ODIR ${CMAKE_CURRENT_BINARY_DIR}/${VCPKG_TARGET_TRIPLET}-release)

set(PACKAGE_VERSION_NAME "Vulcan")
set(PACKAGE_VERSION_MAJOR "0")
set(PACKAGE_VERSION_MINOR "22")
set(PACKAGE_VERSION_PATCH "0") # number of patch release (e.g. "4" for the 0.18.4 release)
set(PACKAGE_VERSION_SUFFIX "dev") # either "dev" for development snapshot or "" (empty string)
set(PACKAGE_BUILD_VERSION "0") # used when the same FreeCAD version will be re-released (for example using an updated LibPack)

set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}")
set(PACKAGE_STRING "${PROJECT_NAME} ${PACKAGE_VERSION}")

# include local modules
include(CheckCXXCompilerFlag)
include(AddFileDependencies)
include(cMake/FreeCadMacros.cmake)
# include helper functions/macros
add_subdirectory(cMake/FreeCAD_Helpers)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cMake")

CompilerChecksAndSetups()
ConfigureCMakeVariables()
InitializeFreeCADBuildOptions()
CheckInterModuleDependencies()
FreeCADLibpackChecks()
SetupDoxygen()
SetupLibFmt()
SetupYamlCpp()
if(NOT FREECAD_LIBPACK_USE OR FREECAD_LIBPACK_CHECKFILE_CLBUNDLER OR FREECAD_LIBPACK_CHECKFILE_VERSION)
    SetupPython()
    SetupPCL()
    SetupPybind11()
    SetupXercesC()
    find_package(ZLIB REQUIRED)
    find_package(PyCXX REQUIRED)
    SetupOpenCasCade()
    if(BUILD_GUI)
        # Do this before the check for SMESH because it depends on vtk
        # that may have its own OpenGL check but possibly fails and leaves
        # OPENGL_gl_LIBRARY empty that results into linker errors
        SetupOpenGL()
    endif(BUILD_GUI)
    SetupBoost()
    SetupSalomeSMESH()
    if (BUILD_FEM_NETGEN)
        find_package(NETGEN)
    endif(BUILD_FEM_NETGEN)
    # not needed at the moment
    #find_package(OpenCV REQUIRED)
    SetupSwig()
    SetupEigen()
    # This file is included directly due to some cMake macros that are defined within.
    # If these macro definitions are embedded within another macro, it causes problems.
    include(cMake/FreeCAD_Helpers/SetupQt.cmake)
    SetupFreetype()

    if(BUILD_GUI)
        SetupCoin3D()
        SetupSpaceball()
        SetupShibokenAndPyside()
        SetupMatplotlib()
    endif(BUILD_GUI)
endif()

if(BUILD_VR)
    find_package(Rift)
endif(BUILD_VR)

SetLibraryVersions()
SetGlobalCompilerAndLinkerSettings()

add_subdirectory(src)
add_subdirectory(data)

BuildAndInstallDesignerPlugin()

CreatePackagingTargets()

if(MSVC AND FREECAD_LIBPACK_USE AND LIBPACK_FOUND)
    CopyLibpackDirectories()
endif()

if (ENABLE_DEVELOPER_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

set(ROOT_FILES
    ${CMAKE_SOURCE_DIR}/README.md
    ${CMAKE_SOURCE_DIR}/PRIVACY_POLICY.md
    ${CMAKE_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_SOURCE_DIR}/cMake/ConfigureChecks.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindCoin3DDoc.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindEigen3.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindKDL.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindMatplotlib.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindMEDFile.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindNETGEN.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindOCC.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindOpenCV.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindPyCXX.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindPySide2Tools.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindPySide6.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindPySide6Tools.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindRift.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindShiboken6.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindSMESH.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FindSpnav.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCadMacros.cmake
    ${CMAKE_SOURCE_DIR}/cMake/UseLibPack3.cmake
    ${CMAKE_SOURCE_DIR}/cMake/UseLibPackCLbundler.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/BuildAndInstallDesignerPlugin.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/CheckInterModuleDependencies.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/ChooseQtVersion.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/CMakeLists.txt
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/CompilerChecksAndSetups.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/ConfigureCMakeVariables.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/CopyLibpackDirectories.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/CreatePackagingTargets.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/FreeCADLibpackChecks.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/InitializeFreeCADBuildOptions.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/PrintFinalReport.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/README.md
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetGlobalCompilerAndLinkerSettings.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetLibraryVersions.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupBoost.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupCoin3D.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupDoxygen.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupEigen.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupFreeType.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupLibFmt.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupLibOndselSolver.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupLibYaml.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupMatplotlib.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupOpenCasCade.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupOpenGL.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupPCL.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupPybind11.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupPython.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupQt.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupSalomeSMESH.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupShibokenAndPyside.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupSpaceball.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupSwig.cmake
    ${CMAKE_SOURCE_DIR}/cMake/FreeCAD_Helpers/SetupXercesC.cmake
    $<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/setup-vcpkg.cmd>
    $<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/mk-msvs-solution.cmd>
    $<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/vs.base.props>
)

add_custom_target(${PROJECT_NAME}_Root SOURCES ${ROOT_FILES})
set_target_properties(${PROJECT_NAME}_Root PROPERTIES FOLDER "A_FC")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ROOT_FILES})

PrintFinalReport()

message("\n=================================================\n"
        "Now run 'cmake --build ${CMAKE_BINARY_DIR}' to build ${PROJECT_NAME}\n"
        "=================================================\n")

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FreeCADMain)
endif()
