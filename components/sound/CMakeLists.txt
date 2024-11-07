idf_component_register(SRCS sound_one.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES driver config)
if(DEFINED EXTERN_BUF)
    target_compile_options(${COMPONENT_LIB} PRIVATE -DEXTERN_BUF=${EXTERN_BUF})
endif()
