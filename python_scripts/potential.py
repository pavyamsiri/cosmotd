from typing import TypeAlias

import matplotlib.figure
from matplotlib import pyplot as plt
import numpy as np
from numpy import typing as npt

MplFigure: TypeAlias = matplotlib.figure.Figure


def potential_ca(
    phi_real: npt.NDArray[np.float32],
    phi_imaginary: npt.NDArray[np.float32],
    psi_real: npt.NDArray[np.float32],
    psi_imaginary: npt.NDArray[np.float32],
    eta: float,
    lam: float,
    n: float,
    n_prime: float,
    m: float,
    m_prime: float,
    K: float,
    kappa: float,
) -> npt.NDArray[np.float32]:
    """Calculates the companion axion potential including both the Peccei-Quinn potential terms and the axion potential
    terms.

    Parameters
    ----------
    phi_real : npt.NDArray[np.float32]
        the real part of the field phi.
    phi_imaginary : npt.NDArray[np.float32]
        the imaginary part of the field phi.
    psi_real : npt.NDArray[np.float32]
        the real part of the field psi.
    psi_imaginary : npt.NDArray[np.float32]
        the imaginary part of the field psi.
    eta : float
        the location of the symmetry broken minima.
    lam : float
        the 'mass' of the field. Related to the width `w` of the walls by the equation lambda = 2*pi^2/w^2.
    n : float
        the first color anomaly coefficient of the phi field.
    n_prime : float
        the first color anomaly coefficient of the psi field.
    m : float
        the second color anomaly coefficient of the phi field.
    m_prime : float
        the second color anomaly coefficient of the psi field.
    K : float
        the strength of the axion potential.
    kappa : float
        the strength of the second axion potential term relative to the other axion potential.

    Returns
    -------
    potential : npt.NDArray[np.float32]
        the full companion axion potential.
    """

    # Compute phase
    phi_phase = np.arctan2(phi_imaginary, phi_real)
    psi_phase = np.arctan2(psi_imaginary, psi_real)
    # PQ potentials
    potential = lam / 4 * (phi_real**2 + phi_imaginary**2 - eta**2) ** 2
    potential += lam / 4 * (psi_real**2 + psi_imaginary**2 - eta**2) ** 2
    # Companion axion potential
    potential += -2 * K * np.cos(n * phi_phase + n_prime * psi_phase)
    potential += -2 * kappa * K * np.cos(m * phi_phase + m_prime * psi_phase)

    return potential


def potential_ca_axion_only(
    phi_phase: npt.NDArray[np.float32],
    psi_phase: npt.NDArray[np.float32],
    n: float,
    n_prime: float,
    m: float,
    m_prime: float,
    K: float,
    kappa: float,
) -> npt.NDArray[np.float32]:
    """Calculates the companion axion potential including only the axion potential terms (cosine terms).

    Parameters
    ----------
    phi_phase : npt.NDArray[np.float32]
        the phase of the field phi.
    psi_phase : npt.NDArray[np.float32]
        the phase of the field phi.
    n : float
        the first color anomaly coefficient of the phi field.
    n_prime : float
        the first color anomaly coefficient of the psi field.
    m : float
        the second color anomaly coefficient of the phi field.
    m_prime : float
        the second color anomaly coefficient of the psi field.
    K : float
        the strength of the axion potential.
    kappa : float
        the strength of the second axion potential term relative to the other axion potential.

    Returns
    -------
    potential : npt.NDArray[np.float32]
        the axion potential terms of the full companion axion potential.
    """
    potential = -2 * K * np.cos(n * phi_phase + n_prime * psi_phase)
    potential += -2 * kappa * K * np.cos(m * phi_phase + m_prime * psi_phase)

    return potential


