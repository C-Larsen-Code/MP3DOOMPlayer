set(COMPS config lcd pin joy c24k_8b)
if(EXISTS ../../components/tone/tone.c)
    set(MILESTONE 2)
    list(APPEND COMPS tone)
else()
    set(MILESTONE 1)
endif()
idf_component_register(SRCS main.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES ${COMPS})
target_compile_options(${COMPONENT_LIB} PRIVATE -DMILESTONE=${MILESTONE})
