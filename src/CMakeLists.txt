set(TARGET qcwdocking)

add_library(${TARGET} SHARED)
add_library(QCWDOCK::${TARGET} ALIAS ${TARGET})

set_target_properties(${TARGET} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON # -fPIC
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME qcwdocking
)

target_sources(${TARGET}
    PRIVATE
        QcwDefaultTabFolderRenderer.cpp
        QcwFastViewStyleSettings.cpp
        QcwFastViewToolBar.cpp
        QcwFilterableListWidget.cpp
        QcwFloatingFolderMonitor.cpp
        QcwTabFolder.cpp
        QcwTabFolderContainer.cpp
        QcwTabFolderRenderer.cpp
        QcwTabFolderRendererCreators.cpp
        QcwTabFolderSite.cpp
        QcwTabPaneManager.cpp
)

target_sources(${TARGET}
    PUBLIC
    FILE_SET headers TYPE HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            QcwDefaultTabFolderRenderer.h
            QcwDockingGlobal.h
            QcwDockingTypes.h
            QcwDockSerializeUtils.h
            QcwFastViewStyleSettings.h
            QcwFastViewToolBar.h
            QcwFilterableListWidget.h
            QcwFloatingFolderMonitor.h
            QcwTabFolder.h
            QcwTabFolderContainer.h
            QcwTabFolderRenderer.h
            QcwTabFolderRendererCreators.h
            QcwTabFolderSite.h
            QcwTabPaneManager.h
)

target_link_libraries(${TARGET}
    PUBLIC Qt::Widgets
)

target_include_directories(${TARGET} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qcwdocking>
)

target_compile_definitions(${TARGET} PRIVATE QCWDOCK_LIBRARY QT_DEPRECATED_WARNINGS)

install(TARGETS ${TARGET}
    EXPORT qcwdockingTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILE_SET headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qcwdocking
)

include(CMakePackageConfigHelpers)

set(QCWDOCKING_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/qcwdocking")

install(EXPORT qcwdockingTargets
    FILE qcwdockingTargets.cmake
    NAMESPACE QCWDOCK::
    DESTINATION "${QCWDOCKING_CMAKE_INSTALL_DIR}"
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/qcwdockingConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/qcwdockingConfig.cmake"
    INSTALL_DESTINATION "${QCWDOCKING_CMAKE_INSTALL_DIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/qcwdockingConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/qcwdockingConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/qcwdockingConfigVersion.cmake"
    DESTINATION "${QCWDOCKING_CMAKE_INSTALL_DIR}"
)
