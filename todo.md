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

- [ ] Abstract the domain wall simulation into its own class or scene thing.
    - It should expose public attributes that ImGUI should allow users to configure.
    - It should be able to start, stop, restart, save and load.
        - [ ] Start button to start simulation.
        - [ ] Stop button to stop simulation.
        - [ ] Restart button to reset simulation back to original state.
        - [ ] Save button to save field configuration as a .ctdd file (file dialogs?).
        - [ ] Load button to load field configuration from a .ctdd file (file dialogs?).
