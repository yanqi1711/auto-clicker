cmake_minimum_required(VERSION 3.16)

project(example VERSION 1.0 LANGUAGES CXX)

# Standard Qt 6 settings
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the packages directly
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml QuickControls2)

# Recommended Qt 6 project setup
qt_standard_project_setup()

# Define your source files manually
set(SOURCE_FILES
        src/main.cpp
)

# Define your resources
set(RESOURCE_FILES
        example.qrc
)

# If you want to keep the icon, keep this line. If not, delete it.
set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/public/Icon.ico"
        src/Theme.cpp
        src/Theme.hpp)

# Create the executable
add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
        ${SOURCE_FILES}
        ${RESOURCE_FILES}
        ${ICON_FILE} # This adds the icon to the EXE
)

# Link everything
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Qml
        Qt6::QuickControls2

)

# Simple install logic
install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION /Applications)