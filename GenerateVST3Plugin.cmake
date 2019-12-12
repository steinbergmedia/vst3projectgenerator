cmake_minimum_required(VERSION 3.14.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
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
    string(REPLACE
        cmake/templates
        ${CMAKE_SOURCE_DIR}
        output_file
        ${input_file}
    )

    get_filename_component(
        TEMPLATE_EXT
        ${output_file}
        LAST_EXT
    )
   # message ("${TEMPLATE_EXT}")
    string(REPLACE
        ${TEMPLATE_EXT}
        ""
        output_file
        ${output_file}
    )

    string(REPLACE
        "plugin"
        ${SMTG_PREFIX_FOR_FILENAMES}
        output_file
        ${output_file}
    )

    configure_file(
        ${input_file}
        ${output_file}
        @ONLY
        LF
    )

    message("${output_file}")
endforeach()
