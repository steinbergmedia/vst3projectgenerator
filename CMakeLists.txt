##########################################################################################
cmake_minimum_required(VERSION 3.5)
project(vst3_project_generator)

set(VSTGUI_DISABLE_UNITTESTS YES)
set(VSTGUI_STANDALONE_EXAMPLES NO)
set(VSTGUI_TOOLS NO)

set(VSTGUI_DIR ../vstgui/vstgui)
add_subdirectory(${VSTGUI_DIR} ${CMAKE_CURRENT_BINARY_DIR}/VSTGUI.build)

list(APPEND CMAKE_MODULE_PATH "${VSTGUI_DIR}/standalone/cmake/modules")

set(target VST3_Project_Generator)

set(${target}_sources
  "source/application.cpp"
  "source/application.h"
)

set(${target}_resources
  "resource/Window.uidesc"
)

##########################################################################################
vstgui_add_executable(${target} "${${target}_sources}" )
vstgui_add_resources(${target} "${${target}_resources}")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/win32_resources.rc")
vstgui_set_cxx_version(${target} 14)

target_include_directories(${target} PUBLIC ../vstgui)
