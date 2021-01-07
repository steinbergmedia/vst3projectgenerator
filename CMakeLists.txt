cmake_minimum_required(VERSION 3.12)

include(FetchContent)

project(vst3_project_generator_distribution)

# Clone vstgui
FetchContent_Declare(
  vstgui
  GIT_REPOSITORY https://github.com/steinbergmedia/vstgui.git
  GIT_TAG        origin/master
  GIT_SHALLOW    OFF
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

FetchContent_MakeAvailable(vstgui)

add_subdirectory(app)
