# IR-Room-Monitor manual

## What is this

This is the source code of a personnel counting device. The device is mounted on the top of a door frame, monitoring an area of about $3m^2$ below it. The device is able to count human enter/leave events relatively, and offers an human count value in the building. The count value can be used for optimization of heating, ventilation and air-condition usage of the building or occupancy control during the Corona pandemic. 

The device is developed with a ESP32 board and an infrared thermopile array (Grideye Amg8833). It uses a traditional blob tracking based method. Data are transfered to a centralized data bank through MQTT.    

Two platforms are used in this project. One of them is Node-Red for debug purpose, which visualizes the real-time camera data stream, show internal states of the device and send new configuration to the device. The other is the CAPS IoT Platform based on ElasticSearch+Kibana, developed by TUM researchers, which stores the count value published by the device in a data bank for long-term evaluation. Configurations of both platforms will also be introduced later in this document.

## How to run it

### in-situation use case

Read camera frames from the termopile sensor.

1. Hardware Circuiting: The thermopile sensor Grideye Amg8833 uses I2C interface. By default, the SDA pin and SCL pin on ESP32 are 22 and 21 relatively (however, they are configurable to any digital pin). 

| ESP32           | 22 (SDA) | 21 (SCL) | GND  | 3.3V |
| --------------- | -------- | -------- | ---- | ---- |
| Grideye Amg8833 | SDA      | SCL      | GND  | Vin  |

2. Configuration of the network: the device need to publish the latest count value to outside through MQTT. So Wifi & MQTT connection need to be established at first. Go to `macro.h` under `/main` and change the Wifi SSID, password and MQTT uri (and MQTT username and password if applicable).

3. Configuration of the SNTP server: we need the timestamp of an event to see when did it happen. The realtime timestamp is retrieved from the router before the program is running. Choose a right sntp server that is available from your home network.

4.  Setup a MQTT broker and a consumer to receive the messages: the topics are

   | topics         | description                                                  |
   | -------------- | ------------------------------------------------------------ |
   | amg8833/pixels | the camera stream of the sensor (data format: 64 grayscale pixels are transferred as one csv format string, separated by a comma. All values are multiplied by 256 to preserve precision, e.g. a pixel of $20^\circ C$ has the value 5120) |
   | amg8833/count  | the final count value accumulated by every relative count    |
   | amg8833/speed  | Used for debug: the time consumed for processing one  frame (Note that because the camera runs at 10 fps, the process time must be less than 0.1s) |

### UART Simulation

 Replay frame sequences by feeding frame data through UART. 

1. go to `macro.h` and uncomment `#define UART_SIM`

2. the main function will not initialize the I2C interface at all but waiting for data through UART, expected data format is the same as `amg8833/pixels`

3. use the python script and the example data file under directory `/uart_sim`. The script will send one line at each time to simulate a frame sequence read from the camera. The command to use the script is

   | python simulation.py -p <port> -m <mode> -f <file> | description                                                  |
   | -------------------------------------------------- | ------------------------------------------------------------ |
   | -p                                                 | the port that the ESP32 board is on, on Windows it can be COM10 |
   | -m                                                 | "console": manually type in the value of 64 pixels, only used for debug<br />"file": iteratively send every line of the file at an interval of 0.1s to simulate an event <br />"frame": send one frame at a time when enter is pressed. Used to replay the event at a slower pace. |
   | -f                                                 | the relative file path to the csv style file                 |



## Algorithm behind it

After a frame is received, it is processed by tradition CV method based on blobs. The whole processing procedure could be divided into three parts: blob detection, feature extraction and object tracking.

In the following subsections, the main algorithm in each part will be introduced individually.

### blob detection

The task of the layer is to find out region of interest (ROI) from the whole frame. 

The original image resolution output by the Grideye sensor is $8\times8$, which is too coarse. To simplify the following segmentation and avoid too raster images (which is hard to read), the original image is linear interpolated to $71\times 71$ by inserting 9 pixels between each original pixel.   

