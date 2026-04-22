#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "webconfig.h"

static const char *TAG = "WebConfig";

extern const uint8_t apcfg_html_start[] asm("_binary_apcfg_html_start");
extern const uint8_t apcfg_html_end[]   asm("_binary_apcfg_html_end");

// 网页服务器全局句柄
static httpd_handle_t web_server_handle = NULL;

static esp_err_t index_html_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    const uint32_t apcfg_html_len = apcfg_html_end - apcfg_html_start;
    httpd_resp_send(req, (const char *)apcfg_html_start, apcfg_html_len);
    return ESP_OK;
}

static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "手机 WebSocket 已连接！");
        return ESP_OK; 
    }

    uint8_t buf[256] = {0};
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = buf;
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 255);
    if (ret != ESP_OK) return ret;

    cJSON *root = cJSON_Parse((char*)ws_pkt.payload);
    if (!root) return ESP_FAIL;

    if (cJSON_HasObjectItem(root, "scan")) {
        char *scan_reply = "{\"wifi_list\":[{\"ssid\":\"My_Home_WiFi\",\"rssi\":-50,\"encrypted\":true},{\"ssid\":\"ChinaNet-888\",\"rssi\":-70,\"encrypted\":true}]}";
        httpd_ws_frame_t res_pkt = {
            .payload = (uint8_t*)scan_reply, .len = strlen(scan_reply), .type = HTTPD_WS_TYPE_TEXT
        };
        httpd_ws_send_frame(req, &res_pkt);
    }
    
    if (cJSON_HasObjectItem(root, "ssid")) {
        char *ssid = cJSON_GetObjectItem(root, "ssid")->valuestring;
        char *pass = cJSON_GetObjectItem(root, "password")->valuestring;
        
        ESP_LOGI(TAG, "🎉 收到配网指令，准备连接路由: %s", ssid);
        
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        wifi_config_t wifi_cfg = {0};
        strncpy((char *)wifi_cfg.sta.ssid, ssid, sizeof(wifi_cfg.sta.ssid) - 1);
        strncpy((char *)wifi_cfg.sta.password, pass, sizeof(wifi_cfg.sta.password) - 1);
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
        esp_wifi_connect();

        char *conn_reply = "{\"event\":\"wifi_status\",\"status\":\"connected\"}";
        httpd_ws_frame_t res_pkt = {
            .payload = (uint8_t*)conn_reply, .len = strlen(conn_reply), .type = HTTPD_WS_TYPE_TEXT 
        };
        httpd_ws_send_frame(req, &res_pkt);
    }
    
    cJSON_Delete(root);
    return ESP_OK;
}

static void start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&web_server_handle, &config) == ESP_OK) {
        httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_html_get_handler, .user_ctx = NULL };
        httpd_register_uri_handler(web_server_handle, &index_uri);
        httpd_uri_t ws_uri = { .uri = "/ws", .method = HTTP_GET, .handler = ws_handler, .user_ctx = NULL, .is_websocket = true };
        httpd_register_uri_handler(web_server_handle, &ws_uri);
        ESP_LOGI(TAG, "网页服务器 & WebSocket 启动成功！");
    }
}

// ============== 新增：关闭服务器函数 ==============
void webconfig_stop(void) {
    if (web_server_handle != NULL) {
        httpd_stop(web_server_handle);
        web_server_handle = NULL;
        ESP_LOGI(TAG, "网页服务器已完全关闭释放");
    }
}

void webconfig_init_ap_and_server(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default()); 
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32_Config", .ssid_len = strlen("ESP32_Config"),
            .channel = 1, .password = "", .max_connection = 4, .authmode = WIFI_AUTH_OPEN
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    start_webserver();
}