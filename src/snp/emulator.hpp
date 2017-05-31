#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <sstream>
#include <thread>

#include <unistd.h>
#include <CL/cl.h>
#include <re2/re2.h>
#include <snp.hpp>
#include <opencl_error.hpp>
#include <array.hpp>

#include <boost/compute/algorithm/copy.hpp>
#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/functional/operator.hpp>
#include <boost/compute/core.hpp>
#include <boost/compute/memory/local_buffer.hpp>

#define SNP_CL_SOURCE "kernels/snp.cl"

class SNPEmulator{

    public:
        SNPEmulator(std::ifstream *file_stream,bool isBinary);
        SNPEmulator(std::string filename,bool isBinary);

        int execute(std::stringstream *outputstream);

        float getRuntime();

        SNP snp;

        std::vector<float> configVector;
        std::vector<float> spikingVector;
        std::vector<float> stateVector;
        std::vector<float> lossVector;
        std::vector<float> gainVector;
        std::vector<float> netGainVector;
        std::vector<float> rules;
        std::vector<float> delays;
        std::vector<float> transitionVector;
        std::vector<float> neuronFlags;

        std::vector<std::string> regexs;

    private:
        std::chrono::microseconds runtime;

        boost::compute::device device;
        boost::compute::context context;
        boost::compute::command_queue queue;

        boost::compute::program program;

        boost::compute::kernel vectorAddKernel;
        boost::compute::kernel vectorSelectiveAddKernel;
        boost::compute::kernel snpComputeNetGainKernel;
        boost::compute::kernel snpPostComputeKernel;
        boost::compute::kernel snpResetKernel;
        boost::compute::kernel snpSetStatesKernel;

        boost::compute::vector<float> deviceConfigVector;
        boost::compute::vector<float> deviceSpikingVector;
        boost::compute::vector<float> deviceStateVector;
        boost::compute::vector<float> deviceLossVector;
        boost::compute::vector<float> deviceGainVector;
        boost::compute::vector<float> deviceNetGainVector;
        boost::compute::vector<float> deviceRules;
        boost::compute::vector<float> deviceDelays;
        boost::compute::vector<float> deviceTransitionVector;
        boost::compute::vector<float> deviceNeuronFlags;

        void initCL();
        void initKernels();
        void initVecs();

        bool areRulesApplicable(std::vector<float> spikingVector, size_t n);

        void vectorAdd(std::vector<float> &vectorA, std::vector<float> &vectorB, std::vector<float> &vectorC);
        void vectorSelectiveAdd(std::vector<float> &vectorA, std::vector<float> &outputVector, size_t rows, size_t cols);
        void snpComputeNetGain(size_t n, size_t m, std::vector<float> &stateVector, std::vector<float> &lossVector,
                std::vector<float> &gainVector, std::vector<float> &netGainVector);
        void snpPostCompute(size_t n, size_t m,  std::vector<float> &rules, std::vector<float> &transitionVector);
        void snpReset(size_t n, size_t m,  std::vector<float> &lossVector, std::vector<float> &gainVector, 
                std::vector<float> &netGainVector, std::vector<float> &neuronFlags);
        void snpSetStates(size_t n, size_t m,  std::vector<float> &configVector, std::vector<float> &spikingVector,
                std::vector<float> &rules,  std::vector<float> &delays,  std::vector<float> &lossVector,
                std::vector<float> &stateVector,  std::vector<float> &transitionVector);
        static void matchRuleRegex(int threadId, std::string regex, std::string str, float* spikingVector,
                float* neuronFlag, float* rules);
        void matchRulesRegex(std::string *regexVector, float* rules, float* configVector, float* spikingVector, float* neuronFlags, 
                size_t n);

};
