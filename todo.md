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
    - [x] It should expose public attributes that ImGUI should allow users to configure.
    - It should be able to start, stop, restart, save and load.
        - [x] Start button to start simulation.
        - [x] Stop button to stop simulation.
        - [ ] Restart button to reset simulation back to original state.
        - [ ] Save button to save field configuration as a .ctdd file (file dialogs?).
        - [ ] Load button to load field configuration from a .ctdd file (file dialogs?).

## Ideas for Simulation Abstraction ##

- Make simulation class generic over all simulations.
- Done by having vectors storing various data types (for uniforms)
    - i.e. `std::vector<float>` for all floats, float2, float3, float4 etc.
- Configure by providing a layout like vertex buffer layout.
- Each step would just then iterate over all fields (images) and then apply the uniforms according to the layout.

## Discussion Points ##

- I noticed a bug in the python version, the velocity update code used dt instead of dt^2.
    - Since changing that I noticed that the evolution has changed.
        - Most of the time, the evolution is just slower but not too different.
        - Single and companion axion simulations seem to be different however. Can run some parameters are bit higher i.e.
            the K value.
- Tested the C++ version against the python version by using the same starting conditions.
    - Results differed. C++ does not seem to dissipate and gets locked into a stable configuration (or at least evolution is so
    slow that it is imperceivable).
    - Tried to develop tests for python version to ensure consistency.
        - Tests were unsuccessful because the floats in the arrays diverged despite evolution being deterministic.
            - File saving and loading seems to work fine though. Tried the test on a calibration file.
- Having trouble with simulation code when changing some things. Weird behaviour on the new architecture.
