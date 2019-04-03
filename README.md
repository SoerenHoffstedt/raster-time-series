# Iterator-Based Processing of Raster Time Series

This is the prototype implementation of a concept for processing raster time series called *Iterator-Based Processing of Raster Time Series (IPRTS)*. It was developed for my Bachelor thesis. The finished thesis can be read [here](https://github.com/SoerenHoffstedt/raster-time-series/blob/dc3b0d6dde009d0fa9985a539ab29a4972a50471/Bacheloarbeit_S%C3%B6renHoffstedt_Iterator-Basd-Processing-of-Raster-Time-Series.pdf).

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
