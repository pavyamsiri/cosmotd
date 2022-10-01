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

- Fitting power laws to string counts that tend to 0. Requires negative offset which doesn't make much sense i.e. implies negative
string counts. Can't fit a power law to this.
- Can't log and use y = mx + b because that implies the power law goes y = ax^q which means offset is always zero. This is not
true for simulations which tend to some stable string count (positive offset). Log of y = ax^q + c is not trivial in terms
as the + c can't be easily separated.
- Configurations that have bad fits:
    - N1112
    - N1121
    - N1123
    - N1132
    - N1211
    - N1213
    - N1221
    - N1223
    - N1232
    - N1233
    - N2111
    - N2112
    - N2113
    - N2122
    - N2123
    - N2131
    - N2132
    - N2133
    - N2212 (string detection is too sensitive on curved surfaces)
    - N2232*
- TODO: Field configurations to test:
    - [X] N1112
    - [X] N1113
    - [X] N1121
    - [X] N1123
    - [X] N1131
    - [X] N1132
    - [X] N1211
    - [X] N1213
    - [X] N1221
    - [X] N1222
    - [X] N1223
    - [X] N1231
    - [X] N1232
    - [X] N1233
    - [X] N1311
    - [X] N1312
    - [X] N1321
    - [X] N1322
    - [X] N1323
    - [X] N1331
    - [X] N1332
    - [X] N1333
    - [X] N2111
    - [X] N2112
    - [X] N2113
    - [X] N2122
    - [X] N2123
    - [X] N2131
    - [X] N2132
    - [X] N2133
    - [X] N2212
    - [X] N2213
    - [X] N2221
    - [X] N2223
    - [X] N2231
    - [X] N2232
    - [X] N2311
    - [X] N2312
    - [X] N2313
    - [X] N2321
    - [X] N2322
    - [X] N2331
    - [X] N2332
    - [X] N2333
    - [X] N3111
    - [X] N3112
    - [X] N3113
    - [X] N3121
    - [X] N3122
    - [X] N3123
    - [X] N3132
    - [X] N3133
    - [X] N3211
    - [X] N3212
    - [X] N3221
    - [X] N3222
    - [X] N3223
    - [X] N3231
    - [X] N3233
    - [X] N3312
    - [X] N3313
    - [X] N3321
    - [X] N3323
    - [X] N3331
    - [X] N3332
