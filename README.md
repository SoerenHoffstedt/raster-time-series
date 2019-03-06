# Iterator-Based Processing of Raster Time Series

This is the prototype implementation of a concept for processing raster time series called *Iterator-Based Processing of Raster Time Series (IPRTS)*.

## Dependencies

IPRTS was developed on Ubuntu 18.04.1 using CLion and CMake.
Two libraries have to be installed:

```
sudo apt install libboost-all-dev
sudo apt install libgdal-dev
```

IPRTS requires a `C++14` compiler.

## Usage

IPRTS creates the `rts_run_query` executable to execute a single query. It executes a query file that is passed as a program argument.