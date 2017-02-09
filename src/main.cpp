#include "main.hpp"

// To be configured to accept input file name
// and number of steps

std::chrono::microseconds runtime;

int main(int argc, char **argv){
    
    std::vector<programFlags::ProgramFlags> flags;
    std::stringstream outputStream;

    if(argc == 3){
        outputFile = std::ofstream(argv[2]);
    }else if(argc < 2){
        std::cout << "Usage: ./oclsnp <input_binary_file> [output_file]" << std::endl;
        std::cout << "I.e ./oclsnp ../inputs/2input_sort.bin outputs/2input_sort_out.txt" << std::endl;
        exit(1);
    }else{
        for(int i = 3; i < argc; i++)
            flags.push_back(checkFlag(argv[i]));
    }
    
    clErr = snp.loadSNPFromFile(argv[1]);
    utils::checkError(clErr, "Invalid Binary file", __FUNCTION__);


    int n = snp.ruleCount;
    int m = snp.neuronCount;
    
    initCL(n,m);
    initKernels();

    configVector = new float[m]();
    spikingVector = new float[n]();
    stateVector = new float[m]();
    lossVector = new float[m]();
    gainVector = new float[m]();
    netGainVector = new float[m]();
    rules = new float[n * 3]();
    delays = new float[n]();
    transitionVector = new float[(m + 1) * n]();
    neuronFlags = new float[m]();

    regexs = new std::string[n]();

    //Copy snp initial config to configVector
    utils::copyIntArrIntoFloatArr(snp.initConfig, configVector, m); 

    //Copy snp rule delays into delays vector
    utils::copyIntArrIntoFloatArr(snp.ruleDelays, delays, n);

    std::fill_n(stateVector, m, 1);
    std::fill_n(neuronFlags, m, -1);

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

    int step = 1;

    //Simulation Proper
    do{

        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        matchRulesRegex(regexs, rules, configVector, spikingVector, neuronFlags, n);
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

        runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

        if(!areRulesApplicable(spikingVector,n))
                break;

        outputStream << "************************************" << std::endl;
        outputStream << "At step " << step << ":" << std::endl;

        gpu::snpSetStates(n, m, configVector, spikingVector, rules, delays, lossVector, stateVector, transitionVector);
        gpu::vectorSelectiveAdd(transitionVector, gainVector, n, m);
        gpu::snpComputeNetGain(n, m, stateVector, lossVector, gainVector, netGainVector);
        gpu::vectorAdd(netGainVector,configVector,configVector,m);
        gpu::snpPostCompute(n, m, rules, transitionVector);
        gpu::snpReset(n, m, lossVector, gainVector, netGainVector,neuronFlags);


        outputStream << "CHOSEN RULES" << std::endl;
        for(int j = 0; j < n; j++){
            if(spikingVector[j] == 1){
                outputStream << "Rule from neuron " << snp.ruleIds[j] << ": " << snp.getRule(j) << std::endl;
            }
        }

        outputStream << "------------------------------------" << std::endl;

        for(int j = 0; j < m; j++){
            outputStream << "NEURON " << j+1 << ":" << snp.neuronLabels[j] << std::endl; 
            outputStream << "Spikes: " << configVector[j] << std::endl;
            outputStream << "State: " << stateVector[j] << std::endl << std::endl;
        }

        step++;
    }while(areRulesApplicable(spikingVector,n));

    outputStream << "************************************" << std::endl;
    outputStream << "Configuration after " << step - 1 << " steps:\n";
    outputStream << "------------------------------------" << std::endl;    

    for(int i = 0; i < m; i++){
        outputStream << "NEURON " << i+1 << ": " << snp.neuronLabels[i] << std::endl; 
        outputStream << "Spikes: " << configVector[i] << std::endl;
        outputStream << "State: " << stateVector[i] << std::endl << std::endl;
    }
    
    double vm;
    double rss;
    getMemUsage(vm, rss);

    outputStream << "Execution time: " << float(runtime.count()) << " ns" << std::endl;
    outputStream << "Memory Usage: " << rss << " kb" << std::endl;
    
    if(std::find(flags.begin(), flags.end(), programFlags::ProgramFlags::SILENT) == flags.end()){
        std::cout << outputStream.str();
    }else{
        std::cout << "Execution time: " << float(runtime.count()) << " ns" << std::endl;
        std::cout << "Memory Usage: " << rss << " kb" << std::endl;
    }

    if(outputFile){
        outputFile << outputStream.str();
    }

    cleanup();

    return 0;

}

