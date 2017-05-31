#include "emulator.hpp"

using namespace std;

namespace compute = boost::compute;

SNPEmulator::SNPEmulator(ifstream *file_stream, bool isBinary){
    
    if(isBinary)
        snp.loadSNPFromFile(file_stream);
    else
        snp.loadSNPFromTextFile(file_stream);

    initCL();
    initVecs();
    initKernels();

};

SNPEmulator::SNPEmulator(string filename, bool isBinary){

    ifstream file_stream(filename);

    if(isBinary)
        snp.loadSNPFromFile(&file_stream);
    else
        snp.loadSNPFromTextFile(&file_stream);

    initCL();
    initVecs();
    initKernels();

}

int SNPEmulator::execute(stringstream *outputStream){

    int step = 1;

    size_t n = snp.ruleCount;
    size_t m = snp.neuronCount;

    //Simulation Proper
    do{

        chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        matchRulesRegex(regexs.data(), rules.data(), configVector.data(), spikingVector.data(), neuronFlags.data(), n);
        chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

        runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

        if(!areRulesApplicable(spikingVector,n))
                break;

        *outputStream << "************************************" << std::endl;
        *outputStream << "At step " << step << ":" << std::endl;

        snpSetStates(n, m, configVector, spikingVector, rules, delays, lossVector, stateVector, transitionVector);
        vectorSelectiveAdd(transitionVector, gainVector, n, m);
        snpComputeNetGain(n, m, stateVector, lossVector, gainVector, netGainVector);
        vectorAdd(netGainVector,configVector,configVector);
        snpPostCompute(n, m, rules, transitionVector);
        snpReset(n, m, lossVector, gainVector, netGainVector,neuronFlags);


        *outputStream << "CHOSEN RULES" << std::endl;
        for(size_t j = 0; j < n; j++){
            if(spikingVector[j] == 1){
                *outputStream << "Rule from neuron " << snp.ruleIds[j] << ": " << snp.getRule(j) << std::endl;
            }
        }

        *outputStream << "------------------------------------" << std::endl;

        for(size_t j = 0; j < m; j++){
            *outputStream << "NEURON " << j+1 << ":" << snp.neuronLabels[j] << std::endl; 
            *outputStream << "Spikes: " << configVector[j] << std::endl;
            *outputStream << "State: " << stateVector[j] << std::endl << std::endl;
        }

        step++;
    }while(areRulesApplicable(spikingVector,n));

    *outputStream << "************************************" << std::endl;
    *outputStream << "Configuration after " << step - 1 << " steps:\n";
    *outputStream << "------------------------------------" << std::endl;    

    for(size_t i = 0; i < m; i++){
        *outputStream << "NEURON " << i+1 << ": " << snp.neuronLabels[i] << std::endl; 
        *outputStream << "Spikes: " << configVector[i] << std::endl;
        *outputStream << "State: " << stateVector[i] << std::endl << std::endl;
    }

    return 0;
}

void SNPEmulator::initCL(){

    ifstream file(SNP_CL_SOURCE);
    string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    device = compute::system::default_device();
    context = compute::context(device);
    queue = compute::command_queue(context,device);
    program = compute::program::build_with_source(src,context);

}

void SNPEmulator::initKernels(){
    
    vectorSelectiveAddKernel = compute::kernel(program, "vectorSelectiveAdd");
    snpComputeNetGainKernel = compute::kernel(program, "snpComputeNetGain");
    snpPostComputeKernel = compute::kernel(program, "snpPostCompute");
    snpResetKernel = compute::kernel(program, "snpReset");
    snpSetStatesKernel = compute::kernel(program, "snpSetStates");

}

