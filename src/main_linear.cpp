#include "main_linear.hpp"

std::chrono::microseconds runtime;

// To be configured to accept input file name
// and number of steps
int main(int argc, char **argv){

    std::stringstream outputStream;

    if(argc == 3){
        outputFile = std::ofstream(argv[2]);
    }else if(argc < 2){
        std::cout << "Usage: ./oclsnp <input_binary_file> [output_file]" << std::endl;
        std::cout << "I.e ./oclsnp ../inputs/2input_sort.bin outputs/2input_sort_out.txt" << std::endl;
        exit(1);
    }

    int err;

    err = snp.loadSNPFromFile(argv[1]);
    checkError(err, "Invalid Binary file", __FUNCTION__);

    snp.printSNPContents();

    int n = snp.ruleCount;
    int m = snp.neuronCount;

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
    regexs = new std::string[n]();

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

    for(int i = 0; i < n; i++){
        regexs[i] = expandRegex(snp.getRuleRegex(i));
    }

    
    for(int i = 0; i < n; i++)
        lhs[i] = snp.getRuleRegexCode(i);
    
    int step = 1;

    //Simulation Proper
    do{

        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        matchRulesRegex(regexs, rules, configVector, spikingVector, n);
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

        runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);


        if(!areRulesApplicable(spikingVector,n))
                break;
    
        outputStream << "************************************" << std::endl;
        outputStream << "At step " << step << ":" << std::endl;

        snpSetStates(n, m, configVector, spikingVector, rules, delays, lossVector, stateVector, transitionVector);
        vectorSelectiveAdd(transitionVector, gainVector, n, m);
        snpComputeNetGain(n, m, stateVector, lossVector, gainVector, netGainVector);
        vectorAdd(netGainVector,configVector,configVector,m);
        snpPostCompute(n, m, rules, transitionVector);
        snpReset(n, m, lossVector, gainVector, netGainVector);


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
    outputStream << "Execution time: " << float(runtime.count()) << " ns" << std::endl;

    std::cout << outputStream.str();

    if(outputFile){
        outputFile << outputStream.str();
    }

    cleanup();

    return 0;

}

inline void checkError(int err, std::string msg, std::string fncName){

    if(err == EXECUTE_FAILURE){
        std::cerr << "Error in " << fncName << "(): " << msg << std::endl;

        cleanup();
        exit(1);
    }

}


void cleanup(){
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

void vectorAdd(float *vectorA, float *vectorB, float *outputVector, int vectorSize){
    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < vectorSize; i++)
        outputVector[i] = vectorA[i] + vectorB[i];
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);;
}

void vectorElemMult(float *vectorA, float *vectorB, float *outputVector, int vectorSize){
    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < vectorSize; i++)
        outputVector[i] = vectorA[i] * vectorB[i];
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);;
}

void vectorSelectiveAdd(float *vectorA, float *outputVector, int rows, int cols){
    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < cols; i++){
        for(int j = 0; j < rows; j++){
            if(vectorA[j * (cols + 1)] == 1)
                outputVector[i] += vectorA[j * (cols +1) + (i+1)];
        }
    }
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);;
}

void snpComputeNetGain(int n, int m, float *stateVector, float *lossVector, float *gainVector, float *netGainVector){
    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < m; i++)
        netGainVector[i] = gainVector[i] * stateVector[i] + lossVector[i]; 
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);;
}

void snpDetermineRules(int n, int m,  float *configVector, float *spikingVector, float *rules, float *lhs){
    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < n; i++){
        if((configVector[ (int)rules[i * 3] - 1 ] == lhs[i] || lhs[i] == 0) &&
            configVector[(int)rules[i * 3] - 1] + rules[i * 3 + 2] >= 0){

            spikingVector[i] = 1; 

         }else{
            spikingVector[i] = 0;
        }
    }
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);;
}

void snpPostCompute(int n, int m,  float *rules, float *transitionVector){
    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < n;  i++){
        if(rules[i * 3 + 1] > -1)
            rules[i * 3 + 1] -= 1;
        transitionVector[i * (m + 1)] = 0;
    }
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);;
}

void snpReset(int n, int m,  float *lossVector, float *gainVector, float *netGainVector){
    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < m; i++){
        lossVector[i] = 0;
        gainVector[i] = 0;
        netGainVector[i] = 0;
    }
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);;
}


void snpSetStates(int n, int m,  float *configVector, float *spikingVector, float* rules,  float* delays,  float* lossVector,
        float* stateVector,  float* transitionVector){

    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < n; i++){
        int index = (int)rules[i * 3] - 1;

        if(spikingVector[i] == 1){
            lossVector[index] = rules[i * 3 + 2];
            rules[i * 3 + 1] = delays[i];
            stateVector[index] = 0;
            if(delays[i] == 0){
                transitionVector[i * (m + 1)] = 1;
                stateVector[index] = 1;
            }
        } else if(rules[i * 3 + 1] == 0){
            transitionVector[i * (m + 1)] = 1;
            stateVector[(int)rules[i * m]] = 1;
        }
    }
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);;

}

void matchRuleRegex(std::string regex, std::string str, float* isMatch){
    re2::StringPiece input(str);
    *(isMatch) = re2::RE2::FullMatch(input,regex);
}

void matchRulesRegex(std::string *regexVector, float* rules, float* configVector, float* spikingVector, int vectorSize){

    std::thread threads[vectorSize];

    for(int i = 0; i < vectorSize; i++){
        std::string expandedRegex = regexVector[i];
        std::string spikeString = expandRegex("a^"+ boost::lexical_cast<std::string>(configVector[(int)rules[3 * i] - 1]));
        float *ruleMarkPtr = spikingVector + i;
        threads[i] = std::thread(matchRuleRegex, expandedRegex, spikeString, ruleMarkPtr);
    }
    
    for(int i = 0; i < vectorSize; i++){
        threads[i].join();
    }
}