//Release OpenCL variables and delete dynamically allocated variables
void cleanup(){
    clReleaseCommandQueue(clCommandQueue);
    clReleaseContext(clContext);

    clReleaseKernel(vectorAddKernel);
    clReleaseProgram(vectorAddProgram);
    clReleaseKernel(vectorElemMultKernel);
    clReleaseProgram(vectorElemMultProgram);
    clReleaseKernel(vectorSelectiveAddKernel);
    clReleaseProgram(vectorSelectiveAddProgram);

    clReleaseKernel(snpComputeNetGainKernel);
    clReleaseProgram(snpComputeNetGainProgram);
    clReleaseKernel(snpPostComputeKernel);
    clReleaseProgram(snpPostComputeProgram);
    clReleaseKernel(snpResetKernel);
    clReleaseProgram(snpResetProgram);
    clReleaseKernel(snpSetStatesKernel);
    clReleaseProgram(snpSetStatesProgram);

    clReleaseMemObject(clConfigBuffer);
    clReleaseMemObject(clSpikingBuffer);
    clReleaseMemObject(clStateBuffer);
    clReleaseMemObject(clLossBuffer);
    clReleaseMemObject(clGainBuffer);
    clReleaseMemObject(clNetGainBuffer);
    clReleaseMemObject(clRulesBuffer);
    clReleaseMemObject(clDelaysBuffer);
    clReleaseMemObject(clTransitionBuffer);
    clReleaseMemObject(clReprBuffer);

    delete[] configVector;
    delete[] spikingVector;
    delete[] stateVector;
    delete[] lossVector;
    delete[] gainVector;
    delete[] netGainVector;
    delete[] rules;
    delete[] delays;
    delete[] transitionVector;
    delete[] regexs;

    if(outputFile)
        outputFile.close();

}

programFlags::ProgramFlags checkFlag(std::string flag){
    programFlags::ProgramFlags progFlag;
    if(flag == "--silent")
        progFlag = programFlags::ProgramFlags::SILENT;
    return progFlag;
}

//Initialize OpenCL variables i.e. Platform IDs and Device IDs
void initCL(int n, int m){

    clErr = clGetPlatformIDs(1, &clPlatform, NULL);  
    utils::checkCLError(clErr, "Unable to retrieve platforms", __FUNCTION__);

    clErr = clGetDeviceIDs(clPlatform, CL_DEVICE_TYPE_GPU, 1, &clDevice, NULL);
    utils::checkCLError(clErr, "Unable to retrieve devices", __FUNCTION__);

    clContext = clCreateContext(0, 1, &clDevice, NULL, NULL, &clErr);
    utils::checkCLError(clErr, "Unable to create context", __FUNCTION__);

    clCommandQueue = clCreateCommandQueue(clContext, clDevice, 0, &clErr);
    utils::checkCLError(clErr, "Unable to create command queue", __FUNCTION__);

    clConfigBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clSpikingBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, n * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clStateBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBuffer", __FUNCTION__);
   
    clLossBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clGainBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clNetGainBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clRulesBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, 3 * n * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clDelaysBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, n * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clTransitionBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, n * (m + 1) * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clNeuronFlagsBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m  * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBuffer", __FUNCTION__);

}

//Load OpenCL Program for source (.cl file)
//Note: Currently not working
char* loadProgramSource(std::string fileName){

    std::ifstream file(fileName);
    std::cout << fileName << std::endl;
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());

    std::cout << (char *)source.c_str() << std::endl;
    return (char *)source.c_str();

}

//Check if there still are applicable rules
bool areRulesApplicable(float* spikingVector, int n){
    bool retVal = false;

    for(int i = 0; i < n; i++){

        if(spikingVector[i] == 1){
            retVal = true; 
            break;
        }

    }

    return retVal;
}

//Init OpenCL Kernels
void initKernels(){

    initVectorAddKernel();
    initVectorElemMultKernel();
    initVectorSelectiveAddKernel();

    initSNPComputeNetGainKernel();
    initSNPPostComputeKernel();
    initSNPResetKernel();
    initSNPSetStatesKernel();

}

/*===OpenCL Kernel Initializations===*/

