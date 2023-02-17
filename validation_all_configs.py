import numpy as np
import matplotlib.pyplot as plt

traces = ["gcc", "leela", "matmul_naive", "matmul_tiled", "mcf"]
num_runs = 24396
num_params = 6

access_times = np.empty((len(traces), num_runs))
configs = np.empty((len(traces), num_runs, num_params), dtype=int)
for i, trace in enumerate(traces):
    with open(f'./data_all_configs/{trace}/access_times.txt') as f:
        access_times[i] = np.array(f.readlines())
    with open(f'./data_all_configs/{trace}/configs.txt') as f:
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
memory_sizes = np.empty((num_runs))
for i, ind in enumerate(sorted_inds):
    cache_sizes[i] = configs[0, ind, 0]
    block_sizes[i] = configs[0, ind, 1]
    set_sizes[i] = configs[0, ind, 2]
    page_sizes[i] = configs[0, ind, 3]
    tlb_sizes[i] = configs[0, ind, 4]
    memory_sizes[i] = configs[0][ind][5]

print("Top 10 Configurations")
with open("top_10_configs_entire_space.txt", "w") as f:
    for i, ind in enumerate(sorted_inds[0:10]):
        config = " ".join([str(val) for val in configs[0, ind]])
        f.write(config + "\n")
        print(f"AAT: {mean_times[ind]}")
        print(f"CONF: {config}")

plt.subplot(321)
plt.tight_layout()
plt.title("C")
plt.xlabel("AMAT")
plt.ylabel("log2(size)")
plt.scatter(sorted_times, cache_sizes, color="C0")
plt.subplot(322)
plt.title("B")
plt.xlabel("AMAT")
plt.ylabel("log2(size)")
plt.scatter(sorted_times, block_sizes, color="C1")
plt.subplot(323)
plt.title("S")
plt.xlabel("AMAT")
plt.ylabel("log2(size)")
plt.scatter(sorted_times, set_sizes, color="C2")
plt.subplot(324)
plt.title("P")
plt.xlabel("AMAT")
plt.ylabel("log2(size)")
plt.scatter(sorted_times, page_sizes, color="C3")
plt.subplot(325)
plt.title("T")
plt.xlabel("AMAT")
plt.ylabel("log2(size)")
plt.scatter(sorted_times, tlb_sizes, color="C4")
plt.subplot(326)
plt.title("M")
plt.xlabel("AMAT")
plt.ylabel("log2(size)")
plt.scatter(sorted_times, memory_sizes, color="C5")
plt.show()
