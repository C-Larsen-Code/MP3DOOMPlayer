idf_component_register(
SRCS
  pac.c
  pac0.c
  pac1.c
  pac2.c
INCLUDE_DIRS
  .
)
# PRIV_REQUIRES driver
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