def generate_models(
    min_value: int, max_value: int, unique: bool = True
) -> list[tuple[int, int, int, int]]:
    """Generates a list of colour anomaly model configurations for the companion axion model. The colour anomalies will only
    take on integer values from given minimum value to the given maximum value.

    Parameters
    ----------
    min_value : int
        the minimum integer value the colour anomaly coefficients can take.
    max_value : int
        the maximum integer value the colour anomaly coefficients can take.
    unique : bool
        if `True` the generated list will omit duplicated model configurations, that is, model configurations that are
        equivalent to another upon swaps of the two axion fields. Default is `True`.

    Returns
    -------
    models : list[tuple[int, int, int, int]]
        the list of generated model configurations.
    """
    models = []
    # Loop over all permutations
    for n in range(min_value, max_value + 1):
        for n_prime in range(min_value, max_value + 1):
            for m in range(min_value, max_value + 1):
                for m_prime in range(min_value, max_value + 1):
                    # Model configurations must satisfy NN_g' != N'N_g to be valid
                    if (n * m_prime) == (n_prime * m):
                        continue
                    else:
                        # If unique models are required, check that the swapped configuration hasn't been added yet
                        reverse_identifier = (n_prime, n, m_prime, m)
                        if reverse_identifier in models and unique:
                            continue
                        # Otherwise add the model to the list
                        else:
                            identifier = (n, n_prime, m, m_prime)
                            models.append(identifier)
    return models


def get_axion_potential_and_minima(
    n: int, n_prime: int, m: int, m_prime: int, K: float, kappa: float, num_samples: int
) -> tuple[
    npt.NDArray[np.float32],
    npt.NDArray[np.float32],
    npt.NDArray[np.float32],
    npt.NDArray[np.float32],
]:
    """Computes the companion axion potential (axion potential only) and the points which lie in the global minimum.

    Parameters
    ----------
    n : float
        the first color anomaly coefficient of the phi field.
    n_prime : float
        the first color anomaly coefficient of the psi field.
    m : float
        the second color anomaly coefficient of the phi field.
    m_prime : float
        the second color anomaly coefficient of the psi field.
    K : float
        the strength of the axion potential.
    kappa : float
        the strength of the second axion potential term relative to the other axion potential.
    num_samples : int
        the number of samples to use in the phase grid.

    Returns
    -------
    companion_axion_potential : npt.NDArray[np.float32]
        the axion potential of the companion axion model.
    min_indices : npt.NDArray[np.float32]
        the indices of the grid points which are lying in the global minimum of the potential.
    phi_phase_range : npt.NDArray[np.float32]
        the range of values taken by the phase of the phi field.
    psi_phase_range : npt.NDArray[np.float32]
        the range of values taken by the phase of the psi field.
    """
    # Create 2D grid of phase values
    phase_range = np.linspace(-np.pi, +np.pi, num_samples)
    phi_phase_range, psi_phase_range = np.meshgrid(phase_range, phase_range)

    # Compute the axion potential of the companion axion model
    companion_axion_potential = potential_ca_axion_only(
        phi_phase_range,
        psi_phase_range,
        n,
        n_prime,
        m,
        m_prime,
        K,
        kappa,
    )

    # The theoretical minimum of the potential
    theoretial_minimum = -(2 * K) - (2 * kappa * K)

    # Find which grid points lie in the global minimum
    min_indices = np.argwhere(np.isclose(companion_axion_potential, theoretial_minimum))

    return companion_axion_potential, min_indices, phi_phase_range, psi_phase_range


