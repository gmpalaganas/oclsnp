# OpenCL SNP Simulation

An OpenCL implementation of a simulator for Spiking Neural Systems.

The project uses OpenCL 1.1 libraries on top of C++11.


Dependencies
------

This project is hardware dependent so machines with different GPUs have different dependencies.

### GENERAL DEPENDENCIES
- Git
- OpenCL ICD Loader
- OpenCL headers
- C++ Boost library
- Make
- GNU g++ compiler

### MACHINES WITH NVIDIA CARD
- nVidia drivers for the card in your machine
- CUDA SDK
- opencl-nvidia (Needed for execution) - cuda (Note: Nvidia implementation only currently supports OpenCL 1.1)

### MACHINES WITH ATI CARD
- ATI drivers for the card in your machine
- AMD App SDK

- opencl-catalyst or opencl-mesa
- amdapp-sdk

Installing Dependencies
------

### Arch-Linux and Arch-based Linux Distributions

1. Update repositories

	```bash
	$ sudo pacman -Syu
	```

2. Install packer

	```
	$ sudo pacman -S wget expac jshon
	$ mkdir packer
	$ cd packer
	$ sudo wget https://aur.archlinux.org/cgit/aur.git/plain/PKGBUILD?h=packer
	$ mv PKGBUILD?h=packer PKGBUILD
	$ makepkg
	$ sudo pacman -U packer-*********-*-any.pkg.tar.xz
	$ cd ..
	$ sudo rm -dR packer 
	```

3. Install gcc

	```bash
	$ sudo pacman -S gcc
	```

4. Install git

	```bash
	$ sudo pacman -S git
	```

5. Install OpenCL headers and ICD Loaders

	```	bash
	$ sudo pacman -S ocl-icd opencl-headers
	```

6. Install GPU dependent dependencies
	AMD

		```bash
		$ sudo packer -S opencl-catalyst amdapp-sdk
		```

	NVIDIA

		```bash
		$ sudo pacman -S opencl-nvidia cuda
		```

7. Install boost

	```bash
	$ sudo pacman -S boost
	```


Building
------
1. CD into project root

2. Build the project using make

	```bash
	$ make
	```


Running
------
1. cd into bin from project root

	```bash
	$ cd bin
	```

2.Type in the following command

	```bash
	//For parallel implementation 

	$ ./oclsnp <input_binary> 
	
	//For linear implementation

	$ ./linsnp <input_binary>
	```
