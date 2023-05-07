import requests
import time
import json


def get_temperature():
    return requests.get('http://192.168.1.207/').json()

last_write = time.time()
with open('temperature_rise.json', 'a') as f:
    while 1:
        data = get_temperature()
        f.write(json.dumps(data) + "\n")
        print(data)
        time.sleep(1)
        if time.time() - last_write > 60:
            f.flush()
            last_write = time.time()
