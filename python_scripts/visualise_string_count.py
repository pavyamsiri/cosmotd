from re import T
import struct
import numpy as np
from matplotlib import pyplot as plt
import sys
from scipy.optimize import curve_fit


def power_law_func(x, a, q, c):
    return a * x ** (-q) + c


def parse_sc_data_file(file_name):
    with open(file_name, "rb") as save_file:
        # The number of string fields
        num_string_fields = struct.unpack("<I", save_file.read(4))[0]
        num_timesteps = struct.unpack("<I", save_file.read(4))[0]

        string_counts = np.zeros(shape=(num_string_fields, num_timesteps))

        for string_idx in range(num_string_fields):
            for timestep_idx in range(num_timesteps):
                string_counts[string_idx][timestep_idx] = struct.unpack(
                    "<i", save_file.read(4)
                )[0]
    return string_counts


if __name__ == "__main__":
    # if len(sys.argv) > 1:
    #     file_name = sys.argv[1]
    # else:
    #     file_name = "default.data"

    num_trials = 100
    start_time = 25
    time_range = np.linspace(0.1, 0.1 * 2700, 2700)
    time_range = time_range[start_time:]
    phi_data = np.zeros(shape=(num_trials, 2700 - start_time))
    psi_data = np.zeros(shape=(num_trials, 2700 - start_time))

    phi_power_scale = np.zeros(num_trials)
    phi_power_laws = np.zeros(num_trials)
    phi_power_offset = np.zeros(num_trials)
    psi_power_scale = np.zeros(num_trials)
    psi_power_laws = np.zeros(num_trials)
    psi_power_offset = np.zeros(num_trials)

    for trial_idx in range(num_trials):
        file_name = f"sc_data/string_count_trial{trial_idx}.data"
        string_counts = parse_sc_data_file(file_name)
        phi_data[trial_idx, :] = string_counts[0, start_time:]
        psi_data[trial_idx, :] = string_counts[1, start_time:]

        (a, q, c), _ = curve_fit(
            power_law_func, time_range, string_counts[0, start_time:]
        )
        phi_power_scale[trial_idx] = a
        phi_power_laws[trial_idx] = q
        phi_power_offset[trial_idx] = c
        (a, q, c), _ = curve_fit(
            power_law_func, time_range, string_counts[1, start_time:]
        )
        psi_power_scale[trial_idx] = a
        psi_power_laws[trial_idx] = q
        psi_power_offset[trial_idx] = c

    phi_mean = np.mean(phi_data, axis=0)
    phi_error = np.std(phi_data, axis=0) / np.sqrt(num_trials)
    phi_low_percentile = np.percentile(phi_data, 5, 0)
    phi_high_percentile = np.percentile(phi_data, 95, 0)
    psi_mean = np.mean(psi_data, axis=0)
    psi_error = np.std(psi_data, axis=0) / np.sqrt(num_trials)
    psi_low_percentile = np.percentile(psi_data, 5, 0)
    psi_high_percentile = np.percentile(psi_data, 95, 0)

    fig = plt.figure(figsize=(12, 12))
    ax = fig.add_subplot(121)
    ax_power_laws = fig.add_subplot(122)

    ax.loglog(
        range(2700 - start_time),
        power_law_func(
            time_range,
            np.percentile(phi_power_scale, 50, 0),
            np.percentile(phi_power_laws, 50, 0),
            np.percentile(phi_power_offset, 50, 0),
        ),
        "--r",
    )
    ax.loglog(
        range(2700 - start_time),
        power_law_func(
            time_range,
            np.percentile(psi_power_scale, 50, 0),
            np.percentile(psi_power_laws, 50, 0),
            np.percentile(psi_power_offset, 50, 0),
        ),
        "--b",
    )
    ax.fill_between(
        range(2700 - start_time),
        phi_low_percentile,
        phi_high_percentile,
        label=r"$\phi$",
    )
    ax.fill_between(
        range(2700 - start_time),
        psi_low_percentile,
        psi_high_percentile,
        label=r"$\psi$",
    )
    ax.set_xlabel("Timestep")
    ax.set_ylabel("String count")
    # ax.set_xscale("log")
    # ax.set_yscale("log")

    ax_power_laws.boxplot(np.column_stack((phi_power_laws, psi_power_laws)))
    ax_power_laws.set_title("Power law distribution")

    ax.legend()
    plt.show()

    plt.close(fig)
