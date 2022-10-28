import glob
import numpy as np
import numpy.typing as npt
from tqdm import tqdm

import matplotlib as mpl
from matplotlib import pyplot as plt
import matplotlib.figure
import matplotlib.axes

# TODO: Move those functions to here
from plot_string_count import get_string_count_file_header, parse_sc_data_file


def get_string_count_from_folder_names(
    folder_names: list[str], identifier_length: int
) -> tuple[dict[str, npt.NDArray[np.float32]], npt.NDArray[np.float32]]:
    start_timestep = 0
    # Dictionary that stores the string count of each folder using an abbreviation of the folder's name as the key
    string_count = {}
    # The time range that is valid with all trial data
    time_range_all = np.zeros(0)
    max_time_length = np.inf

    # Iterate over the folder names
    for folder_name in tqdm(folder_names):
        # Create an abbreviation of the folder using the last few letters
        short_identifier = "N"
        for n in folder_name[-identifier_length:]:
            short_identifier += n

        # Glob all trial data files
        string_count_file_names = list(
            glob.glob(f"{folder_name}/string_count_trial*.ctdsd")
        )
        # The number of trial runs
        num_trials = len(string_count_file_names)
        # Grab header information from a data file
        num_fields, max_timestep, dt = get_string_count_file_header(
            string_count_file_names[0]
        )
        # Time range of the data
        time_range = np.linspace(dt, dt * max_timestep, max_timestep)
        time_range = time_range[start_timestep:max_timestep]

        # Check if this time range is shorter than our current time range
        if (max_timestep - start_timestep) < max_time_length:
            max_time_length = max_timestep - start_timestep
            time_range_all = time_range

        # Store string count per timestep per field
        string_count[short_identifier] = np.zeros(
            shape=(num_fields, num_trials, max_timestep - start_timestep)
        )

        # Go through each trial's data
        for trial_idx, file_name in enumerate(string_count_file_names[:num_trials]):
            # Obtain the string count
            string_counts = parse_sc_data_file(file_name)

            for field_idx in range(num_fields):
                # Consider timesteps that are between start_timestep and max_timestep
                current_string_count = string_counts[
                    field_idx, start_timestep:max_timestep
                ]
                string_count[short_identifier][
                    field_idx, trial_idx, :
                ] = current_string_count

    return string_count, time_range_all


def get_average_string_count_from_dict(
    string_count: dict[str, npt.NDArray[np.float32]], all_fields: bool
) -> dict[str, npt.NDArray]:
    average_string_count = {}

    for identifier in string_count:
        current_string_count_data = string_count[identifier]
        if all_fields:
            average_string_count[identifier] = np.mean(
                current_string_count_data, axis=(0, 1)
            )
        else:
            average_string_count[identifier] = np.mean(
                current_string_count_data, axis=1
            )

    return average_string_count


def plot_average_string_count_log_log_from_dict(
    string_count: dict[str, npt.NDArray[np.float32]],
    time_range: npt.NDArray[np.float32],
    start_timestep: int,
    max_timestep: int,
) -> tuple[matplotlib.figure.Figure, matplotlib.axes.Axes]:
    fig = plt.figure(figsize=(16, 12))
    ax = fig.add_subplot(111)
    ax.set_xlabel(r"Time $\tau$")
    ax.set_ylabel("Average String Count")
    ax.set_xlim(time_range[start_timestep], time_range[max_timestep-1])
    ax.set_xscale("symlog")
    ax.set_yscale("symlog")

    for identifier in string_count:
        ax.plot(
            time_range[start_timestep:max_timestep],
            string_count[identifier][start_timestep:max_timestep],
            # c=(red, green, blue, alpha),
            linestyle="-",
            label=identifier,
            # zorder=order,
        )

    return fig, ax


def compute_pq_power_law_on_average(
    string_count, time_range, start_timestep, max_timestep
):
    exponent = []
    scale = []

    for identifier in string_count:
        log_pq_era_string_count = np.log(
            string_count[identifier][start_timestep:max_timestep]
        )
        log_pq_era_time = np.log(time_range[start_timestep:max_timestep])
        (m, b) = np.polyfit(log_pq_era_time, log_pq_era_string_count, 1)
        exponent.append(m)
        scale.append(np.exp(b))

    # Convert to numpy arrays
    exponent = np.array(exponent)
    scale = np.array(scale)

    exponent_mean = np.mean(exponent)
    exponent_error = np.std(exponent)
    scale_mean = np.mean(scale)
    scale_error = np.std(scale)

    return exponent_mean, exponent_error, scale_mean, scale_error


def compute_pq_power_law_on_trials(
    string_count_trials, time_range, start_timestep, max_timestep
):
    exponent = []
    scale = []

    for identifier in string_count_trials:
        log_pq_era_string_count = np.log(
            string_count_trials[identifier][:, :, start_timestep:max_timestep]
        )
        log_pq_era_time = np.log(time_range[start_timestep:max_timestep])

        num_fields, num_trials, _ = string_count_trials[identifier].shape

        for field_idx in range(num_fields):
            for trial_idx in range(num_trials):
                (m, b) = np.polyfit(
                    log_pq_era_time, log_pq_era_string_count[field_idx, trial_idx, :], 1
                )
                exponent.append(m)
                scale.append(np.exp(b))

    # Convert to numpy arrays
    exponent = np.array(exponent)
    scale = np.array(scale)

    exponent_mean = np.mean(exponent)
    exponent_error = np.std(exponent)
    scale_mean = np.mean(scale)
    scale_error = np.std(scale)

    return exponent, exponent_mean, exponent_error, scale, scale_mean, scale_error
