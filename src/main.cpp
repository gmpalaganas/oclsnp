#include "main.h"

double runtime = 0;

// To be configured to accept input file name
// and number of steps
int main(int argc, char **argv){

    if(argc != 2){
        std::cout << "Usage: <compiled_file> <input_binary_file>" << std::endl;
        std::cout << "I.e ./test inputs/2input_sort.bin" << std::endl;
        exit(1);
    }

    snp.loadSNPFromFile(argv[1]);
    snp.printSNPContents();

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
    lhs = new float[n]();

    //Copy snp initial config to configVector
    copyIntArrIntoFloatArr(snp.initConfig, configVector, m); 

    //Copy snp rule delays into delays vector
    copyIntArrIntoFloatArr(snp.ruleDelays, delays, n);

    std::fill_n(stateVector, m, 1);

    for(int i = 0; i < n; i++){
        int index = i * 3;
        rules[index] = snp.ruleIds[i];
        rules[index + 1] = -1;
        rules[index + 2] = -(snp.ruleConsumedSpikes[i]);
    }

    printVectorAs2DArray(rules, n, 3);

    for(int i = 0; i < n; i++){
        for(int j = 1; j < m + 1; j++){
            if(snp.synapseMatrix[snp.ruleIds[i]][j] == 1)
                transitionVector[i * (m + 1) + j] = snp.ruleProducedSpikes[i];
        }
    }
    
    for(int i = 0; i < n; i++)
        lhs[i] = snp.getRuleRegexCode(i);
    
    int step = 1;

    //Simulation Proper
    do{
    
        gpu::snpDetermineRules(n, m, configVector, spikingVector, rules, lhs);

        if(!areRulesApplicable(spikingVector,n))
                break;

        std::cout << "************************************" << std::endl;
        std::cout << "At step " << step << ":" << std::endl;

        gpu::snpSetStates(n, m, configVector, spikingVector, rules, delays, lossVector, stateVector, transitionVector);
        gpu::vectorSelectiveAdd(transitionVector, gainVector, n, m);
        gpu::snpComputeNetGain(n, m, stateVector, lossVector, gainVector, netGainVector);
        gpu::vectorAdd(netGainVector,configVector,configVector,m);
        gpu::snpPostCompute(n, m, rules, transitionVector);
        gpu::snpReset(n, m, lossVector, gainVector, netGainVector);


        std::cout << "CHOSEN RULES" << std::endl;
        for(int j = 0; j < n; j++){
            if(spikingVector[j] == 1){
                std::cout << "Rule from neuron " << snp.ruleIds[j] << ": " << snp.getRule(j) << std::endl;
            }
        }

        std::cout << "------------------------------------" << std::endl;

        for(int j = 0; j < m; j++){
            std::cout << "NEURON " << j+1 << ":" << snp.neuronLabels[j] << std::endl;
            std::cout << "Spikes: " << configVector[j] << std::endl;
        }

        step++;
    }while(areRulesApplicable(spikingVector,n));

    std::cout << "************************************" << std::endl;
    std::cout << "Configuration after " << step - 1 << " steps:\n";
    for(int i = 0; i < m; i++){
        std::cout << "NEURON " << i+1 << ":" << snp.neuronLabels[i] << std::endl;
        std::cout << "Spikes: " << configVector[i];
        std::cout << " State: " << stateVector[i] << std::endl << std::endl;
    }
    std::cout << "Execution time: " << runtime << std::endl;

    cleanup();

    return 0;

}

//Check for errors, if errors were detected, cleanup
inline void checkError(cl_int err, std::string msg, std::string fncName){
    if(err != CL_SUCCESS){
        std::cerr << "Error in " << fncName << "(): " << msg <<std::endl;
        std::cerr << "Error Code: " << getCLError(err) << std::endl;
        

        cleanup();
        exit(1);    
    }
}


//Check for OpenCL build errors
inline void checkError(cl_int err, std::string msg, std::string fncName, cl_program program){

    if (err == CL_BUILD_PROGRAM_FAILURE) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(program, clDevice, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, clDevice, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        std::cout << log << std::endl;
    }

    checkError(err,msg,fncName);
}

