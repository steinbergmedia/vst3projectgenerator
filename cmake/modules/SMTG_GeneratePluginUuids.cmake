cmake_minimum_required(VERSION 3.14.0)

include(SMTG_GenerateUuid)

macro(smtg_generate_plugin_uuids)
    smtg_generate_uuid(Processor)
    smtg_generate_uuid(Controller)
endmacro()

macro(smtg_print_plugin_uuids)
    message("Processor UUID: ${SMTG_Processor_UUID}")
    message("Controller UUID: ${SMTG_Controller_UUID}")
endmacro()
