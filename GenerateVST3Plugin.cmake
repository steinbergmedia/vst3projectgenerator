cmake_minimum_required(VERSION 3.14.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(SMTG_TEMPLATE_FILES_PATH cmake/templates)
include(SMTG_PrintGeneratorCopyrightHeader)
include(SMTG_GeneratePluginUuids)
include(SMTG_VendorSpecifics)

smtg_print_generator_copyright_header()
smtg_generate_plugin_uuids()

smtg_print_vendor_specifics()
smtg_print_plugin_uuids()

# Collect all template files
file(GLOB_RECURSE 
    template_files 
    RELATIVE 
    ${CMAKE_SOURCE_DIR}
    *.in
)

foreach(input_file ${template_files})
    # Replace "plugin" by SMTG_PREFIX_FOR_FILENAMES
    string(REPLACE
        "plugin"
        ${SMTG_PREFIX_FOR_FILENAMES}
        output_file
        ${input_file}
    )

    # Replace SMTG_TEMPLATE_FILES_PATH by CMAKE_SOURCE_DIR
    string(REPLACE
        ${SMTG_TEMPLATE_FILES_PATH}
        ${CMAKE_SOURCE_DIR}
        output_file
        ${input_file}
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

    message("${output_file}")
endforeach()
