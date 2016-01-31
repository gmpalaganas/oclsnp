# OpenCL SNP Simulation

An OpenCL implementation of a simulator for Spiking Neural Systems.

The project uses OpenCL 1.1 libraries on top of C++11.


Dependencies
------

This project is hardware dependent so machines with different GPUs have different dependencies.

### GENERAL DEPENDENCIES
- ocl-icd
- opencl-headers
- boost (C++ libraries, used for handling Regular Expressions)

### MACHINES WITH NVIDIA CARD
- nVidia drivers for the card in your machine
- opencl-nvidia (Needed for execution) - cuda (Note: Nvidia implementation only currently supports OpenCL 1.1)

### MACHINES WITH ATI CARD
- ATI drivers for the card in your machine
- opencl-catalyst or opencl-mesa
- amdapp-sdk


Compiling
------
For the parallel implementation

```
$ g++ src/main.cpp -o <output_file> -lOpenCL -lboost_regex -std=c++11
```

For the linear implementation

```
$ g++ src/main_linear.cpp -o <output_file> -lboost_regex -std=c++11
```

Running
------
For parallel implementation 

```
$ ./<output_file_from_compilation> <input_binary>  <path_to_kernels_dir>
```

For linear implementation
```
$ ./<output_file_from_compilation> <input_binary>
```
