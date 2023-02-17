import numpy as np
import matplotlib.pyplot as plt

traces = ["gcc", "leela", "linpack", "matmul_naive", "matmul_tiled", "mcf"]
num_runs = 179
num_params = 6

access_times = np.empty((len(traces), num_runs))
configs = np.empty((len(traces), num_runs, num_params), dtype=int)
for i, trace in enumerate(traces):
    with open(f'./data_second_pass/{trace}/access_times.txt') as f:
        access_times[i] = np.array(f.readlines())
    with open(f'./data_second_pass/{trace}/configs.txt') as f:
        for j, line in enumerate(f.readlines()):
            configs[i, j] = np.array(line.strip().split())

# Average over all traces
mean_times = np.mean(access_times, axis=0)
sorted_inds = np.argsort(mean_times)
sorted_times = np.sort(mean_times)
cache_sizes = np.empty((num_runs))
block_sizes = np.empty((num_runs))
set_sizes = np.empty((num_runs))
page_sizes = np.empty((num_runs))
tlb_sizes = np.empty((num_runs))
for i, ind in enumerate(sorted_inds):
    cache_sizes[i] = configs[0, ind, 0]
    block_sizes[i] = configs[0, ind, 1]
    set_sizes[i] = configs[0, ind, 2]
    page_sizes[i] = configs[0, ind, 3]
    tlb_sizes[i] = configs[0, ind, 4]

print("Top 50 Configurations")
with open("top_50_configs.txt", "w") as f:
    for i, ind in enumerate(sorted_inds[0:50]):
        config = " ".join([str(val) for val in configs[0, ind]])
        f.write(config + "\n")
        print(f"AAT: {mean_times[ind]}")
        print(f"CONF: {config}")

plt.title("Varying CBSTP")
plt.xlabel("AMAT")
plt.ylabel("log2(size)")
plt.plot(sorted_times, cache_sizes, label="C")
plt.plot(sorted_times, block_sizes, label="B")
plt.plot(sorted_times, set_sizes, label="S")
plt.plot(sorted_times, page_sizes, label="P")
plt.plot(sorted_times, tlb_sizes, label="T")
plt.legend()
plt.show()
