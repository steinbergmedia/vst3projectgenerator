cmake_minimum_required(VERSION 3.14.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
include(SMTG_PrintGeneratorCopyrightHeader)

smtg_print_generator_copyright_header()

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
        output_file_2
        ${output_file}
    )

    configure_file(
        ${input_file}
        ${output_file_2}
        @ONLY
        LF
    )

    message("${output_file_2}")
endforeach()
