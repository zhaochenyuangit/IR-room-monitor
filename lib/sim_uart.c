#include "sim_uart.h"
#include "grideye_api_common.h"

static const char *TAG = "uart_events";
static const int uart_buf_size = 1024;
static QueueHandle_t uart0_queue;

static void parse_csv_to_array(uint8_t *buf, short *array, int size)
{
    char string[uart_buf_size];
    memcpy(string, buf, size);
    string[size] = '\0';
    int total_length = strlen(string);

    int processed_num;
    short value;
    int index_str = 0;
    int index_ar = 0;

    while (index_str < total_length)
    {
        int ret = sscanf(string + index_str, "%hd%n", &value, &processed_num);
        assert(ret == 1);
        array[index_ar] = value;
        index_ar++;
        index_str += processed_num + 1; //plus 1 to skip comma
    }
}

void uart_receive_pixels(void *pixel_queue)
{
    pixel_queue = (QueueHandle_t)pixel_queue;
    uart_event_t event;
    uint8_t *read_buf = (uint8_t *)malloc(uart_buf_size);
    short pixel_array[SNR_SZ];
    for (int i = 0; i < SNR_SZ; i++)
    {
        pixel_array[i] = 0;
    }
    pixel_array[0] = 12345;
    while (1)
    {
        if (xQueueReceive(uart0_queue, (void *)&event, (portTickType)portMAX_DELAY))
        {
            memset(read_buf, 0, uart_buf_size);
            switch (event.type)
            {
            case UART_PATTERN_DET:;
                int pos = uart_pattern_pop_pos(EX_UART_NUM);
                if (pos == -1)
                {
                    ESP_LOGE(TAG, "pattern symbol missed, reflush the uart buffer");
                    uart_flush_input(EX_UART_NUM);
                }
                else
                {
                    uart_read_bytes(EX_UART_NUM, read_buf, pos + 1, pdMS_TO_TICKS(100));
                    //printf("received data: %s\n", read_buf);
                    parse_csv_to_array(read_buf, pixel_array, pos);
                    if (xQueueSend(pixel_queue, &pixel_array, 10) != pdTRUE)
                    {
                        ESP_LOGE(TAG, "queue is full!");
                    }
                }
                break;
            //Others
            default:
                ESP_LOGD(TAG, "uart event type: %d", event.type);
                break;
            }
            if (uxTaskGetStackHighWaterMark(NULL) < 500)
            {
                ESP_LOGW(TAG, "high watermark %d", uxTaskGetStackHighWaterMark(NULL));
            }
        }
    }
    free(read_buf);
    read_buf = NULL;
    vTaskDelete(NULL);
}

void uart_init(void)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    //Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, uart_buf_size * 2, uart_buf_size * 2, 20, &uart0_queue, 0);
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(EX_UART_NUM, &uart_config);

    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //Set uart pattern detect function.
    uart_enable_pattern_det_baud_intr(EX_UART_NUM, '\n', 1, 9, 0, 0);
    //Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(EX_UART_NUM, 20);
}
