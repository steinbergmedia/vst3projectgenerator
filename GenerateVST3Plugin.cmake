cmake_minimum_required(VERSION 3.14.0)

set(SMTG_CMAKE_SCRIPT_DIR ${CMAKE_CURRENT_LIST_DIR})
if(SMTG_CMAKE_SCRIPT_DIR_CLI)
    set(SMTG_CMAKE_SCRIPT_DIR ${SMTG_CMAKE_SCRIPT_DIR_CLI})
endif()

list(APPEND CMAKE_MODULE_PATH ${SMTG_CMAKE_SCRIPT_DIR}/cmake/modules)
set(SMTG_TEMPLATE_FILES_PATH cmake/templates)

include(SMTG_PrintGeneratorCopyrightHeader)
include(SMTG_GeneratePluginUuids)
include(SMTG_GeneratorSpecifics)
include(SMTG_VendorSpecifics)

smtg_print_generator_copyright_header()
message(STATUS "SMTG_CMAKE_SCRIPT_DIR           : ${SMTG_CMAKE_SCRIPT_DIR}")
smtg_generate_plugin_uuids()
smtg_print_generator_specifics()
smtg_print_vendor_specifics()
smtg_print_plugin_uuids()

# Collect all template files
file(GLOB_RECURSE 
    template_files 
    RELATIVE 
    ${SMTG_CMAKE_SCRIPT_DIR}
    *.in
)

foreach(input_file ${template_files})
    # Replace "plugin" by SMTG_PREFIX_FOR_FILENAMES
    string(REPLACE
        "vst3plugin"
        ${SMTG_PREFIX_FOR_FILENAMES}
        output_file
        ${input_file}
    )

    # Replace SMTG_TEMPLATE_FILES_PATH by SMTG_CMAKE_SCRIPT_DIR
    string(REPLACE
        ${SMTG_TEMPLATE_FILES_PATH}
        ${SMTG_GENERATOR_OUTPUT_DIRECTORY}
        output_file
        ${output_file}
    )
    
    # Get last extension, in this case ".in"
    get_filename_component(
        TEMPLATE_EXT
        ${output_file}
        LAST_EXT
    )
   
    # Remove ".in"
    string(REPLACE
        ${TEMPLATE_EXT}
        ""
        output_file
        ${output_file}
    )

    # Write file to HD
    configure_file(
        ${input_file}
        ${output_file}
        @ONLY
        LF
    )

    message(STATUS "${output_file}")
endforeach()
