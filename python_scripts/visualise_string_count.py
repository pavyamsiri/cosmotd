import struct
import numpy as np
from matplotlib import pyplot as plt
import sys


if __name__ == "__main__":
    if len(sys.argv) > 1:
        file_name = sys.argv[1]
    else:
        file_name = "default.data"

    with open(file_name, "rb") as save_file:
        # The number of fields
        num_timesteps = struct.unpack("<I", save_file.read(4))[0]
        string_counts = np.zeros(num_timesteps)

        for timestep in range(num_timesteps):
            string_counts[timestep] = struct.unpack("<i", save_file.read(4))[0]

    fig = plt.figure(figsize=(12, 12))
    ax_normal = fig.add_subplot(121)
    ax_loglog = fig.add_subplot(122)

    string_counts = string_counts[250:]

    ax_normal.plot(range(len(string_counts)), string_counts)
    ax_normal.set_title(file_name)
    ax_normal.set_xlabel("Timestep")
    ax_normal.set_ylabel("String count")

    ax_loglog.loglog(range(len(string_counts)), string_counts)
    ax_loglog.set_title(file_name)
    ax_loglog.set_xlabel("Timestep")
    ax_loglog.set_ylabel("String count")

    plt.show()

    plt.close(fig)
