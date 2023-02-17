import numpy as np
import matplotlib.pyplot as plt
import IPython

traces = ["gcc", "leela", "linpack", "matmul_naive", "matmul_tiled", "mcf"]
num_runs = 290
num_params = 6

access_times = np.empty((len(traces), num_runs))
configs = np.empty((len(traces), num_runs, num_params), dtype=int)
for i, trace in enumerate(traces):
    with open(f'./data_final_pass/{trace}/access_times.txt') as f:
        access_times[i] = np.array(f.readlines())
    with open(f'./data_final_pass/{trace}/configs.txt') as f:
        for j, line in enumerate(f.readlines()):
            configs[i, j] = np.array(line.strip().split())

# Average over all traces
sorted_inds = np.argsort(access_times, axis=1)
sorted_times = np.sort(access_times, axis=1)
cache_sizes = np.empty((len(traces), num_runs))
block_sizes = np.empty((len(traces), num_runs))
set_sizes = np.empty((len(traces), num_runs))
page_sizes = np.empty((len(traces), num_runs))
tlb_sizes = np.empty((len(traces), num_runs))
memory_sizes = np.empty((len(traces), num_runs))
for i, trace in enumerate(traces):
    for j, ind in enumerate(sorted_inds[i]):
        cache_sizes[i, j] = configs[i, ind, 0]
        block_sizes[i, j] = configs[i, ind, 1]
        set_sizes[i, j] = configs[i, ind, 2]
        page_sizes[i, j] = configs[i, ind, 3]
        tlb_sizes[i, j] = configs[i, ind, 4]
        memory_sizes[i, j] = configs[i, ind, 5]

    print(f"Top 10 Configurations for {trace}")
    with open(f"top_10_configs_{trace}.txt", "w") as f:
        for ind in sorted_inds[i, 0:10]:
            config = " ".join([str(val) for val in configs[i, ind]])
            f.write(config + "\n")
            print(f"AAT: {access_times[i, ind]}")
            print(f"CONF: {config}")

    plt.subplot(321 + i)
    plt.title(f"Top 50 Samples, All M on Trace {trace}")
    plt.xlabel("AMAT")
    plt.ylabel("log2(size)")
    plt.plot(sorted_times[i], cache_sizes[i], label="C")
    plt.plot(sorted_times[i], block_sizes[i], label="B")
    plt.plot(sorted_times[i], set_sizes[i], label="S")
    plt.plot(sorted_times[i], page_sizes[i], label="P")
    plt.plot(sorted_times[i], tlb_sizes[i], label="T")
    plt.plot(sorted_times[i], memory_sizes[i], label="M")
    plt.legend()

plt.tight_layout()
plt.show()
