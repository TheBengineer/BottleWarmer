import requests
import time
import json


def get_temperature():
    return requests.get('http://192.168.1.207/').json()


with open('temperature_rise.json', 'a') as f:
    while 1:
        data = get_temperature()
        f.write(json.dumps(data) + "\n")
        print(data)
        time.sleep(1)
