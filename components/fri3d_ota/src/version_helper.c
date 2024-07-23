#include "sdkconfig.h"

#ifdef CONFIG_FRI3D_BADGE_FOX
#define CONFIG_VERSIONS_URL "https://fri3d.be/firmware/firmware-fox.json"
#endif

#ifdef CONFIG_FRI3D_BADGE_OCTOPUS
#define CONFIG_VERSIONS_URL "https://fri3d.be/firmware/firmware-octopus.json"
#endif

/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
    inspiration for chunked encoding + json decoding from https://github.com/parabuzzle/idf_http_rest_client
 */

#include "esp_crt_bundle.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_tls.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_http_client.h"

#include "fri3d_private/h2non_semver.h"
#include "fri3d_private/ota_wifi_secrets.h"
#include "fri3d_private/version_helper.h"

static const char *TAG = "version_helper";

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{

    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGV(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGV(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGV(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGV(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);

        http_rest_recv_buffer_t *response_buffer = (http_rest_recv_buffer_t *)evt->user_data;

        // ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        // ESP_LOGV(TAG, "DATA: %s", (char *)evt->data);

        // Increase the buffer size to fit the new data
        response_buffer->buffer = realloc(response_buffer->buffer, response_buffer->buffer_len + evt->data_len + 1);
        // ESP_LOGV(TAG, "Buffer realloced to %d bytes", response_buffer->buffer_len + evt->data_len + 1);

        // Copy the new data to the buffer
        memcpy(response_buffer->buffer + response_buffer->buffer_len, (uint8_t *)evt->data, evt->data_len);
        // ESP_LOGV(TAG, "Data copied to buffer");

        // Increase the buffer length
        response_buffer->buffer_len += evt->data_len;
        // ESP_LOGV(TAG, "Buffer length increased to %d bytes", response_buffer->buffer_len);

        // Add a null terminator to the end of the buffer
        response_buffer->buffer[response_buffer->buffer_len] = '\0';
        // ESP_LOGV(TAG, "Null terminator added to buffer");

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        esp_http_client_set_header(evt->client, "From", "user@example.com");
        esp_http_client_set_header(evt->client, "Accept", "text/html");
        esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}

static void https_with_url(http_rest_recv_json_t *response_buffer)
{
    http_rest_recv_buffer_t http_rest_recv_buffer;
    memset(&http_rest_recv_buffer, 0, sizeof(http_rest_recv_buffer_t));

    esp_http_client_config_t config = {
        .url = CONFIG_VERSIONS_URL,
        .event_handler = _http_event_handler,
        .user_data = &http_rest_recv_buffer, // Pass address of local buffer to get response
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    ESP_ERROR_CHECK(esp_http_client_set_header(client, "Accept", "application/vnd.github+json"));
    ESP_LOGD(TAG, "Request to %s", config.url);
    esp_err_t err = esp_http_client_perform(client);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    else
    {
        http_rest_recv_buffer.status_code = esp_http_client_get_status_code(client);
        if (http_rest_recv_buffer.status_code != 200)
        {
            ESP_LOGE(TAG, "HTTP GET request failed with status code: %d", http_rest_recv_buffer.status_code);
            ESP_LOGE(TAG, "HTTP GET reesponse: %s", http_rest_recv_buffer.buffer);
        }
        else
        {
            cJSON *json = cJSON_Parse((char *)http_rest_recv_buffer.buffer);

            if (json == NULL)
            {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                    ESP_LOGE(TAG, "Error before: %s", error_ptr);
                }
                err = ESP_FAIL;
            }
            else
            {
                response_buffer->json = json;
                response_buffer->status_code = http_rest_recv_buffer.status_code;

                ESP_LOGD(TAG, "JSON parsed");
            }
        }
    }

    if (http_rest_recv_buffer.buffer != NULL)
    {
        free(http_rest_recv_buffer.buffer);
    }
    esp_http_client_cleanup(client);
}

void http_get_versions_task(void *pvParameters)
{
    esp_log_level_set(TAG, LOG_LOCAL_LEVEL);

    version_task_parameters_t *params = (version_task_parameters_t *)pvParameters;
    // params->ota_versions;
    ESP_LOGD(TAG, "board_name: %s", params->board_name);

    http_rest_recv_json_t response_buffer = {0};
    https_with_url(&response_buffer);

    ESP_LOGI(TAG, "response status code: %d", response_buffer.status_code);

    // char *jsonString = cJSON_Print(response_buffer.json);
    // ESP_LOGI(TAG, "Response: %s", jsonString);
    // free(jsonString);

    params->json = response_buffer.json;

    // if (response_buffer.json != NULL)
    // {
    //     cJSON_Delete(response_buffer.json);
    // }

    ESP_LOGI(TAG, "Finish http_get_versions_task");

    // uncomment this when it is a task again
    vTaskDelete(NULL);
}
