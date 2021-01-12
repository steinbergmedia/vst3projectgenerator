cmake_minimum_required(VERSION 3.12)

include(FetchContent)

project(vst3_project_generator_distribution)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)

# Clone vstgui
FetchContent_Declare(
  vstgui
  GIT_REPOSITORY https://github.com/steinbergmedia/vstgui.git
  GIT_TAG        origin/develop
  GIT_SHALLOW    ON
)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    macro(FetchContent_MakeAvailable NAME)
        FetchContent_GetProperties(${NAME})
        if(NOT ${NAME}_POPULATED)
            FetchContent_Populate(${NAME})
            add_subdirectory(${${NAME}_SOURCE_DIR} ${${NAME}_BINARY_DIR})
        endif()
    endmacro()
endif()

set(VSTGUI_DISABLE_UNITTESTS ON)
set(VSTGUI_STANDALONE_EXAMPLES OFF)
set(VSTGUI_STANDALONE ON)
set(VSTGUI_TOOLS OFF)

FetchContent_MakeAvailable(vstgui)

add_subdirectory(app)
