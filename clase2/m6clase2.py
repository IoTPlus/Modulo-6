from paho.mqtt import client as mqtt_client
import json

import influxdb_client, time
from influxdb_client import Point
from influxdb_client.client.write_api import SYNCHRONOUS

#DATOS MOSQUITTO
broker = 'test.mosquitto.org'
port = 1883
topic = "pucv/iot/m6/p1/g6"

#DATOS INFLUXDB
token= "XXXXXXxxxxxXXXXXxxxxXXXXxx"
org = "XXXXemailcuentaXXXX"
url = "https://us-west-2-2.aws.cloud2.influxdata.com"
bucket = "XXXXnombre_bucketXXX"

def insert(sensor, key, value):
    influxclient = influxdb_client.InfluxDBClient(url=url, token=token, org=org)
    #client = influxdb_client.InfluxDBClient(url=url, token=token, org=org, verify_ssl=False)
    write_api = influxclient.write_api(write_options=SYNCHRONOUS)

    point = (
        Point(sensor)
        .field(key, value)
        .tag("field1", key)
    )
    write_api.write(bucket=bucket, org=org, record=point)
    print("Point saved: ", point)

def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)
    client = mqtt_client.Client()
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        data = msg.payload
        print(f"Received `{ json.loads(data) }` from `{msg.topic}` topic")
        insert('sensor1','temperature',json.loads(data)['temperature'])
    client.subscribe(topic)
    client.on_message = on_message

def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()

if __name__ == '__main__':
    run()
