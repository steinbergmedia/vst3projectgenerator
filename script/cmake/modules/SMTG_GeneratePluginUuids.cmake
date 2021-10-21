cmake_minimum_required(VERSION 3.14.0)

include(SMTG_GenerateUuid)

macro(smtg_generate_plugin_uuids)
    smtg_generate_uuid(Processor)   # -> SMTG_Processor_UUID
    smtg_generate_uuid(Controller)  # -> SMTG_Controller_UUID
endmacro(smtg_generate_plugin_uuids)

macro(smtg_print_plugin_uuids)
    message(STATUS "SMTG_Processor_UUID         : ${SMTG_Processor_UUID}")
    message(STATUS "SMTG_Controller_UUID        : ${SMTG_Controller_UUID}")
    message("")
endmacro(smtg_print_plugin_uuids)
