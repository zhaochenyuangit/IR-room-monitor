#include "main.hpp"

static const char *TAG = "debug";
esp_mqtt_client_handle_t client = NULL;
TaskHandle_t img_process = NULL;
xQueueHandle q_pixels = NULL;
xQueueHandle q_thms = NULL;
xQueueHandle q_listen = NULL;
static const uint8_t q_len = 5;
SemaphoreHandle_t sema_raw = NULL;
SemaphoreHandle_t sema_im = NULL;

char pixel_msg_buf[400];
char thms_msg_buf[20];
char mask_msg_buf[26000];

uint8_t mask[IM_LEN];
ObjectList tracking;
volatile int global_threshold = 22 * AMG8833_TEMP_FACTOR;

void image_process(void *arg)
{
    short pixel_value[SNR_SZ];
    char performance_msg_buf[10];
    char count_msg_buf[10];
    struct timeval last_entry, now;
    while (1)
    {
        if (xQueueReceive(q_pixels, &pixel_value, portMAX_DELAY) == pdTRUE)
        {
#ifndef UART_SIM
            gettimeofday(&now, NULL);
            if ((now.tv_sec - last_entry.tv_sec) > 2)
            {
                tracking.count_and_delete_every_objects();
            }
            last_entry = now;
#endif
            performance_evaluation(0);
            sh_array_to_string(pixel_value, pixel_msg_buf, SNR_SZ);
            xSemaphoreGive(sema_raw);
            int n_blobs = blob_detection(pixel_value, mask, global_threshold);
#if DEBUG
            c_array_to_string(mask, mask_msg_buf, IM_LEN);
            xSemaphoreGive(sema_im);
#endif
            Blob *blob_list = extract_feature(mask, n_blobs, IM_W, IM_H);
            tracking.matching(blob_list, n_blobs);
            delete_blob_list(blob_list, n_blobs);

#ifdef ENABLE_NETWORK
            sprintf(performance_msg_buf, "%.2f", performance_evaluation(1));
            mqtt_send(client, "amg8833/speed", performance_msg_buf);
            sprintf(count_msg_buf, "%d", tracking.get_count());
            mqtt_send(client, "amg8833/count", count_msg_buf);
#endif
            DBG_PRINT("count %d\n\n", tracking.get_count());
        }
        //vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void pub_raw(void *_)
{
    const char *topic = "amg8833/pixels";
    while (1)
    {
        if (xSemaphoreTake(sema_raw, portMAX_DELAY) == pdTRUE)
        {
#ifdef ENABLE_NETWORK
            mqtt_send(client, topic, pixel_msg_buf);
#endif
        }
    }
}

void pub_im(void *_)
{
    const char *topic = "amg8833/image71";
    while (1)
    {
        if (xSemaphoreTake(sema_im, portMAX_DELAY) == pdTRUE)
        {
#ifdef ENABLE_NETWORK
            mqtt_send(client, topic, mask_msg_buf);
#endif
        }
    }
}

#ifdef ENABLE_NETWORK
void listen_topic(void *_)
{
    struct mqtt_msg msg;
    while (1)
    {
        if (xQueueReceive(q_listen, &msg, portMAX_DELAY))
        {
            if (strcmp(msg.topic, "amg8833/reset") == 0)
            {
                tracking.reset_count();
            }
            if (strcmp(msg.topic, "dht11/room_temp") == 0)
            {
                int temp = atoi(msg.msg);
                printf("change global temp to: %d\n", temp);
                global_threshold = temp*AMG8833_TEMP_FACTOR;
            }
        }
    }
}
#endif

#ifndef UART_SIM
int detect_activation(short *pixels, short thms, UCHAR *mask)
{
    int count = 0;
    memset(mask, 0, SNR_SZ);
    short low_b = global_threshold;
    for (int i = 0; i < SNR_SZ; i++)
    {
        if (pixels[i] > low_b)
        {
            mask[i] = pdTRUE;
            count++;
        }
    }
    return count;
}

void temp_calibration()
{
    int room_temperature = 0;
    struct dht11_reading dht11_data;
    for (int i = 0; i < 100; i++)
    {
        dht11_data = DHT11_read();
        if (dht11_data.status)
        {
            i = 0;
            continue;
        }
        room_temperature *= 0.5;
        room_temperature += dht11_data.temperature * 0.5;
    }
    global_threshold = (room_temperature)*AMG8833_TEMP_FACTOR;
    char buf[20];
    sprintf(buf,"%d",global_threshold/256);
    mqtt_send(client,"dht11/thres",buf);
}

void read_grideye(void *parameter)
{
    short pixel_value[SNR_SZ];
    short thms_value;
    int no_activate_frame = 10;

    while (1)
    {
        read_pixels(pixel_value);
        read_thermistor(&thms_value);
        int count = detect_activation(pixel_value, thms_value, mask);
        if (count <= 5)
        {
            no_activate_frame += 1;
        }
        else
        {
            no_activate_frame = 0;
        }
        if (no_activate_frame <= 10)
        {

            if (xQueueSend(q_pixels, &pixel_value, 10) != pdTRUE)
            {
                ESP_LOGI(TAG, "queue pixel is full");
            }
            if (xQueueSend(q_thms, &thms_value, 10) != pdTRUE)
            {
                ESP_LOGI(TAG, "queue thms is full");
            }
        }
        //printf("task read watermark: %d\n", uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void pub_thms(void *_)
{
    const char *topic2 = "amg8833/thermistor";
    short thms;
    char thms_msg_buf[10];
    while (1)
    {
        if (xQueueReceive(q_thms, &thms, portMAX_DELAY) == pdTRUE)
        {
            sprintf(thms_msg_buf, "%hd", thms);
            mqtt_send(client, topic2, thms_msg_buf);
        }
        //vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
#endif

extern "C" void app_main(void)
{
#ifdef UART_SIM
    printf("main programm start\n");
    uart_init();
#else
    i2c_master_init();
#endif
#ifdef ENABLE_NETWORK
    start_wifi();
    start_mqtt(&client);
    mqtt_listen(client, "amg8833/reset", &q_listen);
    mqtt_listen(client, "dht11/room_temp", &q_listen);
#endif
    q_pixels = xQueueCreate(q_len, sizeof(short[SNR_SZ]));
    q_thms = xQueueCreate(q_len, sizeof(short));
    if (q_pixels == NULL)
    {
        ESP_LOGE(TAG, "create pixels queue failed");
    }
    if (q_thms == NULL)
    {
        ESP_LOGE(TAG, "create thms queue failed");
    }
    sema_raw = xSemaphoreCreateBinary();
    sema_im = xSemaphoreCreateBinary();
    if (sema_raw == NULL)
    {
        ESP_LOGE(TAG, "create sema raw failed");
    }
    if (sema_im == NULL)
    {
        ESP_LOGE(TAG, "create sema im failed");
    }

#ifdef UART_SIM
    xTaskCreatePinnedToCore(uart_receive_pixels, "uart_event", 4000, (void *)q_pixels, 5, NULL, 1);
#else
    xTaskCreatePinnedToCore(read_grideye, "read_grideye", 3000, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(pub_thms, "publish thms", 2000, NULL, 1, NULL, 0);
    // temperature calibration
    DHT11_init(DHT11_PIN);
    temp_calibration();
    printf("init room temperature to %d C\n", global_threshold);
#endif
    xTaskCreatePinnedToCore(image_process, "process", 4000, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(pub_raw, "publish", 2000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(pub_im, "publish im ", 2000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(listen_topic, "listen mqtt", 2000, NULL, 1, NULL, 0);
}