def plot_axion_potential_phase_only(
    n: int,
    n_prime: int,
    m: int,
    m_prime: int,
    K: float,
    kappa: float,
    num_samples: int,
    num_levels: int,
) -> tuple[MplFigure, npt.NDArray[np.float32]]:
    """Plots the companion axion potential (axion potential only) and highlights its minimum. Also returns the plot figure and
    the points which lie in the global minimum.

    Parameters
    ----------
    n : float
        the first color anomaly coefficient of the phi field.
    n_prime : float
        the first color anomaly coefficient of the psi field.
    m : float
        the second color anomaly coefficient of the phi field.
    m_prime : float
        the second color anomaly coefficient of the psi field.
    K : float
        the strength of the axion potential.
    kappa : float
        the strength of the second axion potential term relative to the other axion potential.
    num_samples : int
        the number of samples to use in the phase grid.
    num_levels : int
        the number of contour levels i.e. the resolution of the colour bar.

    Returns
    -------
    fig : matplotlib.figure.Figure
        the plot figure.
    min_indices : npt.NDArray[np.float32]
        the indices of the grid points which are lying in the global minimum of the potential.
    """
    # Get the companion axion potential and minimum points
    (
        potential,
        min_indices,
        phi_phase_range,
        psi_phase_range,
    ) = get_axion_potential_and_minima(n, n_prime, m, m_prime, K, kappa, num_samples)

    # Create figure and axes
    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111)

    # Plot the axion potential
    potential_map = ax.contourf(
        phi_phase_range, psi_phase_range, potential, levels=num_levels
    )

    # Plot the minima points
    for (min_x, min_y) in min_indices:
        ax.scatter(
            phi_phase_range[min_x, min_y], psi_phase_range[min_x, min_y], 100, "r"
        )

    # Formatting
    ax.axis("scaled")
    ax.set_xlabel(r"$\theta$")
    ax.set_ylabel(r"$\theta'$")
    ax.set_title(rf"$N={n}, N'={n_prime}, N_g={m}, N_g'={m_prime}$")
    _ = fig.colorbar(potential_map, fraction=0.046, pad=0.04, label=r"$V(\phi, \psi)$")

    return fig, min_indices


def count_unique_minima(
    n: int,
    n_prime: int,
    m: int,
    m_prime: int,
    K: float,
    kappa: float,
    num_samples: int,
    tolerance: float = 0.3,
) -> int:
    """Counts the number of unique minima for the given companion axion potential. This is done by grouping points that
    are close to each other within a tolerance as belonging to the same minimum.

    Parameters
    ----------
    n : float
        the first color anomaly coefficient of the phi field.
    n_prime : float
        the first color anomaly coefficient of the psi field.
    m : float
        the second color anomaly coefficient of the phi field.
    m_prime : float
        the second color anomaly coefficient of the psi field.
    K : float
        the strength of the axion potential.
    kappa : float
        the strength of the second axion potential term relative to the other axion potential.
    num_samples : int
        the number of samples to use in the phase grid.
    tolerance : float
        the maximum distance two points needs to be from each other to be classed as belongin to different local minima.
        Default is 0.3.

    Returns
    -------
    count : int
        the number of unique minima.
    """
    # Get the index positions of all minimum points for the given companion axion potential
    _, min_indices, phi_phase_range, psi_phase_range = get_axion_potential_and_minima(
        n, n_prime, m, m_prime, K, kappa, num_samples
    )

    # Initialise minima count
    count = 0
    # All points start off as unaccounted
    unaccounted = [True for _ in min_indices]

    # For all minima points
    for idx, ((min_x, min_y), current_flag) in enumerate(zip(min_indices, unaccounted)):
        # If unaccounted, perform check
        if current_flag:
            # Increment count as this must be a new minimum
            count += 1

            # Account for points that are nearby within a radius equal to tolerance. Only perform this check if this is not
            # the last point.
            if idx < len(min_indices):
                # Current point
                current_theta = phi_phase_range[min_x, min_y]
                current_theta_prime = psi_phase_range[min_x, min_y]

                next_idx = idx + 1
                for other_idx, (other_x, other_y) in zip(
                    range(next_idx, len(min_indices)), min_indices[next_idx:]
                ):
                    # Other point
                    other_theta = phi_phase_range[other_x, other_y]
                    other_theta_prime = psi_phase_range[other_x, other_y]

                    # Calculate naive displacement along theta and theta' axes
                    theta_distance = np.abs(current_theta - other_theta)
                    theta_prime_distance = np.abs(
                        current_theta_prime - other_theta_prime
                    )

                    # Distances larger than pi must be wrapped back around as phases are periodic
                    if theta_distance > np.pi:
                        theta_distance = 2 * np.pi - theta_distance
                    if theta_prime_distance > np.pi:
                        theta_prime_distance = 2 * np.pi - theta_prime_distance

                    # Compute the Euclidean distance squared
                    total_distance_squared = (
                        theta_distance**2 + theta_prime_distance**2
                    )
                    # If distance is smaller than the radius = tolerance, then this point lies within the same minimum
                    if total_distance_squared < tolerance**2:
                        unaccounted[other_idx] = False
        # Skip as we have already accounted for this
        else:
            continue

    return count
