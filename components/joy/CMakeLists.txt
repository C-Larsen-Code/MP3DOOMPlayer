idf_component_register(SRCS joy.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES esp_adc config)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
