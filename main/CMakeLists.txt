idf_component_register(
    SRCS "main.c" "app_network.c" "app_sensor.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES BSP nvs_flash esp_wifi esp_netif mqtt esp_event
)