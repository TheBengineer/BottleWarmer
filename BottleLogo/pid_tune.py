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
env_temperatures = []
pwm_value = []
actual_times = []
target_temps = []

with open('temperature_rise.json', 'r') as f:
    lines = f.readlines()
    first_line = json.loads(lines[0])
    starting_time = datetime.strptime(first_line["time"], "%H:%M:%S")
    last_time = starting_time
    for line in lines:
        data = json.loads(line)
        on_time = datetime.strptime(data['time'], "%H:%M:%S")
        if 60 > (last_time - on_time).total_seconds() > -60:
            continue
        lat_time = on_time
        if on_time:
            actual_temperatures.append(data['t1'])
            env_temperatures.append(data['t2'])
            last_temperature = average_temperatures[-1] if len(average_temperatures) > 0 else data['t1']
            average_temperatures.append((last_temperature * .9) + (data['t1'] * .1))
            pwm_value.append(data['pwm'])
            actual_times.append(on_time)
            if "target" in data:
                target_temps.append(data["target"])
            else:
                target_temps.append(data["set"])


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

# temperature_derivative = np.gradient(actual_temperatures, actual_times)

# plt.plot(times, temperatures)
plt.plot(actual_times, target_temps)
plt.plot(actual_times, actual_temperatures)
# plt.plot(actual_times, env_temperatures)
# plt.plot(actual_times, average_temperatures)
# plt.plot(actual_times, temperature_derivative)
# plt.plot(actual_times, pwm_value)
plt.gcf().autofmt_xdate()
plt.show()
