cmake_minimum_required(VERSION 3.14.0)

macro(smtg_make_uuid SHA1_UUID PREFIX)
    string(SUBSTRING ${SHA1_UUID} 0 8  SMTG_${PREFIX}_UUID_PART_0)
    string(SUBSTRING ${SHA1_UUID} 8 8  SMTG_${PREFIX}_UUID_PART_1)
    string(SUBSTRING ${SHA1_UUID} 16 8 SMTG_${PREFIX}_UUID_PART_2)
    string(SUBSTRING ${SHA1_UUID} 24 8 SMTG_${PREFIX}_UUID_PART_3)

    set(SMTG_${PREFIX}_UUID "0x${SMTG_${PREFIX}_UUID_PART_0}, 0x${SMTG_${PREFIX}_UUID_PART_1}, 0x${SMTG_${PREFIX}_UUID_PART_2}, 0x${SMTG_${PREFIX}_UUID_PART_3}")
    set(SMTG_${PREFIX}_PLAIN_UUID "${SMTG_${PREFIX}_UUID_PART_0}${SMTG_${PREFIX}_UUID_PART_1}${SMTG_${PREFIX}_UUID_PART_2}${SMTG_${PREFIX}_UUID_PART_3}")
endmacro(smtg_make_uuid)

macro(smtg_generate_uuid PREFIX)
    string(RANDOM SMTG_RAND_NUMBER)

    # UUID DNS namespace generated at Steinberg
    set(UUID_DNS_NAMESPACE ed66da11-5014-4e8a-876d-829007337274)
    string(
        UUID SHA1_UUID
        NAMESPACE ${UUID_DNS_NAMESPACE}
        NAME ${SMTG_RAND_NUMBER}
        TYPE SHA1 UPPER
    )

    # Remove all hyphens
    string(
        REPLACE "-"
        ""
        SHA1_UUID
        ${SHA1_UUID}
    )
    smtg_make_uuid(${SHA1_UUID} ${PREFIX})
endmacro(smtg_generate_uuid)
