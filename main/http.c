#include "esp_http_server.h"
#include "esp_log.h"
#include <string.h>
#include "frame_base.h"

#define POST_DATA_BUFSIZE 10240

static const char *TAG = "light frame http";
static char* postDataBuffer;

void pause();
void resume();
void stop();

static esp_err_t pauseHandler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "pause");
    pause();
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static httpd_uri_t api_pause = {
    .uri        = "/pause",
    .method     = HTTP_POST,
    .handler    = pauseHandler,
    .user_ctx   = NULL
};

static esp_err_t resumeHandler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "resume");
    resume();
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static httpd_uri_t api_resume = {
    .uri        = "/resume",
    .method     = HTTP_POST,
    .handler    = resumeHandler,
    .user_ctx   = NULL
};

static esp_err_t stopHandler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "stop");
    stop();
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static httpd_uri_t api_stop = {
    .uri        = "/stop",
    .method     = HTTP_POST,
    .handler    = stopHandler,
    .user_ctx   = NULL
};

static esp_err_t setSceneConfigHandler(httpd_req_t *req)
{
    int query_len = httpd_req_get_url_query_len(req) + 1;
    if (query_len <= 1) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "scene must be specified as query param");
        return ESP_FAIL;
    }
    if (query_len > 10) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "query string too long");
        return ESP_FAIL;
    }
    char queryStringBuffer[20];
    char scene[10];
    if (httpd_req_get_url_query_str(req, queryStringBuffer, query_len) == ESP_OK) {
        if (httpd_query_key_value(queryStringBuffer, "scene", scene, sizeof(scene)) == ESP_OK) {
            ESP_LOGI(TAG, "Scene query parameter: %s", scene);
        }
    }

    int body_len = req->content_len;
    int cur_len = 0;
    int received = 0;
    if (body_len >= POST_DATA_BUFSIZE) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < body_len) {
        received = httpd_req_recv(req, postDataBuffer + cur_len, body_len);
        if (received <= 0) {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Post value is not valid");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    postDataBuffer[body_len] = '\0';

    cJSON *json = cJSON_Parse(postDataBuffer);
    setSceneConfig(scene, json);
    cJSON_Delete(json);

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static httpd_uri_t api_scene_config = {
    .uri        = "/scene-config",
    .method     = HTTP_POST,
    .handler    = setSceneConfigHandler,
    .user_ctx   = NULL
};

static esp_err_t setCurrentSceneHandler(httpd_req_t *req)
{
    char scene[10];

    int body_len = req->content_len;
    if (body_len >= 10) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "content too long");
        return ESP_FAIL;
    }
    int received = httpd_req_recv(req, scene, body_len);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Post value is not valid");
        return ESP_FAIL;
    }
    scene[body_len] = '\0';

    setCurrentScene(scene);

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static httpd_uri_t api_current_scene = {
    .uri        = "/current-scene",
    .method     = HTTP_POST,
    .handler    = setCurrentSceneHandler,
    .user_ctx   = NULL
};

httpd_handle_t http_start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    postDataBuffer = (char*) malloc(POST_DATA_BUFSIZE);
    if (postDataBuffer == NULL) {
        ESP_LOGI(TAG, "Error allocating memory for post data buffer");
        return NULL;
    }

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &api_pause);
        httpd_register_uri_handler(server, &api_resume);
        httpd_register_uri_handler(server, &api_stop);
        httpd_register_uri_handler(server, &api_scene_config);
        httpd_register_uri_handler(server, &api_current_scene);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void http_stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}