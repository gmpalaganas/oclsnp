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

4. Install and configure git

	```bash
	$ sudo pacman -S git
	$ git config --global user.name "your name"
	$ git config --global user.email your@email.com
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

### Windows

1. Install the latest driver for your GPU

2. Install and configure git

	1. Open git bash
	2. Set global variables
	
		```bash
		> git config --global user.name "your name"
		> git config --global user.email your@email.com
		```

3. Install OpenCL SDK

  * For NVIDIA install CUDA SDK
  * For AMD install AMD App SDK

4. Install MinGW

  * Set destination folder as "C:\MinGW" (default)
  * On installer select
     * mingw-developer-toolkit
     * mingw32-base
	 * mingw32-gcc-g++
	 * msys-base
	 * msys-make (optional)

5. Download Boost 1.55 for windows then extract to C:\

  * Rename folder to boost
  * cd into boost folder

      ```
      > cd C:\boost
      ```
  * Build boost
   
      ```
      > bootsrap mingw
	  > b2 toolset=gcc
	  ````

Building
------

### On Linux Distros

1. CD into project root

2. Build the project using make

	```bash
	$ make
	```

### On Windows 

1. CD into project root

2. CD into scripts
	
	```
	> cd scripts
	```

3. Run compile.bat
	
	```
	> compile
	```

Running
------

### On Linux Distros

1. cd into bin from project root

	```bash
	$ cd bin
	```

2. For Parallel

	```bash
	$ ./oclsnp <input_binary>
	```

3. For Linear
	
	```bash
	$ ./linsnp <input_binary>
	```

### On Windows

1. cd into bin from project root

	```
	> cd bin
	```

2. For Parallel

	```
	> oclsnp <input_binary>
	```

3. For Linear
	
	```
	> linsnp <input_binary>
	```