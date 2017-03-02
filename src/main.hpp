#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <thread>
#include <chrono>
#include <vector>
#include <ctime>

#include <emulator.hpp>

namespace programFlags{
    enum ProgramFlags { SILENT };
}

programFlags::ProgramFlags checkFlag(std::string flag);

void getMemUsage(double& vmUsage, double& residentSet);