//Display corresponding OpenCL error based on error code
inline std::string getCLError(cl_int err){

    switch(err){
        case 0: return "CL_SUCCESS";
        case -1: return "CL_DEVICE_NOT_FOUND";
        case -2: return "CL_DEVICE_NOT_AVAILABLE";
        case -3: return "CL_COMPILER_NOT_AVAILABLE";
        case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case -5: return "CL_OUT_OF_RESOURCES";
        case -6: return "CL_OUT_OF_HOST_MEMORY";
        case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case -8: return "CL_MEM_COPY_OVERLAP";
        case -9: return "CL_IMAGE_FORMAT_MISMATCH";
        case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case -11: return "CL_BUILD_PROGRAM_FAILURE";
        case -12: return "CL_MAP_FAILURE";
        case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case -15: return "CL_COMPILE_PROGRAM_FAILURE";
        case -16: return "CL_LINKER_NOT_AVAILABLE";
        case -17: return "CL_LINK_PROGRAM_FAILURE";
        case -18: return "CL_DEVICE_PARTITION_FAILED";
        case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

        case -30: return "CL_INVALID_VALUE";
        case -31: return "CL_INVALID_DEVICE_TYPE";
        case -32: return "CL_INVALID_PLATFORM";
        case -33: return "CL_INVALID_DEVICE";
        case -34: return "CL_INVALID_CONTEXT";
        case -35: return "CL_INVALID_QUEUE_PROPERTIES";
        case -36: return "CL_INVALID_COMMAND_QUEUE";
        case -37: return "CL_INVALID_HOST_PTR";
        case -38: return "CL_INVALID_MEM_OBJECT";
        case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case -40: return "CL_INVALID_IMAGE_SIZE";
        case -41: return "CL_INVALID_SAMPLER";
        case -42: return "CL_INVALID_BINARY";
        case -43: return "CL_INVALID_BUILD_OPTIONS";
        case -44: return "CL_INVALID_PROGRAM";
        case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
        case -46: return "CL_INVALID_KERNEL_NAME";
        case -47: return "CL_INVALID_KERNEL_DEFINITION";
        case -48: return "CL_INVALID_KERNEL";
        case -49: return "CL_INVALID_ARG_INDEX";
        case -50: return "CL_INVALID_ARG_VALUE";
        case -51: return "CL_INVALID_ARG_SIZE";
        case -52: return "CL_INVALID_KERNEL_ARGS";
        case -53: return "CL_INVALID_WORK_DIMENSION";
        case -54: return "CL_INVALID_WORK_GROUP_SIZE";
        case -55: return "CL_INVALID_WORK_ITEM_SIZE";
        case -56: return "CL_INVALID_GLOBAL_OFFSET";
        case -57: return "CL_INVALID_EVENT_WAIT_LIST";
        case -58: return "CL_INVALID_EVENT";
        case -59: return "CL_INVALID_OPERATION";
        case -60: return "CL_INVALID_GL_OBJECT";
        case -61: return "CL_INVALID_BUFFER_SIZE";
        case -62: return "CL_INVALID_MIP_LEVEL";
        case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
        case -64: return "CL_INVALID_PROPERTY";
        case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
        case -66: return "CL_INVALID_COMPILER_OPTIONS";
        case -67: return "CL_INVALID_LINKER_OPTIONS";
        case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

        case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
        case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
        case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
        case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
        case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
        case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
        default: return "Unknown OpenCL error";
    }
}

//Release OpenCL variables and delete dynamically allocated variables
void cleanup(){
    std::cout << "Goodbye" << std::endl;
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
    clReleaseKernel(snpDetermineRulesKernel);
    clReleaseProgram(snpDetermineRulesProgram);
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
    clReleaseMemObject(clLHSBuffer);

    delete[] configVector;
    delete[] spikingVector;
    delete[] stateVector;
    delete[] lossVector;
    delete[] gainVector;
    delete[] netGainVector;
    delete[] rules;
    delete[] delays;
    delete[] transitionVector;
    delete[] lhs;

}

