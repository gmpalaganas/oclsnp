#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <bitset>
#include <vector>
#include <iterator>
#include <algorithm>
#include <regex>
#include <boost/regex.hpp>

#include <CL/cl.h>

#include "snp/snp.cpp"
#include "utils/array.cpp"

#define MAX_SNP_STRING_LEN 255

#define VECTOR_ADD_SRC "./kernels/vector_add.cl"
#define VECTOR_ELEM_MULT_SRC "./kernels/vector_elem_mult.cl"
#define VECTOR_SELECTIVE_ADD_SRC "./kernels/vector_selective_add.cl"

#define SNP_COMPUTE_NET_GAIN_SRC "./kernels/snp_compute_net_gain.cl"
#define SNP_DETERMINE_RULES_SRC  "./kernels/snp_determine_rules.cl"
#define SNP_POST_COMPUTE_SRC     "./kernels/snp_post_compute.cl"
#define SNP_RESET_SRC            "./kernels/snp_reset.cl"
#define SNP_SET_STATES_SRC       "./kernels/snp_set_states.cl"

inline void checkError(cl_int err, std::string msg, std::string fncName);
inline void checkError(cl_int err, std::string msg, std::string fncName, cl_program program);
inline std::string getCLError(cl_int errorCode);
void cleanup();

void initCL();

char* loadProgramSource(std::string fileName);

void initKernels();
void initVectorAddKernel();
void initVectorElemMultKernel();
void initVectorSelectiveAddKernel();
void initSNPComputeNetGainKernel();
void initSNPDetermineRulesKernel();
void initSNPPostComputeKernel();
void initSNPResetKernel();
void initSNPSetStatesKernel();

void printSNPContents();

namespace gpu{
    void vectorAdd(float *vectorA, float *vectorB, float *outputVector, int vectorSize);
    void vectorElemMult(float *vectorA, float *vectorB, float *outputVector, int vectorSize);
    void vectorSelectiveAdd(float *vectorA, float *outputVector, int rows, int cols);
    void snpComputeNetGain(int n, int m, float *stateVector, float *lossVector, float *gainVector, float *netGainVector);
    void snpDetermineRules(int n, int m,  float *configVector, float *spikingVector, float *rules, float *lhs);
    void snpPostCompute(int n, int m,  float *rules, float *transitionVector);
    void snpReset(int n, int m,  float *lossVector, float *gainVector, float *netGainVector);
    void snpSetStates(int n, int m,  float *configVector, float *spikingVector, float* rules,  float* delays,  float* lossVector,
            float* stateVector,  float* transitionVector);
};

//OpenCL variables
cl_context clContext; 
cl_command_queue clCommandQueue; 
cl_platform_id clPlatform;
cl_device_id clDevice;

//Kernels and Programs
cl_kernel  vectorAddKernel;
cl_program vectorAddProgram;

cl_kernel  vectorElemMultKernel;
cl_program vectorElemMultProgram;

cl_kernel  vectorSelectiveAddKernel;
cl_program vectorSelectiveAddProgram;

cl_kernel  snpComputeNetGainKernel;
cl_program snpComputeNetGainProgram;

cl_kernel  snpDetermineRulesKernel;
cl_program snpDetermineRulesProgram;

cl_kernel  snpPostComputeKernel;
cl_program snpPostComputeProgram;

cl_kernel  snpResetKernel;
cl_program snpResetProgram;

cl_kernel  snpSetStatesKernel;
cl_program snpSetStatesProgram;

//Matrix Buffers
cl_mem clBufferA;
cl_mem clBufferB;
cl_mem clBufferC;
cl_mem clBufferD;
cl_mem clBufferE;
cl_mem clBufferF;
cl_mem clBufferG;

//Error code holder
cl_int clErr;

SNP snp;

float *configVector;
float *spikingVector;
float *stateVector;
float *lossVector;
float *gainVector;
float *netGainVector;
float *rules;
float *delays;
float *transitionVector;
float *lhs;


