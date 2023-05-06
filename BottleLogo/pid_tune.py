import matplotlib.pyplot as plt
import numpy as np

max_time = 2 * 60 * 60  # 2 hours
time_step = .1  # seconds
starting_temperature = 65  # degrees F

times = np.arange(0.0, max_time, time_step)
temperatures = []


def get_temperature(current_temperature, time_step):
    return current_temperature + (time_step * 0.1)


for sample_time in times:
    last_temperature = temperatures[-1] if len(temperatures) > 0 else starting_temperature
    current_temperature = get_temperature(last_temperature, time_step)
    temperatures.append(current_temperature)

plt.plot(times, temperatures)
plt.show()
