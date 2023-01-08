#The following code is a microservice written in python using paho library which extract data by subscribing to a MQTT broker,
# such broker is receiving data from a wemos which works as publisher, once the microservice displays the data it also saves it in an
# influxdb bucket.

from paho.mqtt import client as mqtt_client
import json
import influxdb_client, time
from influxdb_client import Point
from influxdb_client.client.write_api import SYNCHRONOUS
from influxdb_client.domain.write_precision import WritePrecision

#Credentials MQTT Broker
broker = 'test.mosquitto.org'
port = 1883
topic = "pucv/iot/m6/p1"

#Credentials InfluxDB
token = ""
org = "Education"
url = "https://us-west-2-2.aws.cloud2.influxdata.com"

client = influxdb_client.InfluxDBClient(url=url, token=token, org=org)

bucket = "insert_fluxdb"

write_api = client.write_api(write_options=SYNCHRONOUS)

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
        data_dict = json.loads(data)
        epoch_ms = data_dict['ts']*1000
        print(data_dict)
        #To send continuously only one temp measurement:
        #point = (
        #    Point("Plant_sensor")
        #    .field("temperature", data_dict['temperatura'])
        #    .tag("id_sensor", data_dict['device_id'])\
        #    .time(epoch_ms, write_precision=WritePrecision.MS)
        #)
        #write_api.write(bucket=bucket, org=org, record=point)
        #print("Point saved: ", point)
        #To send continuously a set of measurements from multiple sensors in a system (1st way):
        #point = (
        #    Point("Plant_sensors")
        #    .field("temperature", data_dict['temperature'])
        #    .field("moisture", data_dict['moisture'])
        #    .tag("id_sensor", data_dict['device_id'])
        #    .time(epoch_ms, write_precision=WritePrecision.MS)
        #)
        #write_api.write(bucket=bucket, org=org, record=point)
        #print("Point saved: ", point)
        #To send continuously a set of measurements from multiple sensors in a system (2nd way):
        for measurement in ['temperature','moisture']:
            point = (
             Point("Plant_sensors")
             .field(measurement, data_dict[measurement])
             .tag("id_sensor", data_dict['device_id'])
             .time(epoch_ms, write_precision=WritePrecision.MS)
             )
            write_api.write(bucket=bucket, org=org, record=point)
            print("Point saved: ", point)

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()

if __name__ == '__main__':
    run()
