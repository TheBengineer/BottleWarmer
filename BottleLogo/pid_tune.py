from datetime import datetime
import scipy
import matplotlib.pyplot as plt
import numpy as np
import json

max_time = 2 * 60 * 60  # 2 hours
time_step = .1  # seconds
starting_temperature = 65  # degrees F
end_temperature = 209  # degrees F
cooling_coefficient = 0.001
e = 2.71828
max_rise_rate = 0.2  # degrees F / second

times = np.arange(0.0, max_time, time_step)
temperatures = []

actual_temperatures = []
average_temperatures = []
pwm_value = []
actual_times = []
starting_time = datetime.strptime("23:59:01", "%H:%M:%S")
with open('temperature_rise.json', 'r') as f:
    for line in f:
        data = json.loads(line)
        on_time = (datetime.strptime(data['time'], "%H:%M:%S") - starting_time).total_seconds()
        if on_time < 0:
            on_time += 24 * 60 * 60
        actual_temperatures.append(data['t1'])
        last_temperature = average_temperatures[-1] if len(average_temperatures) > 0 else data['t1']
        average_temperatures.append((last_temperature * .9) + (data['t1'] * .1))
        pwm_value.append(data['pwm'])
        actual_times.append(on_time)


def temperature(run_time):
    return end_temperature + (starting_temperature - end_temperature) * pow(e, -cooling_coefficient * run_time)


def temperature_rise(ambient_temperature, run_time):
    return ambient_temperature + (time_step * 0.1)


def get_temperature(current_temperature, time_step):
    return current_temperature + (time_step * 0.1)


for sample_time in times:
    last_temperature = temperatures[-1] if len(temperatures) > 0 else starting_temperature
    current_temperature = temperature(sample_time)
    temperatures.append(current_temperature)

# plt.plot(times, temperatures)
# plt.plot(actual_times, actual_temperatures)
temperature_derivative = np.gradient(actual_temperatures, actual_times)
plt.plot(actual_times, average_temperatures)
#plt.plot(actual_times, temperature_derivative)
# plt.plot(actual_times, pwm_value)
plt.show()
