
configure_file(freecad.rc.cmake ${CMAKE_CURRENT_BINARY_DIR}/freecad.rc)
configure_file(freecadCmd.rc.cmake ${CMAKE_CURRENT_BINARY_DIR}/freecadCmd.rc)
file(COPY icon.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(VCPKG_APPLOCAL_DEPS OFF)

if(BUILD_GUI)
    include_directories(
        ${Boost_INCLUDE_DIRS}
        ${COIN3D_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIR}
        ${QtWidgets_INCLUDE_DIRS}
        ${QT_INCLUDE_DIR}
        ${PYTHON_INCLUDE_DIRS}
        ${XercesC_INCLUDE_DIRS}
    )
else(BUILD_GUI)
    include_directories(
        ${Boost_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIR}
        ${QT_INCLUDE_DIR}
        ${PYTHON_INCLUDE_DIRS}
        ${XercesC_INCLUDE_DIRS}
    )
endif(BUILD_GUI)

######################## FreeCADMain ########################
if(BUILD_GUI)

    SET(FreeCAD_SRCS
        ${CMAKE_CURRENT_BINARY_DIR}/freecad.rc
        icon.ico
        MainGui.cpp
    )

    SET(FreeCAD_LIBS
        FreeCADGui
    )

    if(NOT BUILD_DYNAMIC_LINK_PYTHON)
        # executables have to be linked against python libraries,
        # because extension modules are not.
        list(APPEND FreeCAD_LIBS
            ${PYTHON_LIBRARIES}
        )
    endif(NOT BUILD_DYNAMIC_LINK_PYTHON)

    add_executable(FreeCADMain WIN32 ${FreeCAD_SRCS})
    set_target_properties(FreeCADMain PROPERTIES FOLDER "Main")
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
        icon.ico
        MainGui.cpp
    )
    
    target_link_libraries(FreeCADMain ${FreeCAD_LIBS})
    if (FREECAD_WARN_ERROR)
        target_compile_warn_error(FreeCADMain)
    endif()

    SET_BIN_DIR(FreeCADMain FreeCAD)

    if(WIN32)
        INSTALL(TARGETS FreeCADMain
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    elseif(APPLE AND NOT BUILD_WITH_CONDA)
        INSTALL(TARGETS FreeCADMain
        RUNTIME DESTINATION MacOS
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    else()
        INSTALL(TARGETS FreeCADMain
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()

    if (MSVC)
        set(DBG_PATH "PATH=$(PATH);${VCPKG_INSTALLED_CONF_DIR}/bin")
        set(QTP_PATH "QT_QPA_PLATFORM_PLUGIN_PATH=\$(VCPKG_ROOT)\\installed\\x64-windows\\Qt6\\plugins\\platforms")
        set(FC_PYTHONHOME "FC_PYTHONHOME=\$(VCPKG_ROOT)\\installed\\x64-windows\\tools\\python3")
        set_target_properties(FreeCADMain PROPERTIES VS_DEBUGGER_ENVIRONMENT "${FC_PYTHONHOME}\n${QTP_PATH}\n${DBG_PATH}")
        set_target_properties(FreeCADMain PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "\$(SolutionDir)bin\\\$(ConfigurationName)")
        #set_target_properties(FreeCADMain PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS
            # #TODO: DEBUG command line here
        #)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FreeCADMain)
    endif()

endif(BUILD_GUI)
######################## FreeCADMainCmd ########################

SET(FreeCADMainCmd_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/freecadCmd.rc
    icon.ico
    MainCmd.cpp
)

add_executable(FreeCADMainCmd ${FreeCADMainCmd_SRCS})
set_target_properties(FreeCADMainCmd PROPERTIES FOLDER "Main")

SET(FreeCADMainCmd_LIBS
    FreeCADApp
    ${QtCore_LIBRARIES}
    ${QtXml_LIBRARIES}
)

if(NOT BUILD_DYNAMIC_LINK_PYTHON)
    # executables have to be linked against python libraries,
    # because extension modules are not.
    list(APPEND FreeCADMainCmd_LIBS
        ${PYTHON_LIBRARIES}
    )
endif(NOT BUILD_DYNAMIC_LINK_PYTHON)

target_link_libraries(FreeCADMainCmd
    ${FreeCADMainCmd_LIBS}
)
if (FREECAD_WARN_ERROR)
    target_compile_warn_error(FreeCADMainCmd)
endif()

SET_BIN_DIR(FreeCADMainCmd FreeCADCmd)

if(WIN32)
    INSTALL(TARGETS FreeCADMainCmd
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
elseif(APPLE AND NOT BUILD_WITH_CONDA)
    INSTALL(TARGETS FreeCADMainCmd
        RUNTIME DESTINATION MacOS
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
else()
    INSTALL(TARGETS FreeCADMainCmd
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

######################## FreeCADMainPy ########################

SET(FreeCADMainPy_SRCS
    MainPy.cpp
)

add_library(FreeCADMainPy SHARED ${FreeCADMainPy_SRCS})
set_target_properties(FreeCADMainPy PROPERTIES FOLDER "Main")

target_link_libraries(FreeCADMainPy FreeCADApp)
if (FREECAD_WARN_ERROR)
    target_compile_warn_error(FreeCADMainPy)
endif()

SET_BIN_DIR(FreeCADMainPy FreeCAD)
SET_PYTHON_PREFIX_SUFFIX(FreeCADMainPy)

if(WIN32)
    INSTALL(TARGETS FreeCADMainPy
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
else(WIN32)
    INSTALL(TARGETS FreeCADMainPy
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif(WIN32)

######################## FreeCADGuiPy ########################
if(BUILD_GUI)

    SET(FreeCADGuiPy_SRCS
        FreeCADGuiPy.cpp
    )

    add_library(FreeCADGuiPy SHARED ${FreeCADGuiPy_SRCS})
    set_target_properties(FreeCADGuiPy PROPERTIES FOLDER "Main")

    target_link_libraries(FreeCADGuiPy FreeCADGui)
    if (FREECAD_WARN_ERROR)
        target_compile_warn_error(FreeCADGuiPy)
    endif()

    SET_BIN_DIR(FreeCADGuiPy FreeCADGui)
    SET_PYTHON_PREFIX_SUFFIX(FreeCADGuiPy)
    if(WIN32)
        # Name clash with target "FreeCADGui"
        # Must be called "FreeCADGuiPy_d" and "FreeCADGuiPy" to work so override default
        set_target_properties(FreeCADGuiPy PROPERTIES PDB_NAME_DEBUG "FreeCADGuiPy_d")
        set_target_properties(FreeCADGuiPy PROPERTIES PDB_NAME_RELEASE "FreeCADGuiPy")
    endif(WIN32)

    if(WIN32)
        INSTALL(TARGETS FreeCADGuiPy
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    else(WIN32)
        INSTALL(TARGETS FreeCADGuiPy
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif(WIN32)
endif(BUILD_GUI)
