import sys
import os
import serial
import paho.mqtt.client as mqtt
import json
from datetime import datetime
from time import sleep
import numpy as np

sensor = "anno"
username = "JWT"
username_pub = "chenyuan"
device_id = 143
jsonfile_path = os.path.join("..","tum_iotplatform_related","device_{}_key.json".format(device_id))
topic = ""
password = ""
with open(jsonfile_path) as f:
    info_dict = json.load(f)
    topic = "{}_{}".format(info_dict["user_id"],info_dict["device_id"])
    password = info_dict["token"]

def calenda_to_ms(datetime):
    return (int)(datetime.timestamp()) * 1000

mqtt_gw = '131.159.35.132'
port_gw = 1883

#mqtt callbacks
def on_connect_impl(client, userdata, flags, rc):
    print('Connection result code: ' + str(rc))
    (res, mid) = client.subscribe(topic)
    print('Subscribed with result code: ' + str(res))
    print('with mesage id: ' + str(mid))

def on_message_impl(client, userdata, msg):
    print('Message received: ')
    print(msg.topic + '-> ' + str(msg.payload))
    
def on_publish_impl(client, userdata, mid):
    print("Published......",mid)
    
# start a number topic

client = mqtt.Client()
client.loop_start()
client.on_connect = on_connect_impl
client.on_message = on_message_impl
client.on_publish = on_publish_impl

client.username_pw_set(username, password)
client.connect(mqtt_gw, port_gw, 60)
sleep(2)

time_parse_patterns = ["%Y %m %d %H %M","%Y %m %d %H %M %S"]
while(1):
    timestr = input("\n calendar time of event: ")
    if timestr=="q":
        exit()
    for p in time_parse_patterns:
        try:
            time = datetime.strptime(timestr,p)
            break
        except ValueError:
            continue
    else:
        continue
    print(time)
    timestamp = calenda_to_ms(time)
    count = ""
    while(1):
        try:
            count = input("\n count?: ")
            if count == "":
                continue
            count = eval(count)
            break
        except ValueError:
            continue
        
    msg_dict = {}
    msg_dict["username"] = username_pub
    msg_dict[sensor] = count
    msg_dict["device_id"] = device_id
    msg_dict["timestamp"] = timestamp
    msg_str = json.dumps(msg_dict)
    print(msg_str)
    client.publish(topic,msg_str,1)



