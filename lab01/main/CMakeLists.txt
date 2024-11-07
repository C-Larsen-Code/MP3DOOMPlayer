idf_component_register(SRCS main.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES lcd mono)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
