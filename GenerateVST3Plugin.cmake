cmake_minimum_required(VERSION 3.14.0)

set(SMTG_CMAKE_SCRIPT_DIR ${CMAKE_CURRENT_LIST_DIR})
if(SMTG_CMAKE_SCRIPT_DIR_CLI)
    string(REPLACE "\"" "" SMTG_CMAKE_SCRIPT_DIR ${SMTG_CMAKE_SCRIPT_DIR_CLI})
endif()

list(APPEND CMAKE_MODULE_PATH ${SMTG_CMAKE_SCRIPT_DIR}/cmake/modules)

include(SMTG_SystemCheck)
include(SMTG_PrintGeneratorCopyrightHeader)
include(SMTG_GeneratePluginUuids)
include(SMTG_GeneratorSpecifics)
include(SMTG_VendorSpecifics)

smtg_print_generator_copyright_header()
smtg_check_system()
message(STATUS "SMTG_CMAKE_SCRIPT_DIR           : ${SMTG_CMAKE_SCRIPT_DIR}")
smtg_generate_plugin_uuids()
smtg_print_generator_specifics()
smtg_print_vendor_specifics()
smtg_print_plugin_uuids()

# Collect all template files
file(GLOB_RECURSE 
    template_files 
    RELATIVE 
        ${SMTG_TEMPLATE_FILES_PATH}
    ${SMTG_TEMPLATE_FILES_PATH}/*.in
)

foreach(rel_input_file ${template_files})
    # Set the plug-in folder name which should be the plug-in's name
    string(REPLACE
        "vst3plugin_folder"
        ${SMTG_PLUGIN_NAME}
        rel_output_file
        ${rel_input_file}
    )

    # Set the plug-in's file prefix
    if(NOT SMTG_PREFIX_FOR_FILENAMES)
        string(REPLACE
            "vst3plugin"
            ""
            rel_output_file
            ${rel_output_file}
        )
    else()
        string(REPLACE
            "vst3plugin"
            ${SMTG_PREFIX_FOR_FILENAMES}
            rel_output_file
            ${rel_output_file}
        )
    endif()
    
    # Get last extension, in this case ".in"
    get_filename_component(
        TEMPLATE_EXT
        ${rel_output_file}
        LAST_EXT
    )
   
    # Remove ".in"
    string(REPLACE
        ${TEMPLATE_EXT}
        ""
        rel_output_file
        ${rel_output_file}
    )

    # Create absolute paths from relative paths
    set(abs_input_file ${SMTG_TEMPLATE_FILES_PATH}/${rel_input_file})
    set(abs_output_file ${SMTG_GENERATOR_OUTPUT_DIRECTORY}/${rel_output_file})

    # Write file to HD
    configure_file(
        ${abs_input_file}
        ${abs_output_file}
        @ONLY
        LF
    )

    message(STATUS "${abs_output_file}")
endforeach()