void SNPEmulator::initVecs(){

    int n = snp.ruleCount;
    int m = snp.neuronCount;

    configVector = vector<float>(m);
    spikingVector = vector<float>(n);
    stateVector = vector<float>(m);
    lossVector = vector<float>(m);
    gainVector = vector<float>(m);
    netGainVector = vector<float>(m);
    rules = vector<float>(n * 3);
    delays = vector<float>(n);
    transitionVector = vector<float>((m + 1) * n);
    neuronFlags = vector<float>(m);

    deviceConfigVector = compute::vector<float>(m,context);
    deviceSpikingVector = compute::vector<float>(n,context);
    deviceStateVector = compute::vector<float>(m,context);
    deviceLossVector = compute::vector<float>(m,context);
    deviceGainVector = compute::vector<float>(m,context);
    deviceNetGainVector = compute::vector<float>(m,context);
    deviceRules = compute::vector<float>(n * 3,context);
    deviceDelays = compute::vector<float>(n,context);
    deviceTransitionVector = compute::vector<float>((m + 1) * n,context);
    deviceNeuronFlags = compute::vector<float>(m,context);
    
    regexs = vector<string>(n);

    //Copy snp initial config to configVector
    utils::copyIntArrIntoFloatArr(snp.initConfig, configVector.data(), m); 

    //Copy snp rule delays into delays vector
    utils::copyIntArrIntoFloatArr(snp.ruleDelays, delays.data(), n);

    fill_n(stateVector.data(), m, 1);
    fill_n(neuronFlags.data(), m, -1);

    for(int i = 0; i < n; i++){
        int index = i * 3;
        rules[index] = snp.ruleIds[i];
        rules[index + 1] = -1;
        rules[index + 2] = -(snp.ruleConsumedSpikes[i]);
    }

    for(int i = 0; i < n; i++){
        for(int j = 1; j < m + 1; j++){
            if(snp.synapseMatrix[snp.ruleIds[i]][j] == 1)
                transitionVector[i * (m + 1) + j] = snp.ruleProducedSpikes[i];
        } 
    }

    for(int i = 0; i < n; i++){
        regexs[i] = utils::expandRegex(snp.getRuleRegex(i));
    }

}

bool SNPEmulator::areRulesApplicable(vector<float> spikingVector, size_t n){
    bool retVal = false;

    for(size_t i = 0; i < n; i++){

        if(spikingVector[i] == 1){
            retVal = true; 
            break;
        }

    }

    return retVal;
}

void SNPEmulator::vectorAdd(vector<float> &vectorA, vector<float> &vectorB, vector<float> &vectorC){

    compute::vector<float> deviceVectorA(vectorA.size(),context);
    compute::vector<float> deviceVectorB(vectorB.size(),context);
    compute::vector<float> deviceVectorC(vectorC.size(),context);

    compute::copy(
            vectorA.begin(),
            vectorA.end(),
            deviceVectorA.begin(),
            queue
            );

    compute::copy(
            vectorB.begin(),
            vectorB.end(),
            deviceVectorB.begin(),
            queue
            );

    compute::transform(
            deviceVectorA.begin(),
            deviceVectorA.end(),
            deviceVectorB.begin(),
            deviceVectorC.begin(),
            compute::plus<float>(),
            queue
            );

    compute::copy(
            deviceVectorC.begin(),
            deviceVectorC.end(),
            vectorC.begin(),
            queue
            );

}

void SNPEmulator::vectorSelectiveAdd(std::vector<float> &vectorA, std::vector<float> &outputVector, size_t rows, size_t cols){

    size_t globalSize = cols;
    size_t localSize = 1;

    compute::vector<float> deviceVectorA(vectorA.size(), context);
    compute::vector<float> deviceOutputVector(outputVector.size(), context);

    compute::copy(
            vectorA.begin(),
            vectorA.end(),
            deviceVectorA.begin(),
            queue
            );
   
    vectorSelectiveAddKernel.set_arg(0, deviceVectorA);
    vectorSelectiveAddKernel.set_arg(1, deviceOutputVector);
    vectorSelectiveAddKernel.set_arg(2, sizeof(int), (int *)&rows);
    vectorSelectiveAddKernel.set_arg(3, sizeof(int), (int *)&cols);

    queue.enqueue_1d_range_kernel(vectorSelectiveAddKernel,0,globalSize,localSize);
    queue.finish();

    compute::copy(
            deviceOutputVector.begin(),
            deviceOutputVector.end(),
            outputVector.begin(),
            queue
            );

}

