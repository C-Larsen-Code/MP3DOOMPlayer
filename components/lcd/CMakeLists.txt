idf_component_register(SRCS lcd.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES driver
                       REQUIRES config)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
