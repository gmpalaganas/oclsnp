@echo off

set ocl_dll="C:\Program Files\NVIDIA Corporation\OpenCL\OpenCL.dll"
set ocl_headers="C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v7.5\include"
set boost_headers="C:\boost"
set boost_lib="C:\boost\stage\lib"
set boost_regex_lib=-lboost_regex-mgw48-mt-1_55
set source_files=../src/utils/array.cpp ../src/snp/utils.cpp ../src/snp/regex_tree.cpp ../src/snp/snp.cpp

echo Compiling Parallel Implementation
g++ ../src/main.cpp %source_files% %ocl_dll% -o ../bin/oclsnp.exe -I%ocl_headers% -I%boost_headers% -L%boost_lib% %boost_regex_lib% -std=c++11

echo Compiling Linear Implementation
g++ ../src/main_linear.cpp %source_files% -o ../bin/linsnp.exe -I%boost_headers% -L%boost_lib% %boost_regex_lib% -std=c++11

echo Making kernels dir
mkdir "../bin/kernels"

echo Copying kernels
xcopy "../src/kernels" "../bin/kernels"