void SNPEmulator::snpComputeNetGain(size_t n, size_t m, vector<float> &stateVector, vector<float> &lossVector,
        vector<float> &gainVector, vector<float> &netGainVector){

    size_t globalSize = m;

    compute::copy(
            stateVector.begin(),
            stateVector.end(),
            deviceStateVector.begin(),
            queue
            );

    compute::copy(
            lossVector.begin(),
            lossVector.end(),
            deviceLossVector.begin(),
            queue
            );

    compute::copy(
            gainVector.begin(),
            gainVector.end(),
            deviceGainVector.begin(),
            queue
            );

    snpComputeNetGainKernel.set_arg(0,sizeof(int), (int *)&n);
    snpComputeNetGainKernel.set_arg(1,sizeof(int), (int *)&m);
    snpComputeNetGainKernel.set_arg(2,deviceStateVector);
    snpComputeNetGainKernel.set_arg(3,deviceLossVector);
    snpComputeNetGainKernel.set_arg(4,deviceGainVector);
    snpComputeNetGainKernel.set_arg(5,deviceNetGainVector);

    queue.enqueue_1d_range_kernel(snpComputeNetGainKernel,0,globalSize,1);
    queue.finish();

    compute::copy(
            deviceNetGainVector.begin(),
            deviceNetGainVector.end(),
            netGainVector.begin(),
            queue
            );

}

void SNPEmulator::snpPostCompute(size_t n, size_t m,  vector<float> &rules, vector<float> &transitionVector){
    
    size_t globalSize = 1024;
    size_t localSize = 64;
    
    compute::copy(
            rules.begin(),
            rules.end(),
            deviceRules.begin(),
            queue
            );

    compute::copy(
            transitionVector.begin(),
            transitionVector.end(),
            deviceTransitionVector.begin(),
            queue
            );

    snpPostComputeKernel.set_arg(0,sizeof(int), (int *)&n);
    snpPostComputeKernel.set_arg(1,sizeof(int), (int *)&m);
    snpPostComputeKernel.set_arg(2,deviceRules);
    snpPostComputeKernel.set_arg(3,deviceTransitionVector);
    snpPostComputeKernel.set_arg(4,compute::local_buffer<float>(n));

    queue.enqueue_1d_range_kernel(snpPostComputeKernel,0,globalSize,localSize);
    queue.finish();

    compute::copy(
            deviceRules.begin(),
            deviceRules.end(),
            rules.begin(),
            queue
            );

    compute::copy(
            deviceTransitionVector.begin(),
            deviceTransitionVector.end(),
            transitionVector.begin(),
            queue
            );

}

void SNPEmulator::snpReset(size_t n, size_t m,  vector<float> &lossVector, vector<float> &gainVector, 
        vector<float> &netGainVector, vector<float> &neuronFlags){

    size_t globalSize = m;

    snpResetKernel.set_arg(0,sizeof(int), (int *)&n);
    snpResetKernel.set_arg(1,sizeof(int), (int *)&m);
    snpResetKernel.set_arg(2,deviceLossVector);
    snpResetKernel.set_arg(3,deviceGainVector);
    snpResetKernel.set_arg(4,deviceNetGainVector);
    snpResetKernel.set_arg(5,deviceNeuronFlags);

    queue.enqueue_1d_range_kernel(snpResetKernel,0,globalSize,1);
    queue.finish();

    compute::copy(
            deviceLossVector.begin(),
            deviceLossVector.end(),
            lossVector.begin(),
            queue
            );

    compute::copy(
            deviceGainVector.begin(),
            deviceGainVector.end(),
            gainVector.begin(),
            queue
            );

    compute::copy(
            deviceNetGainVector.begin(),
            deviceNetGainVector.end(),
            netGainVector.begin(),
            queue
            );

    compute::copy(
            deviceNeuronFlags.begin(),
            deviceNeuronFlags.end(),
            neuronFlags.begin(),
            queue
            );

}

