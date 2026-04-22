set(src_dirs LED)
set(include_dirs LED)
set(requires driver)

idf_component_register(
    SRCS "LED/led.c"          
    INCLUDE_DIRS "LED"        
    REQUIRES driver
)


idf_component_get_property(_lib ${COMPONENT_NAME} COMPONENT_LIB)
target_compile_options(${_lib} INTERFACE
    -ffast-math
    -O3
    -Wno-error=format
    -Wno-format
)