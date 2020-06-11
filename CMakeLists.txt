##########################################################################################
cmake_minimum_required(VERSION 3.12)
project(vst3_project_generator)

	if(CMAKE_CONFIGURATION_TYPES)
		set(CMAKE_CONFIGURATION_TYPES Debug Release)
		set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	endif()

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/Debug/>$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Release/>$<$<CONFIG:ReleaseLTO>:${CMAKE_BINARY_DIR}/ReleaseLTO/>
    )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/Debug/libs/>$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Release/libs/>$<$<CONFIG:ReleaseLTO>:${CMAKE_BINARY_DIR}/ReleaseLTO/libs/>
    )
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/Debug/libs>$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Release/libs>$<$<CONFIG:ReleaseLTO>:${CMAKE_BINARY_DIR}/ReleaseLTO/libs>
    )

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)

set(VSTGUI_DISABLE_UNITTESTS ON)
set(VSTGUI_STANDALONE_EXAMPLES OFF)
set(VSTGUI_STANDALONE ON)
set(VSTGUI_TOOLS OFF CACHE BOOL "VSTGUI TOOLS")

set(VSTGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../vstgui/vstgui" CACHE PATH "VSTGUI Path")
set(PROJECTGENERATOR_SCRIPT_DIR "../vst3_project_generator_script" CACHE PATH "Project Generator Script Path")
add_subdirectory(${VSTGUI_DIR} ${CMAKE_CURRENT_BINARY_DIR}/VSTGUI.build)

list(APPEND CMAKE_MODULE_PATH "${VSTGUI_DIR}/standalone/cmake/modules")

set(target VST3_Project_Generator)

set(${target}_sources
  "source/application.cpp"
  "source/application.h"
  "source/cmakecapabilities.cpp"
  "source/cmakecapabilities.h"
  "source/controller.cpp"
  "source/controller.h"
  "source/process.h"
  "source/dimmviewcontroller.cpp"
  "source/dimmviewcontroller.h"
  "source/linkcontroller.cpp"
  "source/linkcontroller.h"
  "source/valuelistenerviewcontroller.h"
  "source/scriptscrollviewcontroller.cpp"
  "source/scriptscrollviewcontroller.h"
  "source/version_buildnumber.h"
  "source/version.h"
)

set(${target}_resources
  "resource/Window.uidesc"
  "resource/links.json"
)

##########################################################################################
## Platform Specific
##########################################################################################
if(CMAKE_HOST_APPLE)
  set(${target}_sources 
    ${${target}_sources}
    "source/platform/mac/process.mm"
    "source/platform/win32"
  )
  set(${target}_resources 
    ${${target}_resources}
    "resource/VST_Logo_Steinberg.icns"
  )
elseif(MSVC)
  set(${target}_sources 
    ${${target}_sources}
    "source/platform/win32/process.cpp"
    "source/platform/mac"
  )
endif()

##########################################################################################
vstgui_add_executable(${target} "${${target}_sources}" )
vstgui_add_resources(${target} "${${target}_resources}")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/win32_resources.rc")
vstgui_set_cxx_version(${target} 14)

target_include_directories(${target} PUBLIC "${VSTGUI_DIR}/..")

## cmake script
vstgui_add_resources(${target} "${PROJECTGENERATOR_SCRIPT_DIR}/GenerateVST3Plugin.cmake")
vstgui_add_resources(${target} "${PROJECTGENERATOR_SCRIPT_DIR}/cmake/modules" cmake/)
vstgui_add_resources(${target} "${PROJECTGENERATOR_SCRIPT_DIR}/cmake/templates" cmake/)

## code signing macOS
if(CMAKE_HOST_APPLE)
  if(SMTG_DEVELOPMENT_TEAMID)
    set_target_properties(${target} PROPERTIES
      XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${SMTG_DEVELOPMENT_TEAMID}
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${SMTG_CODE_SIGN_IDENTITY}"
    )
    message(STATUS "Setup macOS signing with TeamID: ${SMTG_DEVELOPMENT_TEAMID} and Identity: ${SMTG_CODE_SIGN_IDENTITY}")
  endif()
  set_target_properties(${target} PROPERTIES
    XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS YES
    XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET 10.9
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    XCODE_ATTRIBUTE_CODE_SIGN_INJECT_BASE_ENTITLEMENTS $<$<CONFIG:Debug>:YES>$<$<CONFIG:Release>:NO>
    XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS $<$<CONFIG:Debug>:>$<$<CONFIG:Release>:--timestamp>
  )
elseif(MSVC)
  target_link_options(${target}
    PRIVATE /MAP /DEBUG
  )
endif()
