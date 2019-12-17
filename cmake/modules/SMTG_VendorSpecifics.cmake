cmake_minimum_required(VERSION 3.14.0)

string(TIMESTAMP SMTG_CURRENT_YEAR %Y)

set(SMTG_VENDOR_NAME "My Company Name")
set(SMTG_VENDOR_HOMEPAGE "https://www.mycompanyname.com")
set(SMTG_VENDOR_EMAIL "info@mycompanyname.com")
set(SMTG_SOURCE_COPYRIGHT_HEADER "Copyright(c) ${SMTG_CURRENT_YEAR} ${SMTG_VENDOR_NAME}.")
set(SMTG_PLUGIN_NAME "My Plugin")

# Source code specifics
set(SMTG_VENDOR_NAMESPACE "MyCompanyName")
set(SMTG_PLUGIN_CLASS_NAME "MyPlugin")
set(SMTG_PREFIX_FOR_FILENAMES "myplugin")
set(SMTG_PLUGIN_BUNDLE_NAME "MyPlugin")
set(SMTG_CMAKE_PROJECT_NAME ${SMTG_PLUGIN_BUNDLE_NAME})

function(smtg_print_vendor_specifics)
    message(STATUS "SMTG_VENDOR_NAME            : ${SMTG_VENDOR_NAME}")
    message(STATUS "SMTG_VENDOR_HOMEPAGE        : ${SMTG_VENDOR_HOMEPAGE}")
    message(STATUS "SMTG_VENDOR_EMAIL           : ${SMTG_VENDOR_EMAIL}")
    message(STATUS "SMTG_SOURCE_COPYRIGHT_HEADER: ${SMTG_SOURCE_COPYRIGHT_HEADER}")
    message(STATUS "SMTG_PLUGIN_NAME            : ${SMTG_PLUGIN_NAME}")
    message(STATUS "SMTG_PREFIX_FOR_FILENAMES   : ${SMTG_PREFIX_FOR_FILENAMES}")
    message("")
    message(STATUS "SMTG_VENDOR_NAMESPACE       : e.g. namespace ${SMTG_VENDOR_NAMESPACE} {...}")
    message(STATUS "SMTG_PLUGIN_CLASS_NAME      : e.g. class ${SMTG_PLUGIN_CLASS_NAME} : public AudioEffect {...}")
    message("")
endfunction()