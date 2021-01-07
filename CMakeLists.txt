cmake_minimum_required(VERSION 3.12)

include(FetchContent)

project(vst3_project_generator_distribution)

# Clone vstgui
FetchContent_Declare(
  vstgui
  GIT_REPOSITORY https://stash.steinberg-intra.net/scm/vstsdk/vstgui.git
  GIT_TAG        origin/master
  GIT_SHALLOW    OFF
)
FetchContent_GetProperties(vstgui)
if(NOT vstgui_POPULATED)
  FetchContent_Populate(vstgui)
endif()
set(VSTGUI_DIR "${vstgui_SOURCE_DIR}/vstgui/" CACHE PATH "VSTGUI Path")
message(STATUS VSTGUI_DIR=${VSTGUI_DIR})

add_subdirectory(app)

