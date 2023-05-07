import requests
import time
import json


def get_temperature():
    try:
        return requests.get('http://192.168.1.207/', timeout=10).json()
    except:
        return None


last_write = time.time()
with open('temperature_rise.json', 'a') as f:
    while 1:
        data = get_temperature()
        if data:
            f.write(json.dumps(data) + "\n")
            print(data)
            time.sleep(1)
            if time.time() - last_write > 60:
                f.flush()
                last_write = time.time()
