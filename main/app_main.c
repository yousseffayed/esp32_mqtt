/* MQTT over SSL Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_sntp.h"
#include "esp_timer.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota_ops.h"
#include <sys/param.h>
#include "driver/gpio.h" 
static const char *TAG = "mqtts_example";

#define BLINK_GPIO 2
static uint8_t s_led_state = 0;


#define CLIENT_ID "esp32" // Specify the client ID for your device
#define CUSTOM_TOPIC "hello"
#define CUSTOM_MESSAGE "Hello from ESP32!"

#if CONFIG_BROKER_CERTIFICATE_OVERRIDDEN == 1
static const uint8_t mqtt_eclipseprojects_io_pem_start[]  = "-----BEGIN CERTIFICATE-----\n" CONFIG_BROKER_CERTIFICATE_OVERRIDE "\n-----END CERTIFICATE-----";
#else
extern const uint8_t mqtt_eclipseprojects_io_pem_start[]   asm("_binary_mqtt_eclipseprojects_io_pem_start");
#endif
extern const uint8_t mqtt_eclipseprojects_io_pem_end[]   asm("_binary_mqtt_eclipseprojects_io_pem_end");
// Declare the certificate and key symbols
extern const uint8_t esp32_cert_pem_start[] asm("_binary_esp32_cert_pem_start");
extern const uint8_t esp32_cert_pem_end[] asm("_binary_esp32_cert_pem_end");
extern const uint8_t esp32_key_pem_start[] asm("_binary_esp32_private_key_start");
extern const uint8_t esp32_key_pem_end[] asm("_binary_esp32_private_key_end");






//       (to be checked against the original binary)
//
// static void send_binary(esp_mqtt_client_handle_t client)
// {
//     esp_partition_mmap_handle_t out_handle;
//     const void *binary_address;
//     const esp_partition_t *partition = esp_ota_get_running_partition();
//     esp_partition_mmap(partition, 0, partition->size, ESP_PARTITION_MMAP_DATA, &binary_address, &out_handle);
//     // sending only the configured portion of the partition (if it's less than the partition size)
//     int binary_size = MIN(CONFIG_BROKER_BIN_SIZE_TO_SEND, partition->size);
//     int msg_id = esp_mqtt_client_publish(client, "/topic/binary", binary_address, binary_size, 0, 0);
//     ESP_LOGI(TAG, "binary sent with msg_id=%d", msg_id);
// }

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);                          
}


static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}



static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        // msg_id = esp_mqtt_client_subscribe(client, CUSTOM_TOPIC, 0);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);


        msg_id = esp_mqtt_client_subscribe(client, CUSTOM_TOPIC, 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "hello/qos0", 0);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "hello/qos1", 1);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);



        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, CUSTOM_TOPIC, CUSTOM_MESSAGE, strlen(CUSTOM_MESSAGE), 0, 1);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_publish(client, CUSTOM_TOPIC, CUSTOM_MESSAGE, strlen(CUSTOM_MESSAGE), 1, 1);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
       // printf("DATA=%.*s\r\n", event->data_len, event->data);
        if (strncmp(event->data, "ledon", event->data_len) == 0) {
            ESP_LOGI(TAG, "turning ON LED");
            s_led_state = 1;
            blink_led();
        }
        else if (strncmp(event->data, "ledoff", event->data_len) == 0) {
            ESP_LOGI(TAG, "turning OFF LED");
            s_led_state = 0;
            blink_led();
        }
         else if (strncmp(event->data, "", event->data_len) != 0){
             printf("Unknown command: %.*s\r\n", event->data_len, event->data);
         }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


static void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());

    // Configure the MQTT client with the certificates and private key
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtts://aehnkcy6nec1e-ats.iot.eu-central-1.amazonaws.com:8883",  // Your AWS IoT Core endpoint (e.g., "mqtts://your-endpoint.amazonaws.com")
        .broker.verification.certificate = (const char *)mqtt_eclipseprojects_io_pem_start,  // Root CA certificate (ISRG Root X1)
              //  .client_id = CLIENT_ID, // Set your client ID here

        // Device certificate and private key for mutual authentication
        .credentials.authentication.certificate = (const char *)esp32_cert_pem_start,   // Device certificate
        .credentials.authentication.key = (const char *)esp32_key_pem_start,           // Device private key
    };

    // Initialize the MQTT client
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    // Register the event handler for MQTT events
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    // Start the MQTT client
    esp_mqtt_client_start(client);


}


void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */

    configure_led();
    ESP_ERROR_CHECK(example_connect());
    // initialize_sntp();
    // vTaskDelay(pdMS_TO_TICKS(1000));

    mqtt_app_start();
}
