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

set(VSTGUI_DIR "${vstgui_SOURCE_DIR}/vstgui" CACHE PATH "VSTGUI Path")
message(STATUS VSTGUI_DIR=${VSTGUI_DIR})
set(PROJECTGENERATOR_SCRIPT_DIR "../script" CACHE PATH "Project Generator Script Path")

list(APPEND CMAKE_MODULE_PATH "${VSTGUI_DIR}/cmake/modules")
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
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    XCODE_ATTRIBUTE_CODE_SIGN_INJECT_BASE_ENTITLEMENTS $<$<CONFIG:Debug>:YES>$<$<CONFIG:Release>:NO>
    XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS $<$<CONFIG:Debug>:>$<$<CONFIG:Release>:--timestamp>
    XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD_64_BIT)"
    XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "$<$<CONFIG:Debug>:YES>$<$<CONFIG:Release>:NO>"
    XCODE_GENERATE_SCHEME YES
	XCODE_MACOSX_DEPLOYMENT_TARGET 10.12
  )
  #make sure the vstgui libraries are build as universal binary
  set_target_properties(vstgui PROPERTIES
    XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD_64_BIT)"
    XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "$<$<CONFIG:Debug>:YES>$<$<CONFIG:Release>:NO>"
	XCODE_MACOSX_DEPLOYMENT_TARGET 10.12
  )
  set_target_properties(vstgui_uidescription PROPERTIES
    XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD_64_BIT)"
    XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "$<$<CONFIG:Debug>:YES>$<$<CONFIG:Release>:NO>"
	XCODE_MACOSX_DEPLOYMENT_TARGET 10.12
  )
  set_target_properties(vstgui_standalone PROPERTIES
    XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD_64_BIT)"
    XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "$<$<CONFIG:Debug>:YES>$<$<CONFIG:Release>:NO>"
	XCODE_MACOSX_DEPLOYMENT_TARGET 10.12
  )
  
elseif(MSVC)
  target_link_options(${target}
    PRIVATE /MAP /DEBUG
  )
endif()
