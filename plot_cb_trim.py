import numpy as np
import matplotlib.pyplot as plt

traces = ["gcc", "leela", "linpack", "matmul_naive", "matmul_tiled", "mcf"]
num_runs = 56
num_params = 6

access_times = np.empty((len(traces), num_runs))
configs = np.empty((len(traces), num_runs, num_params), dtype=int)
for i, trace in enumerate(traces):
    with open(f'./data_first_pass/{trace}/access_times.txt') as f:
        access_times[i] = np.array(f.readlines())
    with open(f'./data_first_pass/{trace}/configs.txt') as f:
        for j, line in enumerate(f.readlines()):
            configs[i, j] = np.array(line.strip().split())

plt.title("Varying CB")
plt.xlabel("log2(size)")
plt.ylabel("AMAT")

cache_sizes = np.arange(9, 16)
av_access_time = np.empty(cache_sizes.shape)
for i, cache_size in enumerate(cache_sizes):
    mask = configs[:, :, 0] == cache_size
    av_access_time[i] = np.sum(access_times[mask]) / sum(mask.flatten())

plt.plot(cache_sizes, av_access_time, label="C")

block_sizes = np.arange(4, 8)
av_access_time = np.empty(block_sizes.shape)
for i, block_size in enumerate(block_sizes):
    mask = configs[:, :, 1] == block_size
    av_access_time[i] = np.sum(access_times[mask]) / sum(mask.flatten())

plt.plot(block_sizes, av_access_time, label="B")

plt.legend()
plt.show()
