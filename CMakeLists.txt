cmake_minimum_required(VERSION 3.16)

project(example VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml QuickControls2)

qt_standard_project_setup()

set(SOURCE_FILES
        src/main.cpp
)

set(RESOURCE_FILES
        example.qrc
)

set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/public/Icon.ico"
        src/Theme.cpp
        src/Theme.hpp)

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

install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION /Applications)