//Initialize OpenCL variables i.e. Platform IDs and Device IDs
void initCL(int n, int m){

    clErr = clGetPlatformIDs(1, &clPlatform, NULL);  
    checkError(clErr, "Unable to retrieve platforms", __FUNCTION__);

    clErr = clGetDeviceIDs(clPlatform, CL_DEVICE_TYPE_GPU, 1, &clDevice, NULL);
    checkError(clErr, "Unable to retrieve devices", __FUNCTION__);

    clContext = clCreateContext(0, 1, &clDevice, NULL, NULL, &clErr);
    checkError(clErr, "Unable to create context", __FUNCTION__);

    clCommandQueue = clCreateCommandQueue(clContext, clDevice, 0, &clErr);
    checkError(clErr, "Unable to create command queue", __FUNCTION__);

    clConfigBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clSpikingBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, n * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clStateBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBuffer", __FUNCTION__);
   
    clLossBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clGainBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clNetGainBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, m * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clRulesBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, 3 * n * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clDelaysBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, n * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clTransitionBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, n * (m + 1) * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBuffer", __FUNCTION__);

    clLHSBuffer = clCreateBuffer(clContext, CL_MEM_READ_ONLY, n * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBuffer", __FUNCTION__);

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
    initSNPDetermineRulesKernel();
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
    checkError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(vectorAddProgram,1,&clDevice,NULL,NULL,NULL);
    checkError(clErr, "Unable to build program", __FUNCTION__, vectorAddProgram);

    vectorAddKernel = clCreateKernel(vectorAddProgram, "vectorAdd", &clErr);
    checkError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initVectorElemMultKernel(){

    //char *src = loadProgramSource(VECTOR_ELEM_MULT_SRC);
    std::ifstream file(VECTOR_ELEM_MULT_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();


    vectorElemMultProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    checkError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(vectorElemMultProgram,1,&clDevice,NULL,NULL,NULL);
    checkError(clErr, "Unable to build program", __FUNCTION__, vectorElemMultProgram);

    vectorElemMultKernel = clCreateKernel(vectorElemMultProgram, "vectorElemMult", &clErr);
    checkError(clErr, "Unable to create kernel", __FUNCTION__);
   
}


void initVectorSelectiveAddKernel(){
    //char *src = loadProgramSource(VECTOR_SELECTIVE_ADD_SRC);

    std::ifstream file(VECTOR_SELECTIVE_ADD_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    vectorSelectiveAddProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    checkError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(vectorSelectiveAddProgram,1,&clDevice,NULL,NULL,NULL);
    checkError(clErr, "Unable to build program", __FUNCTION__, vectorSelectiveAddProgram);

    vectorSelectiveAddKernel = clCreateKernel(vectorSelectiveAddProgram, "vectorSelectiveAdd", &clErr);
    checkError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initSNPComputeNetGainKernel(){
    //char *src = loadProgramSource(SNP_COMPUTE_NET_GAIN_SRC);

    std::ifstream file(SNP_COMPUTE_NET_GAIN_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    snpComputeNetGainProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    checkError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(snpComputeNetGainProgram,1,&clDevice,NULL,NULL,NULL);
    checkError(clErr, "Unable to build program", __FUNCTION__, snpComputeNetGainProgram);

    snpComputeNetGainKernel = clCreateKernel(snpComputeNetGainProgram, "snpComputeNetGain", &clErr);
    checkError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initSNPDetermineRulesKernel(){
    //char *src = loadProgramSource(SNP_DETERMINE_RULES_SRC);

    std::ifstream file(SNP_DETERMINE_RULES_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    snpDetermineRulesProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    checkError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(snpDetermineRulesProgram,1,&clDevice,NULL,NULL,NULL);
    checkError(clErr, "Unable to build program", __FUNCTION__, snpDetermineRulesProgram);

    snpDetermineRulesKernel = clCreateKernel(snpDetermineRulesProgram, "snpDetermineRules", &clErr);
    checkError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initSNPPostComputeKernel(){
    //char *src = loadProgramSource(SNP_POST_COMPUTE_SRC);

    std::ifstream file(SNP_POST_COMPUTE_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    snpPostComputeProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    checkError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(snpPostComputeProgram,1,&clDevice,NULL,NULL,NULL);
    checkError(clErr, "Unable to build program", __FUNCTION__, snpPostComputeProgram);

    snpPostComputeKernel = clCreateKernel(snpPostComputeProgram, "snpPostCompute", &clErr);
    checkError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initSNPResetKernel(){
    //char *src = loadProgramSource(SNP_RESET_SRC);

    std::ifstream file(SNP_RESET_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    snpResetProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    checkError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(snpResetProgram,1,&clDevice,NULL,NULL,NULL);
    checkError(clErr, "Unable to build program", __FUNCTION__, snpResetProgram);

    snpResetKernel = clCreateKernel(snpResetProgram, "snpReset", &clErr);
    checkError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

void initSNPSetStatesKernel(){
    //char *src = loadProgramSource(SNP_SET_STATES_SRC);

    std::ifstream file(SNP_SET_STATES_SRC);
    std::string source((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    file.close();

    char *src = (char *)source.c_str();

    snpSetStatesProgram = clCreateProgramWithSource(clContext,1,(const char **)&src, NULL, &clErr);
    checkError(clErr, "Unable to create program", __FUNCTION__);
    clErr = clBuildProgram(snpSetStatesProgram,1,&clDevice,NULL,NULL,NULL);
    checkError(clErr, "Unable to build program", __FUNCTION__, snpSetStatesProgram);

    snpSetStatesKernel = clCreateKernel(snpSetStatesProgram, "snpSetStates", &clErr);
    checkError(clErr, "Unable to create kernel", __FUNCTION__);
   
}

/*===End OpenCL Kernel Initializations===*/

void gpu::vectorAdd(float *vectorA, float *vectorB, float *outputVector, int vectorSize){

    clBufferA = clCreateBuffer(clContext, CL_MEM_READ_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBufferA", __FUNCTION__);
    clBufferB = clCreateBuffer(clContext, CL_MEM_READ_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBufferB", __FUNCTION__);
    clBufferC = clCreateBuffer(clContext, CL_MEM_WRITE_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBufferC", __FUNCTION__);
    
    clErr = clEnqueueWriteBuffer(clCommandQueue,clBufferA,CL_FALSE,0,vectorSize * sizeof(float), vectorA, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clBufferB,CL_FALSE,0,vectorSize * sizeof(float), vectorB, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer B", __FUNCTION__);

    clErr = clSetKernelArg(vectorAddKernel,0,sizeof(cl_mem),&clBufferA);
    checkError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(vectorAddKernel,1,sizeof(cl_mem),&clBufferB);
    checkError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(vectorAddKernel,2,sizeof(cl_mem),&clBufferC);
    checkError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(vectorAddKernel,3,sizeof(cl_int),&vectorSize);
    checkError(clErr, "Unable to set kernel param 3", __FUNCTION__);

    size_t globalSize = vectorSize;

    std::clock_t begin = std::clock();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, vectorAddKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::clock_t end = std::clock();

    runtime += double(end - begin) / CLOCKS_PER_SEC;

    clEnqueueReadBuffer(clCommandQueue, clBufferC, CL_TRUE, 0, vectorSize * sizeof(float), outputVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer", __FUNCTION__);

    clReleaseMemObject(clBufferA);
    clReleaseMemObject(clBufferB);
    clReleaseMemObject(clBufferC);

} 

void gpu::vectorElemMult(float *vectorA, float *vectorB, float *outputVector, int vectorSize){

    clBufferA = clCreateBuffer(clContext, CL_MEM_READ_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBufferA", __FUNCTION__);
    clBufferB = clCreateBuffer(clContext, CL_MEM_READ_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBufferB", __FUNCTION__);
    clBufferC = clCreateBuffer(clContext, CL_MEM_WRITE_ONLY, vectorSize * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBufferC", __FUNCTION__);
    
    clErr = clEnqueueWriteBuffer(clCommandQueue,clBufferA,CL_FALSE,0,vectorSize * sizeof(float), vectorA, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clBufferB,CL_FALSE,0,vectorSize * sizeof(float), vectorB, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer B", __FUNCTION__);

    clErr = clSetKernelArg(vectorElemMultKernel,0,sizeof(cl_mem),&clBufferA);
    checkError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(vectorElemMultKernel,1,sizeof(cl_mem),&clBufferB);
    checkError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(vectorElemMultKernel,2,sizeof(cl_mem),&clBufferC);
    checkError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(vectorElemMultKernel,3,sizeof(cl_int),&vectorSize);
    checkError(clErr, "Unable to set kernel param 3", __FUNCTION__);

    size_t globalSize = vectorSize;

    std::clock_t begin = std::clock();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, vectorElemMultKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::clock_t end = std::clock();

    runtime += double(end - begin) / CLOCKS_PER_SEC;

    clEnqueueReadBuffer(clCommandQueue, clBufferC, CL_TRUE, 0, vectorSize * sizeof(float), outputVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer", __FUNCTION__);

    clReleaseMemObject(clBufferA);
    clReleaseMemObject(clBufferB);
    clReleaseMemObject(clBufferC);

}

void gpu::vectorSelectiveAdd(float *vectorA, float *outputVector, int rows, int cols){
    
    size_t globalSize = cols;

    clBufferA = clCreateBuffer(clContext, CL_MEM_READ_ONLY, rows * (cols + 1) * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBufferA", __FUNCTION__);
    clBufferB = clCreateBuffer(clContext, CL_MEM_WRITE_ONLY, globalSize * sizeof(float), NULL, &clErr); 
    checkError(clErr, "Unable to create clBufferB", __FUNCTION__);
    
    clErr = clEnqueueWriteBuffer(clCommandQueue,clBufferA,CL_FALSE,0, rows * (cols + 1) * sizeof(float), vectorA, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);

    clErr = clSetKernelArg(vectorSelectiveAddKernel,0,sizeof(cl_mem),&clBufferA);
    checkError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(vectorSelectiveAddKernel,1,sizeof(cl_mem),&clBufferB);
    checkError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(vectorSelectiveAddKernel,2,sizeof(cl_int),&rows);
    checkError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(vectorSelectiveAddKernel,3,sizeof(cl_int),&cols);
    checkError(clErr, "Unable to set kernel param 3", __FUNCTION__);

    std::clock_t begin = std::clock();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, vectorSelectiveAddKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::clock_t end = std::clock();

    runtime += double(end - begin) / CLOCKS_PER_SEC;

    clEnqueueReadBuffer(clCommandQueue, clBufferB, CL_TRUE, 0, globalSize * sizeof(float), outputVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer", __FUNCTION__);

    clReleaseMemObject(clBufferA);
    clReleaseMemObject(clBufferB);

}

void gpu::snpComputeNetGain(int n, int m, float *stateVector, float *lossVector, float *gainVector, float *netGainVector){

    size_t globalSize = m;
   
    clErr = clEnqueueWriteBuffer(clCommandQueue,clStateBuffer,CL_FALSE,0, m * sizeof(float), stateVector, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clLossBuffer,CL_FALSE,0,  m * sizeof(float), lossVector, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer B", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clGainBuffer,CL_FALSE,0, m * sizeof(float), gainVector, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer C", __FUNCTION__);

    clErr = clSetKernelArg(snpComputeNetGainKernel,0,sizeof(cl_int),&n);
    checkError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(snpComputeNetGainKernel,1,sizeof(cl_int),&m);
    checkError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(snpComputeNetGainKernel,2,sizeof(cl_mem),&clStateBuffer);
    checkError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(snpComputeNetGainKernel,3,sizeof(cl_mem),&clLossBuffer);
    checkError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpComputeNetGainKernel,4,sizeof(cl_mem),&clGainBuffer);
    checkError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpComputeNetGainKernel,5,sizeof(cl_mem),&clNetGainBuffer);
    checkError(clErr, "Unable to set kernel param 5", __FUNCTION__);

    std::clock_t begin = clock();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, snpComputeNetGainKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::clock_t end = clock();

    runtime += double(end - begin) / CLOCKS_PER_SEC;

    clEnqueueReadBuffer(clCommandQueue, clNetGainBuffer, CL_TRUE, 0, m * sizeof(float), netGainVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer", __FUNCTION__);

}



void gpu::snpDetermineRules(int n, int m,  float *configVector, float *spikingVector, float *rules, float *lhs){
    size_t globalSize = n;
    
    clErr = clEnqueueWriteBuffer(clCommandQueue,clConfigBuffer,CL_FALSE,0, m * sizeof(float), configVector, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clRulesBuffer,CL_FALSE,0,3 * n * sizeof(float), rules, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer C", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clLHSBuffer,CL_FALSE,0,n * sizeof(float), lhs, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer D", __FUNCTION__);

    clErr = clSetKernelArg(snpDetermineRulesKernel,0,sizeof(cl_int),&n);
    checkError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(snpDetermineRulesKernel,1,sizeof(cl_int),&m);
    checkError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(snpDetermineRulesKernel,2,sizeof(cl_mem),&clConfigBuffer);
    checkError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(snpDetermineRulesKernel,3,sizeof(cl_mem),&clSpikingBuffer);
    checkError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpDetermineRulesKernel,4,sizeof(cl_mem),&clRulesBuffer);
    checkError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpDetermineRulesKernel,5,sizeof(cl_mem),&clLHSBuffer);
    checkError(clErr, "Unable to set kernel param 5", __FUNCTION__);

    std::clock_t begin = clock();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, snpDetermineRulesKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::clock_t end = clock();

    runtime += double(end - begin) / CLOCKS_PER_SEC;

    clEnqueueReadBuffer(clCommandQueue, clSpikingBuffer, CL_TRUE, 0, n * sizeof(float), spikingVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer", __FUNCTION__);

}

void gpu::snpPostCompute(int n, int m,  float *rules, float *transitionVector){

    size_t globalSize = n;

    clErr = clEnqueueWriteBuffer(clCommandQueue,clRulesBuffer,CL_FALSE,0, 3 * n * sizeof(float), rules, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clTransitionBuffer,CL_FALSE,0, n * ( m + 1) * sizeof(float), transitionVector, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer B", __FUNCTION__);

    clErr = clSetKernelArg(snpPostComputeKernel,0,sizeof(cl_int),&n);
    checkError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(snpPostComputeKernel,1,sizeof(cl_int),&m);
    checkError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(snpPostComputeKernel,2,sizeof(cl_mem),&clRulesBuffer);
    checkError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(snpPostComputeKernel,3,sizeof(cl_mem),&clTransitionBuffer);
    checkError(clErr, "Unable to set kernel param 3", __FUNCTION__);

    std::clock_t begin = clock();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, snpPostComputeKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::clock_t end = clock();

    runtime += double(end - begin) / CLOCKS_PER_SEC;

    clEnqueueReadBuffer(clCommandQueue, clRulesBuffer, CL_TRUE, 0, n * 3 * sizeof(float), rules, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer A", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clTransitionBuffer, CL_TRUE, 0, n * (m + 1) * sizeof(float), transitionVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer B", __FUNCTION__);

}


void gpu::snpReset(int n, int m,  float *lossVector, float *gainVector, float *netGainVector){

    size_t globalSize = m;

    clErr = clSetKernelArg(snpResetKernel,0,sizeof(cl_int),&n);
    checkError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(snpResetKernel,1,sizeof(cl_int),&m);
    checkError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(snpResetKernel,2,sizeof(cl_mem),&clLossBuffer);
    checkError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(snpResetKernel,3,sizeof(cl_mem),&clGainBuffer);
    checkError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpResetKernel,4,sizeof(cl_mem),&clNetGainBuffer);
    checkError(clErr, "Unable to set kernel param 4", __FUNCTION__);

    std::clock_t begin = clock();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, snpResetKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::clock_t end = clock();

    runtime += double(end - begin) / CLOCKS_PER_SEC;

    clEnqueueReadBuffer(clCommandQueue, clLossBuffer, CL_TRUE, 0, globalSize * sizeof(float), lossVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer A", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clGainBuffer, CL_TRUE, 0, globalSize * sizeof(float), gainVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer B", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clNetGainBuffer, CL_TRUE, 0, globalSize * sizeof(float), netGainVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer C", __FUNCTION__);

}


void gpu::snpSetStates(int n, int m,  float *configVector, float *spikingVector, float* rules,  float* delays,  float* lossVector,
            float* stateVector,  float* transitionVector){
    
    size_t globalSize = n;

    clErr = clEnqueueWriteBuffer(clCommandQueue,clConfigBuffer,CL_FALSE,0, m * sizeof(float), configVector, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer A", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clSpikingBuffer,CL_FALSE,0, n * sizeof(float), spikingVector, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer B", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clRulesBuffer,CL_FALSE,0, 3 * n * sizeof(float), rules, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer C", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clDelaysBuffer,CL_FALSE,0, n * sizeof(float), delays, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer D", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clLossBuffer,CL_FALSE,0, m * sizeof(float), lossVector, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer E", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clStateBuffer,CL_FALSE,0, m * sizeof(float), stateVector, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer F", __FUNCTION__);
    clErr = clEnqueueWriteBuffer(clCommandQueue,clTransitionBuffer,CL_FALSE,0, n * (1 + m) * sizeof(float), transitionVector, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue write clBuffer G", __FUNCTION__);

    clErr = clSetKernelArg(snpSetStatesKernel,0,sizeof(cl_int),&n);
    checkError(clErr, "Unable to set kernel param 0", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,1,sizeof(cl_int),&m);
    checkError(clErr, "Unable to set kernel param 1", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,2,sizeof(cl_mem),&clConfigBuffer);
    checkError(clErr, "Unable to set kernel param 2", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,3,sizeof(cl_mem),&clSpikingBuffer);
    checkError(clErr, "Unable to set kernel param 3", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,4,sizeof(cl_mem),&clRulesBuffer);
    checkError(clErr, "Unable to set kernel param 4", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,5,sizeof(cl_mem),&clDelaysBuffer);
    checkError(clErr, "Unable to set kernel param 5", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,6,sizeof(cl_mem),&clLossBuffer);
    checkError(clErr, "Unable to set kernel param 6", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,7,sizeof(cl_mem),&clStateBuffer);
    checkError(clErr, "Unable to set kernel param 7", __FUNCTION__);
    clErr = clSetKernelArg(snpSetStatesKernel,8,sizeof(cl_mem),&clTransitionBuffer);
    checkError(clErr, "Unable to set kernel param 8", __FUNCTION__);

    std::clock_t begin = clock();
    clErr = clEnqueueNDRangeKernel(clCommandQueue, snpSetStatesKernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);
    checkError(clErr, "Unable to enqueue kernel", __FUNCTION__);
    std::clock_t end = clock();

    runtime += double(end - begin) / CLOCKS_PER_SEC;

    clEnqueueReadBuffer(clCommandQueue, clConfigBuffer, CL_TRUE, 0, m * sizeof(float), configVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer A", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clSpikingBuffer, CL_TRUE, 0, n * sizeof(float), spikingVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer B", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clRulesBuffer, CL_TRUE, 0, m * sizeof(float), rules, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer C", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clLossBuffer, CL_TRUE, 0, m * sizeof(float), lossVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer E", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clStateBuffer, CL_TRUE, 0, m * sizeof(float), stateVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer F", __FUNCTION__);
    clEnqueueReadBuffer(clCommandQueue, clTransitionBuffer, CL_TRUE, 0, (m + 1) * n  * sizeof(float), transitionVector, 0, NULL, NULL); 
    checkError(clErr, "Unable to enqueue read clBuffer G", __FUNCTION__);

}
