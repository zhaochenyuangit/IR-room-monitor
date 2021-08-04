#define IM_W (71)
#define IM_H (71)
#define IM_LEN (IM_W * IM_H)
#define AMG8833_TEMP_FACTOR (256)
#define DHT11_PIN GPIO_NUM_25 

//#define UART_SIM
#define ENABLE_NETWORK

#define DEBUG 0

#if DEBUG
#define DBG_PRINT(format, args...)\                             
        do{\
        printf(format, ##args);\
        }while (0)
#else
#define DBG_PRINT(format, args...)
#endif