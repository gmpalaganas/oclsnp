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

#define VECTOR_ADD_SRC             "kernels/vector_add.cl"
#define VECTOR_ELEM_MULT_SRC       "kernels/vector_elem_mult.cl"
#define VECTOR_SELECTIVE_ADD_SRC   "kernels/vector_selective_add.cl"

#define SNP_COMPUTE_NET_GAIN_SRC "kernels/snp_compute_net_gain.cl"
#define SNP_DETERMINE_RULES_SRC  "kernels/snp_determine_rules.cl"
#define SNP_POST_COMPUTE_SRC     "kernels/snp_post_compute.cl"
#define SNP_RESET_SRC            "kernels/snp_reset.cl"
#define SNP_SET_STATES_SRC       "kernels/snp_set_states.cl"

namespace programFlags{
    enum ProgramFlags { SILENT, TEXT, PRINT_SNP };
}

programFlags::ProgramFlags checkFlag(std::string flag);

void getMemUsage(double& vmUsage, double& residentSet);
