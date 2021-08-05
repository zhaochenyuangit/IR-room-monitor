extern "C"
{
#include <stdio.h>
#include "stdlib.h"
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <sys/time.h>
#include "time.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "sdkconfig.h"

#include "network_common.h"
#include "grideye_api_lv1.h"
#include "detect.h"
#include "feature_extraction.h"
#include "sim_uart.h"
#include "helper.h"
#include "dht11.h"
#include "sntp_controller.h"
}

#include "human_object.hpp"
#include "tracking.hpp"
#include "macro.h"

