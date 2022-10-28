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
- Configurations that have bad fits (TODO: Categorise):
    <!-- Z -> Goes to zero -->
    <!-- S -> Strange pattern -->
    <!-- C -> Close to zero but not zero -->
    <!-- H -> One field is zero or almost zero but the other field is not -->
    <!-- N -> Not going to disperse -->
    - N1112 [Z]
    - N1121 [Z]
    - N1123 [Z]
    - N1132 [Z]
    - N1211 [Z]
    - N1213 [Z]
    - N1221 [S]
    - N1222 [S] (There is a weird loop like thing)
    - N1223 [Z]
    - N1231 [S]
    - N1232 [S]
    - N1233 [S]
    - N1312 [C]
    - N2111 [Z]
    - N2112 [S]
    - N2113 [S]
    - N2122 [S]
    - N2123 [S]
    - N2131 [Z]
    - N2132 [Z]
    - N2133 [S]
    - N2212 [S]
    - N2213 [S]
    - N2221 [H]
    - N2223 [H]
    - N2231 [N]
    - N2232 [H]
- N = 1 or N' = 1 configurations:
    - N1113 does not disperse but N1213 does. N1413 does not seem to disperse either.
    - N1131 does not disperse.
    - N1221 does disperse but not always to zero. It does not always disperse (at least within the time range used).
    - N1231 does disperse but not always to zero.

U - Unstable Walls (Single Wall Pattern)
U+ - Unstable + Dual Wall Pattern (in one field, the other field has strange lines)
Ul - Unstable + Loops (in one field, the other field has alternating domains)
S - Stable Walls
S' - Nearly Unstable Walls
Sa - Stable Walls but with alternating domains on either side (could decay but very slow)
Ne - No strings but has domain walls?

Add s for can decay but slow
Add + for dual pattern
Add m for multidomain
Add ! for further inspection

Increasing kappa should lead to more stable walls. We know that N3111 produces stable walls when kappa = 0.04 (small). But if
kappa = 1, then Ng and Ng' would be on equal footing with N and N' and so N1131 will be stable as well (these are stable when kappa
is small as well). I think stability will still increase in that the domain wall length would be decrease less as small kappa
causes the walls to shrink more before becoming stable (in some cases the walls can fully decay, guessing there is enough energy
to overcome gap).

Ideally I can cut down the number of configurations in half. phi and psi are symmetric fields and so N3111 vs N1311 should be
the same and N2123 and N1232.

N1221 at kappa = 0.04 can decay if left for long enough.
N1221 at kappa = 1 won't decay
