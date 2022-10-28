# CosmoTD #

This repository contains the source code for a C++ application that runs scalar field simulations that aim to study cosmological
topological defects. This is an improved version of the cosmotd-python project, greatly improving the performance speed
of the field simulations (around 100 times faster). The source code is not particularly well organised and is currently very messy.
The application however should be relatively intuitive.

## Running the Application ##

The application can either be built from source using Cmake or be downloaded through a release (currently only for Windows).

## Caution with Field Sizes ##

Note that due to how GPU compute shaders and work groups function, the size of the fields must be divisible by 4, otherwise
parts of the field will never be simulated. This should be apparent in the application when the field size is wrong.