void SNPEmulator::snpSetStates(size_t n, size_t m,  vector<float> &configVector, vector<float> &spikingVector,
        vector<float> &rules,  vector<float> &delays,  vector<float> &lossVector,
        vector<float> &stateVector,  vector<float> &transitionVector){

    size_t globalSize = n;

    compute::copy(
            configVector.begin(),
            configVector.end(),
            deviceConfigVector.begin(),
            queue
            );

    compute::copy(
            spikingVector.begin(),
            spikingVector.end(),
            deviceSpikingVector.begin(),
            queue
            );

    compute::copy(
            rules.begin(),
            rules.end(),
            deviceRules.begin(),
            queue
            );

    compute::copy(
            delays.begin(),
            delays.end(),
            deviceDelays.begin(),
            queue
            );

    compute::copy(
            lossVector.begin(),
            lossVector.end(),
            deviceLossVector.begin(),
            queue
            );

    compute::copy(
            stateVector.begin(),
            stateVector.end(),
            deviceStateVector.begin(),
            queue
            );

    compute::copy(
            transitionVector.begin(),
            transitionVector.end(),
            deviceTransitionVector.begin(),
            queue
            );

    snpSetStatesKernel.set_arg(0,sizeof(int), (int *)&n);
    snpSetStatesKernel.set_arg(1,sizeof(int), (int *)&m);
    snpSetStatesKernel.set_arg(2,deviceConfigVector);
    snpSetStatesKernel.set_arg(3,deviceSpikingVector);
    snpSetStatesKernel.set_arg(4,deviceRules);
    snpSetStatesKernel.set_arg(5,deviceDelays);
    snpSetStatesKernel.set_arg(6,deviceLossVector);
    snpSetStatesKernel.set_arg(7,deviceStateVector);
    snpSetStatesKernel.set_arg(8,deviceTransitionVector);

    queue.enqueue_1d_range_kernel(snpSetStatesKernel, 0, globalSize, 1);

    compute::copy(
            deviceConfigVector.begin(),
            deviceConfigVector.end(),
            configVector.begin(),
            queue
            );

    compute::copy(
            deviceSpikingVector.begin(),
            deviceSpikingVector.end(),
            spikingVector.begin(),
            queue
            );

    compute::copy(
            deviceRules.begin(),
            deviceRules.end(),
            rules.begin(),
            queue
            );

    compute::copy(
            deviceLossVector.begin(),
            deviceLossVector.end(),
            lossVector.begin(),
            queue
            );

    compute::copy(
            deviceStateVector.begin(),
            deviceStateVector.end(),
            stateVector.begin(),
            queue
            );

    compute::copy(
            deviceTransitionVector.begin(),
            deviceTransitionVector.end(),
            transitionVector.begin(),
            queue
            );

}

float SNPEmulator::getRuntime(){
    return float(runtime.count());
}

void SNPEmulator::matchRuleRegex(int threadId, std::string regex, std::string str, 
        float* spikingVector, float* neuronFlag, float* rules){
    re2::StringPiece input(str);
    spikingVector[threadId] = re2::RE2::FullMatch(input,regex);
}

void SNPEmulator::matchRulesRegex(string *regexVector, float* rules, float* configVector, float* spikingVector, 
        float* neuronFlags, size_t n){

    thread threads[n];

    for(size_t i = 0; i < n; i++){
        string expandedRegex = regexVector[i];
        string spikeString = utils::expandRegex("a^"+ boost::lexical_cast<std::string>(configVector[(int)rules[3 * i] - 1]));
        threads[i] = thread(matchRuleRegex, i, expandedRegex, spikeString, spikingVector, neuronFlags, rules);
    }
    
    for(size_t i = 0; i < n; i++){
        threads[i].join();
    }
}
