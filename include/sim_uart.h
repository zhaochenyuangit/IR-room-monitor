#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

#define EX_UART_NUM UART_NUM_0
/**
 * @brief initilize UART0, which is connected to USB
 */ 
void uart_init(void);
/**
 * @brief read simulated event data from serial, 
 * and pass data as if received from sensor
 * @param queue the queue handler which mqtt is listened on
 */ 
void uart_receive_pixels(void *pixel_queue);