set(src_dirs LED)
set(include_dirs LED)
set(requires driver)

# 引入所有源文件
idf_component_register(
    SRCS "LED/led.c" "DHT11/dht11.c" "WebConfig/webconfig.c"
    
    INCLUDE_DIRS "LED" "DHT11" "WebConfig"
    
    # 网页文件直接嵌进 Flash 内存里
    EMBED_TXTFILES "WebConfig/apcfg.html"

    REQUIRES driver esp_http_server json esp_wifi nvs_flash
)


idf_component_get_property(_lib ${COMPONENT_NAME} COMPONENT_LIB)
target_compile_options(${_lib} INTERFACE
    -ffast-math
    -O3
    -Wno-error=format
    -Wno-format
)