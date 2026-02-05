cmake_minimum_required(VERSION 3.16)

project(AutoClicker VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml QuickControls2)

qt_standard_project_setup()

set(SOURCE_FILES
        src/main.cpp
        src/Theme.cpp
        src/Theme.hpp
        src/Config.cpp
        src/Config.hpp
)

set(RESOURCE_FILES
        example.qrc
)

set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/public/Icon.ico"
        public/version.rc)

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${SOURCE_FILES}
        ${RESOURCE_FILES}
        ${ICON_FILE}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Qml
        Qt6::QuickControls2
)

if(WIN32)
    find_package(Qt6 REQUIRED COMPONENTS Core)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    set(WINDEPLOYQT_EXECUTABLE "${_qt_bin_dir}/windeployqt.exe")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${WINDEPLOYQT_EXECUTABLE}" 
            "$<TARGET_FILE:${PROJECT_NAME}>" 
            --qmldir "${CMAKE_CURRENT_SOURCE_DIR}"
            --no-translations 
            --compiler-runtime
            --force
    COMMENT "Executing windeployqt with QML support..."
)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/config" DESTINATION .)

set(QT_PLUGINS_DIRS "qml" "platforms" "styles" "iconengines" "imageformats" "tls")

foreach(DIRNAME ${QT_PLUGINS_DIRS})
    if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${DIRNAME}")
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${DIRNAME}" DESTINATION .)
    else()
        message(STATUS "Skipping optional directory: ${DIRNAME} (not found)")
    endif()
endforeach()

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
    DESTINATION .
    FILES_MATCHING 
    PATTERN "*.dll"
    PATTERN "qt.conf"
    EXCLUDE PATTERN "*/"
    EXCLUDE PATTERN "_CPack_Packages/*"
    EXCLUDE PATTERN "CMakeFiles/*"
)

set(CPACK_PACKAGE_NAME "AutoClicker")
set(CPACK_PACKAGE_VENDOR "AutoClickerStudio")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High Performance Auto Clicker")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "AutoClicker")

set(CPACK_GENERATOR "WIX")

set(CPACK_WIX_PROPERTY_ARPPRODUCTICON "${CMAKE_CURRENT_SOURCE_DIR}/public/Icon.ico")
set(CPACK_WIX_UPGRADE_GUID "A1B2C3D4-E5F6-4789-B0C1-D2E3F4A5B6C7")
set(CPACK_WIX_PROGRAM_MENU_FOLDER "AutoClicker")

include(CPack)