void initVectorAddKernel(){

    //char *src = loadProgramSource(VECTOR_ADD_SRC);

    std::ifstream file(VECTOR_ADD_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    vectorAddProgram = clCreateProgramWithSource(clContext,1, (const char **)&src, NULL, &clErr);
    utils::checkCLError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(vectorAddProgram,1,&clDevice,NULL,NULL,NULL);
    utils::checkCLError(clErr, "Unable to build program", __FUNCTION__, vectorAddProgram,clDevice);

    vectorAddKernel = clCreateKernel(vectorAddProgram, "vectorAdd", &clErr);
    utils::checkCLError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initVectorElemMultKernel(){

    //char *src = loadProgramSource(VECTOR_ELEM_MULT_SRC);
    std::ifstream file(VECTOR_ELEM_MULT_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    vectorElemMultProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    utils::checkCLError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(vectorElemMultProgram,1,&clDevice,NULL,NULL,NULL);
    utils::checkCLError(clErr, "Unable to build program", __FUNCTION__, vectorElemMultProgram,clDevice);

    vectorElemMultKernel = clCreateKernel(vectorElemMultProgram, "vectorElemMult", &clErr);
    utils::checkCLError(clErr, "Unable to create kernel", __FUNCTION__);
   
}


void initVectorSelectiveAddKernel(){
    //char *src = loadProgramSource(VECTOR_SELECTIVE_ADD_SRC);

    std::ifstream file(VECTOR_SELECTIVE_ADD_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    vectorSelectiveAddProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    utils::checkCLError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(vectorSelectiveAddProgram,1,&clDevice,NULL,NULL,NULL);
    utils::checkCLError(clErr, "Unable to build program", __FUNCTION__, vectorSelectiveAddProgram,clDevice);

    vectorSelectiveAddKernel = clCreateKernel(vectorSelectiveAddProgram, "vectorSelectiveAdd", &clErr);
    utils::checkCLError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initSNPComputeNetGainKernel(){
    //char *src = loadProgramSource(SNP_COMPUTE_NET_GAIN_SRC);

    std::ifstream file(SNP_COMPUTE_NET_GAIN_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    snpComputeNetGainProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    utils::checkCLError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(snpComputeNetGainProgram,1,&clDevice,NULL,NULL,NULL);
    utils::checkCLError(clErr, "Unable to build program", __FUNCTION__, snpComputeNetGainProgram,clDevice);

    snpComputeNetGainKernel = clCreateKernel(snpComputeNetGainProgram, "snpComputeNetGain", &clErr);
    utils::checkCLError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initSNPPostComputeKernel(){
    //char *src = loadProgramSource(SNP_POST_COMPUTE_SRC);

    std::ifstream file(SNP_POST_COMPUTE_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    snpPostComputeProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    utils::checkCLError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(snpPostComputeProgram,1,&clDevice,NULL,NULL,NULL);
    utils::checkCLError(clErr, "Unable to build program", __FUNCTION__, snpPostComputeProgram,clDevice);

    snpPostComputeKernel = clCreateKernel(snpPostComputeProgram, "snpPostCompute", &clErr);
    utils::checkCLError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initSNPResetKernel(){
    //char *src = loadProgramSource(SNP_RESET_SRC);

    std::ifstream file(SNP_RESET_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    snpResetProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    utils::checkCLError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(snpResetProgram,1,&clDevice,NULL,NULL,NULL);
    utils::checkCLError(clErr, "Unable to build program", __FUNCTION__, snpResetProgram,clDevice);

    snpResetKernel = clCreateKernel(snpResetProgram, "snpReset", &clErr);
    utils::checkCLError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initSNPSetStatesKernel(){
    //char *src = loadProgramSource(SNP_SET_STATES_SRC);

    std::ifstream file(SNP_SET_STATES_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    snpSetStatesProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    utils::checkCLError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(snpSetStatesProgram,1,&clDevice,NULL,NULL,NULL);
    utils::checkCLError(clErr, "Unable to build program", __FUNCTION__, snpSetStatesProgram,clDevice);

    snpSetStatesKernel = clCreateKernel(snpSetStatesProgram, "snpSetStates", &clErr);
    utils::checkCLError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

/*===End OpenCL Kernel Initializations===*/

void gpu::vectorAdd(float *vectorA, float *vectorB, float *outputVector, int vectorSize){

    clBufferA = clCreateBuffer(clContext, CL_MEM_READ_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBufferA", __FUNCTION__);
    clBufferB = clCreateBuffer(clContext, CL_MEM_READ_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBufferB", __FUNCTION__);
    clBufferC = clCreateBuffer(clContext, CL_MEM_WRITE_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBufferC", __FUNCTION__);
    
    clErr = clEnqueueWriteBuffer(clCommandQueue,clBufferA,CL_FALSE,0,vectorSize * sizeof(float), vectorA, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clBufferB,CL_FALSE,0,vectorSize * sizeof(float), vectorB, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer B", __FUNCTION__);

    clErr = clSetKernelArg(vectorAddKernel,0,sizeof(cl_mem),&clBufferA);
    utils::checkCLError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(vectorAddKernel,1,sizeof(cl_mem),&clBufferB);
    utils::checkCLError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(vectorAddKernel,2,sizeof(cl_mem),&clBufferC);
    utils::checkCLError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(vectorAddKernel,3,sizeof(cl_int),&vectorSize);
    utils::checkCLError(clErr, "Unable to set kernel param 3", __FUNCTION__);

    size_t globalSize = vectorSize;

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, vectorAddKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    clEnqueueReadBuffer(clCommandQueue, clBufferC, CL_TRUE, 0, vectorSize * sizeof(float), outputVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer", __FUNCTION__);

    clReleaseMemObject(clBufferA);
    clReleaseMemObject(clBufferB);
    clReleaseMemObject(clBufferC);

} 

void gpu::vectorElemMult(float *vectorA, float *vectorB, float *outputVector, int vectorSize){

    clBufferA = clCreateBuffer(clContext, CL_MEM_READ_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBufferA", __FUNCTION__);
    clBufferB = clCreateBuffer(clContext, CL_MEM_READ_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBufferB", __FUNCTION__);
    clBufferC = clCreateBuffer(clContext, CL_MEM_WRITE_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBufferC", __FUNCTION__);
    
    clErr = clEnqueueWriteBuffer(clCommandQueue,clBufferA,CL_FALSE,0,vectorSize * sizeof(float), vectorA, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clBufferB,CL_FALSE,0,vectorSize * sizeof(float), vectorB, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer B", __FUNCTION__);

    clErr = clSetKernelArg(vectorElemMultKernel,0,sizeof(cl_mem),&clBufferA);
    utils::checkCLError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(vectorElemMultKernel,1,sizeof(cl_mem),&clBufferB);
    utils::checkCLError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(vectorElemMultKernel,2,sizeof(cl_mem),&clBufferC);
    utils::checkCLError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(vectorElemMultKernel,3,sizeof(cl_int),&vectorSize);
    utils::checkCLError(clErr, "Unable to set kernel param 3", __FUNCTION__);

    size_t globalSize = vectorSize;

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, vectorElemMultKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    clEnqueueReadBuffer(clCommandQueue, clBufferC, CL_TRUE, 0, vectorSize * sizeof(float), outputVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer", __FUNCTION__);

    clReleaseMemObject(clBufferA);
    clReleaseMemObject(clBufferB);
    clReleaseMemObject(clBufferC);

}

void gpu::vectorSelectiveAdd(float *vectorA, float *outputVector, int rows, int cols){
    
    size_t globalSize = cols;

    clBufferA = clCreateBuffer(clContext, CL_MEM_READ_ONLY, rows * (cols + 1) * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBufferA", __FUNCTION__);
    clBufferB = clCreateBuffer(clContext, CL_MEM_WRITE_ONLY, globalSize * sizeof(float), NULL, &clErr); 
    utils::checkCLError(clErr, "Unable to create clBufferB", __FUNCTION__);
    
    clErr = clEnqueueWriteBuffer(clCommandQueue,clBufferA,CL_FALSE,0, rows * (cols + 1) * sizeof(float), vectorA, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);

    clErr = clSetKernelArg(vectorSelectiveAddKernel,0,sizeof(cl_mem),&clBufferA);
    utils::checkCLError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(vectorSelectiveAddKernel,1,sizeof(cl_mem),&clBufferB);
    utils::checkCLError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(vectorSelectiveAddKernel,2,sizeof(cl_int),&rows);
    utils::checkCLError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(vectorSelectiveAddKernel,3,sizeof(cl_int),&cols);
    utils::checkCLError(clErr, "Unable to set kernel param 3", __FUNCTION__);

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, vectorSelectiveAddKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    clEnqueueReadBuffer(clCommandQueue, clBufferB, CL_TRUE, 0, globalSize * sizeof(float), outputVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer", __FUNCTION__);

    clReleaseMemObject(clBufferA);
    clReleaseMemObject(clBufferB);

}

void gpu::snpComputeNetGain(int n, int m, float *stateVector, float *lossVector, float *gainVector, float *netGainVector){

    size_t globalSize = m;
   
    clErr = clEnqueueWriteBuffer(clCommandQueue,clStateBuffer,CL_FALSE,0, m * sizeof(float), stateVector, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clLossBuffer,CL_FALSE,0,  m * sizeof(float), lossVector, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer B", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clGainBuffer,CL_FALSE,0, m * sizeof(float), gainVector, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer C", __FUNCTION__);

    clErr = clSetKernelArg(snpComputeNetGainKernel,0,sizeof(cl_int),&n);
    utils::checkCLError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(snpComputeNetGainKernel,1,sizeof(cl_int),&m);
    utils::checkCLError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(snpComputeNetGainKernel,2,sizeof(cl_mem),&clStateBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(snpComputeNetGainKernel,3,sizeof(cl_mem),&clLossBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpComputeNetGainKernel,4,sizeof(cl_mem),&clGainBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpComputeNetGainKernel,5,sizeof(cl_mem),&clNetGainBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 5", __FUNCTION__);

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, snpComputeNetGainKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    clEnqueueReadBuffer(clCommandQueue, clNetGainBuffer, CL_TRUE, 0, m * sizeof(float), netGainVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer", __FUNCTION__);

}


void gpu::snpPostCompute(int n, int m,  float *rules, float *transitionVector){

    size_t globalSize = n;

    clErr = clEnqueueWriteBuffer(clCommandQueue,clRulesBuffer,CL_FALSE,0, 3 * n * sizeof(float), rules, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clTransitionBuffer,CL_FALSE,0, n * ( m + 1) * sizeof(float), transitionVector, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer B", __FUNCTION__);

    clErr = clSetKernelArg(snpPostComputeKernel,0,sizeof(cl_int),&n);
    utils::checkCLError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(snpPostComputeKernel,1,sizeof(cl_int),&m);
    utils::checkCLError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(snpPostComputeKernel,2,sizeof(cl_mem),&clRulesBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(snpPostComputeKernel,3,sizeof(cl_mem),&clTransitionBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpPostComputeKernel,4,n * sizeof(float),NULL);
    utils::checkCLError(clErr, "Unable to set kernel param 4", __FUNCTION__);

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, snpPostComputeKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    clEnqueueReadBuffer(clCommandQueue, clRulesBuffer, CL_TRUE, 0, n * 3 * sizeof(float), rules, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer A", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clTransitionBuffer, CL_TRUE, 0, n * (m + 1) * sizeof(float), transitionVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer B", __FUNCTION__);

}


void gpu::snpReset(int n, int m,  float *lossVector, float *gainVector, float *netGainVector, float *neuronFlags){

    size_t globalSize = m;

    clErr = clSetKernelArg(snpResetKernel,0,sizeof(cl_int),&n);
    utils::checkCLError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(snpResetKernel,1,sizeof(cl_int),&m);
    utils::checkCLError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(snpResetKernel,2,sizeof(cl_mem),&clLossBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(snpResetKernel,3,sizeof(cl_mem),&clGainBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpResetKernel,4,sizeof(cl_mem),&clNetGainBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 4", __FUNCTION__);
    clErr = clSetKernelArg(snpResetKernel,5,sizeof(cl_mem),&clNeuronFlagsBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 5", __FUNCTION__);

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, snpResetKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    clEnqueueReadBuffer(clCommandQueue, clLossBuffer, CL_TRUE, 0, globalSize * sizeof(float), lossVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer A", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clGainBuffer, CL_TRUE, 0, globalSize * sizeof(float), gainVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer B", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clNetGainBuffer, CL_TRUE, 0, globalSize * sizeof(float), netGainVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer C", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clNeuronFlagsBuffer, CL_TRUE, 0, m * sizeof(float), neuronFlags, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer C", __FUNCTION__);

}


void gpu::snpSetStates(int n, int m,  float *configVector, float *spikingVector, float* rules,  float* delays,  float* lossVector,
            float* stateVector,  float* transitionVector){
    
    size_t globalSize = n;

    clErr = clEnqueueWriteBuffer(clCommandQueue,clConfigBuffer,CL_FALSE,0, m * sizeof(float), configVector, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clSpikingBuffer,CL_FALSE,0, n * sizeof(float), spikingVector, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer B", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clRulesBuffer,CL_FALSE,0, 3 * n * sizeof(float), rules, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer C", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clDelaysBuffer,CL_FALSE,0, n * sizeof(float), delays, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer D", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clLossBuffer,CL_FALSE,0, m * sizeof(float), lossVector, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer E", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clStateBuffer,CL_FALSE,0, m * sizeof(float), stateVector, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer F", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clTransitionBuffer,CL_FALSE,0, n * (1 + m) * sizeof(float), transitionVector, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue write clBuffer G", __FUNCTION__);

    clErr = clSetKernelArg(snpSetStatesKernel,0,sizeof(cl_int),&n);
    utils::checkCLError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,1,sizeof(cl_int),&m);
    utils::checkCLError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,2,sizeof(cl_mem),&clConfigBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,3,sizeof(cl_mem),&clSpikingBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,4,sizeof(cl_mem),&clRulesBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 4", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,5,sizeof(cl_mem),&clDelaysBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 5", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,6,sizeof(cl_mem),&clLossBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 6", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,7,sizeof(cl_mem),&clStateBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 7", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,8,sizeof(cl_mem),&clTransitionBuffer);
    utils::checkCLError(clErr, "Unable to set kernel param 8", __FUNCTION__);

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, snpSetStatesKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    utils::checkCLError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    clEnqueueReadBuffer(clCommandQueue, clConfigBuffer, CL_TRUE, 0, m * sizeof(float), configVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer A", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clSpikingBuffer, CL_TRUE, 0, n * sizeof(float), spikingVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer B", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clRulesBuffer, CL_TRUE, 0, m * sizeof(float), rules, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer C", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clLossBuffer, CL_TRUE, 0, m * sizeof(float), lossVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer E", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clStateBuffer, CL_TRUE, 0, m * sizeof(float), stateVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer F", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clTransitionBuffer, CL_TRUE, 0, (m + 1) * n  * sizeof(float), transitionVector, 0, NULL, NULL); 
    utils::checkCLError(clErr, "Unable to enqueue read clBuffer G", __FUNCTION__);

}

void matchRuleRegex(int threadId, std::string regex, std::string str, float* spikingVector, float* neuronFlag, float* rules){
    re2::StringPiece input(str);
    spikingVector[threadId] = re2::RE2::FullMatch(input,regex);
}

void matchRulesRegex(std::string *regexVector, float* rules, float* configVector, float* spikingVector, float* neuronFlags,  int n){

    std::thread threads[n];

    for(int i = 0; i < n; i++){
        std::string expandedRegex = regexVector[i];
        std::string spikeString = utils::expandRegex("a^"+ boost::lexical_cast<std::string>(configVector[(int)rules[3 * i] - 1]));
        threads[i] = std::thread(matchRuleRegex, i, expandedRegex, spikeString, spikingVector, neuronFlags, rules);
    }
    
    for(int i = 0; i < n; i++){
        threads[i].join();
    }
}

void getMemUsage(double& vmUsage, double& residentSet){

    vmUsage     = 0.0;
    residentSet = 0.0;

    // 'file' stat seems to give the most reliable results
    //
    std::ifstream stat_stream("/proc/self/stat",std::ios_base::in);

    // dummy vars for leading entries in stat that we don't care about
    //
    std::string pid, comm, state, ppid, pgrp, session, tty_nr;
    std::string tpgid, flags, minflt, cminflt, majflt, cmajflt;
    std::string utime, stime, cutime, cstime, priority, nice;
    std::string O, itrealvalue, starttime;

    // the two fields we want
    //
    unsigned long vsize;
    long rss;

    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
        >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
        >> utime >> stime >> cutime >> cstime >> priority >> nice
        >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

    stat_stream.close();

    long pageSizeKb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    vmUsage     = vsize / 1024.0;
    residentSet = rss * pageSizeKb;
}
