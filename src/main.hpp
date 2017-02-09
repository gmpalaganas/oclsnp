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

#include <CL/cl.h>
#include <boost/lexical_cast.hpp>
#include <snp.hpp>
#include <array.hpp>
#include <regex.hpp>
#include <opencl_error.hpp>

#define VECTOR_ADD_SRC             "kernels/vector_add.cl"
#define VECTOR_ELEM_MULT_SRC       "kernels/vector_elem_mult.cl"
#define VECTOR_SELECTIVE_ADD_SRC   "kernels/vector_selective_add.cl"

#define SNP_COMPUTE_NET_GAIN_SRC "kernels/snp_compute_net_gain.cl"
#define SNP_DETERMINE_RULES_SRC  "kernels/snp_determine_rules.cl"
#define SNP_POST_COMPUTE_SRC     "kernels/snp_post_compute.cl"
#define SNP_RESET_SRC            "kernels/snp_reset.cl"
#define SNP_SET_STATES_SRC       "kernels/snp_set_states.cl"

namespace programFlags{
    enum ProgramFlags { SILENT };
}

void cleanup();

programFlags::ProgramFlags checkFlag(std::string flag);

void initCL(int n, int m);

char* loadProgramSource(std::string fileName);

bool areRulesApplicable(float *spikingVector, int n);

void initKernels();
void initVectorAddKernel();
void initVectorElemMultKernel();
void initVectorSelectiveAddKernel();
void initSNPComputeNetGainKernel();
void initSNPPostComputeKernel();
void initSNPResetKernel();
void initSNPSetStatesKernel();

namespace gpu{
    void vectorAdd(float *vectorA, float *vectorB, float *outputVector, int vectorSize);
    void vectorElemMult(float *vectorA, float *vectorB, float *outputVector, int vectorSize);
    void vectorSelectiveAdd(float *vectorA, float *outputVector, int rows, int cols);
    void snpComputeNetGain(int n, int m, float *stateVector, float *lossVector, float *gainVector, float *netGainVector);
    void snpPostCompute(int n, int m,  float *rules, float *transitionVector);
    void snpReset(int n, int m,  float *lossVector, float *gainVector, float *netGainVector, float *neuronFlags);
    void snpSetStates(int n, int m,  float *configVector, float *spikingVector, float* rules,  float* delays,  float* lossVector,
            float* stateVector,  float* transitionVector);
};

void matchRuleRegex(int threadId, std::string regex, std::string str, float* spikingVector, float* neuronFlag, float* rules);
void matchRulesRegex(std::string *regexVector, float* rules, float* configVector, float* spikingVector, float* neuronFlags,  int n);

void getMemUsage(double& vmUsage, double& residentSet);

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
cl_mem clConfigBuffer;
cl_mem clSpikingBuffer;
cl_mem clStateBuffer;
cl_mem clLossBuffer;
cl_mem clGainBuffer;
cl_mem clNetGainBuffer;
cl_mem clRulesBuffer;
cl_mem clDelaysBuffer;
cl_mem clTransitionBuffer;
cl_mem clNeuronFlagsBuffer;
cl_mem clReprBuffer;

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

float *neuronFlags;

std::string *regexs;

std::ofstream outputFile;
