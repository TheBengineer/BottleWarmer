import matplotlib.pyplot as plt
import numpy as np

max_time = 2 * 60 * 60  # 2 hours
time_step = .1  # seconds
starting_temperature = 65  # degrees F
end_temperature = 209  # degrees F
cooling_coefficient = 0.001
e = 2.71828

times = np.arange(0.0, max_time, time_step)
temperatures = []



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

plt.plot(times, temperatures)
plt.show()
