# Standard libraries
import argparse
import glob
import struct
import sys
import warnings

# External libraries
import pandas as pd
from matplotlib import pyplot as plt
import numpy as np
from scipy.optimize import curve_fit
from tqdm import tqdm

# Internal libraries


# Plotting configuration constants
LINE_STYLES = ["--", "--"]
LINE_COLORS = ["blue", "red"]
FILL_COLORS = ["tab:blue", "tab:orange"]
FIELD_NAMES = ["phi", "psi"]


def main(args: list[str]):
    # Parse CL arguments
    parsed_args = _parse_args(args)
    folder_name = parsed_args.data_folder
    start_time = parsed_args.min_timestep
    end_time = parsed_args.max_timestep

    # Grab all trial data files, assuming they conform to the set naming scheme
    string_count_file_names = list(
        glob.glob(f"{folder_name}/string_count_trial*.ctdsd")
    )

    # Exit out if there are no such data files
    num_trials = len(string_count_file_names)
    if num_trials == 0:
        print(f"The given folder {folder_name} does not contain any string count data!")
        return

    # Grab header information from a data file
    num_fields, max_time, dt = get_string_count_file_header(string_count_file_names[0])
    # The maximum time is either the given maximum time or the data's maximum time
    max_time = min(end_time, max_time)
    # Time range of the data
    time_range = np.linspace(dt, dt * max_time, max_time)
    time_range = time_range[start_time:max_time]

    # Store string count per timestep per field
    string_count_data = []
    # Store scale coefficient `a` per timestep per field
    power_scale_data = []
    # Store power law exponent `q` per timestep per field
    power_law_data = []
    # Store constant offset `c` per timestep per field
    power_offset_data = []
    for _ in range(num_fields):
        string_count_data.append(np.zeros(shape=(num_trials, max_time - start_time)))
        power_scale_data.append(np.zeros(num_trials))
        power_law_data.append(np.zeros(num_trials))
        power_offset_data.append(np.zeros(num_trials))

    # Go through each file and curve fit a power law
    for trial_idx, file_name in tqdm(
        enumerate(string_count_file_names[:num_trials]), total=num_fields
    ):
        string_counts = parse_sc_data_file(file_name)

        for field_idx in range(num_fields):
            # Consider only times between start_time and end_time
            current_string_count = string_counts[field_idx, start_time:max_time]
            string_count_data[field_idx][trial_idx, :] = current_string_count

            with warnings.catch_warnings():
                # Ignore curve fit warnings
                warnings.simplefilter("ignore")
                # Fit a power law to the data
                (a, q, c), _ = curve_fit(
                    power_law_func,
                    time_range,
                    current_string_count,
                    # bounds=((-np.inf, -np.inf, -np.inf), (np.inf, np.inf, np.inf)),
                    bounds=((-np.inf, -np.inf, 0), (np.inf, np.inf, np.inf)),
                )
                # Store fits
                power_scale_data[field_idx][trial_idx] = a
                power_law_data[field_idx][trial_idx] = q
                power_offset_data[field_idx][trial_idx] = c

    # Create figures and axes
    fig_power_law = plt.figure(figsize=(12, 12))
    ax_power_law = fig_power_law.add_subplot(111)
    fig_power_exponent = plt.figure(figsize=(12, 12))
    ax_power_exponent = fig_power_exponent.add_subplot(111)

    # Store power law exponent data to save later
    power_law_fits = {
        "exponent_median": [],
        "exponent_error": [],
        "offset_median": [],
        "offset_error": [],
    }

    for field_idx in range(num_fields):
        # 2 sigma bounds
        string_lower_bound = np.percentile(string_count_data[field_idx], 16, 0)
        string_upper_bound = np.percentile(string_count_data[field_idx], 84, 0)

        # Median of curve fit values
        scale_median = np.median(power_scale_data[field_idx])
        exponent_median = np.median(power_law_data[field_idx])
        offset_median = np.median(power_offset_data[field_idx])
        # Mean of curve fit values
        # scale_mean = np.mean(power_scale_data[field_idx])
        # exponent_mean = np.mean(power_law_data[field_idx])
        # offset_mean = np.mean(power_offset_data[field_idx])
        # Error of curve fit values
        exponent_error = np.std(power_law_data[field_idx])
        offset_error = np.std(power_offset_data[field_idx])

        # Store median and error
        power_law_fits["exponent_median"].append(exponent_median)
        power_law_fits["exponent_error"].append(exponent_error)
        power_law_fits["offset_median"].append(offset_median)
        power_law_fits["offset_error"].append(offset_error)

        # NOTE: plotted from start_time to end_time however this looks worse than plotting 0 to end_time - start_time. What to
        # choose?
        # Plot power law fit using median values
        ax_power_law.plot(
            range(start_time, max_time),
            power_law_func(
                time_range,
                scale_median,
                exponent_median,
                offset_median,
            ),
            linestyle=LINE_STYLES[field_idx],
            color=LINE_COLORS[field_idx],
            label=f"$\\{FIELD_NAMES[field_idx]}$ power law fit",
        )
        # Plot the string count data as a range using the above lower and upper bounds
        ax_power_law.fill_between(
            range(start_time, max_time),
            string_lower_bound,
            string_upper_bound,
            label=f"$\\{FIELD_NAMES[field_idx]}$ data",
            color=FILL_COLORS[field_idx],
            alpha=0.5,
        )

        # Print result with error rounded to 1 significant figure
        num_leading_zeros = int(-np.floor(np.log10(exponent_error)))
        exponent_median_rounded = np.around(exponent_median, decimals=num_leading_zeros)
        exponent_error_1sf = np.format_float_positional(
            exponent_error, precision=1, unique=False, fractional=False, trim="k"
        )
        print(
            f"The median exponent of {FIELD_NAMES[field_idx]} is {exponent_median_rounded}+-{exponent_error_1sf}"
        )

    # Plotting configuration
    ax_power_law.set_xscale("symlog")
    ax_power_law.set_yscale("symlog")
    ax_power_law.set_xlim(start_time, end_time)
    ax_power_law.set_xlabel("Timestep")
    ax_power_law.set_ylabel("String count")

    # Plot boxplot of exponents
    ax_power_exponent.boxplot(
        np.column_stack([power_law_dist for power_law_dist in power_law_data])
    )
    ax_power_exponent.set_title("Power law distribution")
    ax_power_law.legend()

    # Show, save and close
    plt.show()
    fig_power_law.savefig(f"{folder_name}/power_law_fit.png")
    fig_power_exponent.savefig(f"{folder_name}/boxplot.png")
    plt.close(fig_power_law)
    plt.close(fig_power_exponent)

    # Write out exponent data as .csv
    exponent_df = pd.DataFrame.from_dict(power_law_fits)
    exponent_df.index.name = "Field"
    exponent_df.to_csv(f"{folder_name}/power_law_exponent.csv")


def _parse_args(args: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="This CLI program is used to plot the string count of a field simulation."
    )
    parser.add_argument(
        "data_folder",
        help="The name of the folder that contains the string count data.",
    )
    parser.add_argument(
        "min_timestep",
        type=int,
        help="""The minimum timestep to consider. Due to the initial field configuration being normally distributed,
        string counts are not meaningful for ~4 unit times.""",
    )
    parser.add_argument(
        "max_timestep",
        type=int,
        help="The maximum timestep to consider. For most simulations after 280 unit times, the field becomes unstable.",
    )

    parsed_args = parser.parse_args(args)
    return parsed_args


def get_string_count_file_header(file_name: str) -> tuple[int, int, float]:
    num_timesteps = 0
    with open(file_name, "rb") as save_file:
        # The number of string fields
        num_string_fields = struct.unpack("<I", save_file.read(4))[0]
        num_timesteps = struct.unpack("<I", save_file.read(4))[0]
        dt = struct.unpack("<f", save_file.read(4))[0]

    return (num_string_fields, num_timesteps, dt)


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
    args = sys.argv[1:]
    main(args)
