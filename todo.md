# Progress Overview #

- Added domain wall simulation (but very hacky and needs to be cleaned up), however comparing the results with the python version,
the simulations differ. Maybe check that the field loading is correct by creating data files with defined shapes.

- Could also create a function that can compare texture data.

## TODO ##

- [x] Set up test data files to check that loading is functional and consistent i.e. create fields with distinct patterns that
can be visually compared.
- [ ] Set up testing in the python version for all simulations i.e. save the field at steps 1, 2, 5, 10, 100, ... and compare
the simulation with the saved fields. Can't be done because arrays diverge slightly which propagates.
- [ ] Perform same tests with C++ version. Same as above.

- [x] Abstract the domain wall simulation into its own class or scene thing.
    - It should expose public attributes that ImGUI should allow users to configure.
    - It should be able to start, stop, restart, save and load.
        - [x] Start button to start simulation.
        - [x] Stop button to stop simulation.
        - [x] Restart button to reset simulation back to original state.
        - [x] Save button to save field configuration as a .ctdd file (file dialogs?).
        - [x] Load button to load field configuration from a .ctdd file (file dialogs?).

## Discussion Points ##

- Fixed the cosmic string simulation.
- Problems with the single axion simulation. Probably due to the inaccuracy/stability of the atan.
    - Simulations seem to be blow up in the C++ version despite being stable in the python version.
- Test images can be seen in:
    - "single_axion_target_seed1897198.png": The python version's result after running for 4000 timesteps with the following
    parameters:
        - lam = 10, color_anomaly = 4, K = 0.025, t0 = 75, growth = 2
    - "single_axion_cpp_seed1897198.png": The C++ version's result after running for 4000 timesteps with same parameters as above.

- Interesting things to note with the images:
    - The white regions are basically non-existent in the C++ version. There should be 4 distinct domains equal to 0, pi/2, -pi/2
    and pi i.e. white, red, blue and black. There are 4 colours in the C++ version but its 2 shades of red and 2 shades of blue.
        - Maybe save the field and check in the python notebook whether it looks correct/the domains are actually like that.

- Ideas to test/diagnose the bug. Hardcode the python version i.e. don't use potential_derivative_single_axion_*, and just write
the potential derivative directly into the evolve_acceleration function.
