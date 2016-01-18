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
#include <ctime>
#include <regex>
#include <boost/regex.hpp>

#include "snp/snp.cpp"
#include "utils/array.cpp"

void cleanup();

bool areRulesApplicable(float *spikingVector, int n);

void vectorAdd(float *vectorA, float *vectorB, float *outputVector, int vectorSize);
void vectorElemMult(float *vectorA, float *vectorB, float *outputVector, int vectorSize);
void vectorSelectiveAdd(float *vectorA, float *outputVector, int rows, int cols);
void snpComputeNetGain(int n, int m, float *stateVector, float *lossVector, float *gainVector, float *netGainVector);
void snpDetermineRules(int n, int m,  float *configVector, float *spikingVector, float *rules, float *lhs);
void snpPostCompute(int n, int m,  float *rules, float *transitionVector);
void snpReset(int n, int m,  float *lossVector, float *gainVector, float *netGainVector);
void snpSetStates(int n, int m,  float *configVector, float *spikingVector, float* rules,  float* delays,  float* lossVector,
        float* stateVector,  float* transitionVector);


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

double runtime = 0;

// To be configured to accept input file name
// and number of steps
int main(int argc, char **argv){


    snp.loadSNPFromFile(argv[1]);
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
    
        std::cout << "************************************" << std::endl;
        std::cout << "At step " << step << ":" << std::endl;

        snpDetermineRules(n, m, configVector, spikingVector, rules, lhs);

        if(!areRulesApplicable(spikingVector,n))
                break;

        snpSetStates(n, m, configVector, spikingVector, rules, delays, lossVector, stateVector, transitionVector);
        vectorSelectiveAdd(transitionVector, gainVector, n, m);
        snpComputeNetGain(n, m, stateVector, lossVector, gainVector, netGainVector);
        vectorAdd(netGainVector,configVector,configVector,m);
        snpPostCompute(n, m, rules, transitionVector);
        snpReset(n, m, lossVector, gainVector, netGainVector);


        std::cout << "CHOSEN RULES" << std::endl;
        for(int j = 0; j < n; j++){
            if(spikingVector[j] == 1){
                std::cout << "Rule from neuron " << snp.ruleIds[j] << ": " << snp.getRule(j) << std::endl;
            }
        }

        std::cout << "------------------------------------" << std::endl;

        for(int j = 0; j < m; j++){
            std::cout << "NEURON " << j+1 << ":" << snp.neuronLabels[j] << std::endl; std::cout << "Spikes: " << configVector[j] << std::endl;
        }

        step++;
    }while(areRulesApplicable(spikingVector,n));

    std::cout << "************************************" << std::endl;
    std::cout << "Configuration after " << step - 1 << " steps:\n";
    for(int i = 0; i < m; i++){
        std::cout << "Neuron " << i << " :" << std::endl;
        std::cout << "Spikes: " << configVector[i];
        std::cout << " State: " << stateVector[i] << std::endl << std::endl;
    }
    std::cout << "Execution time: " << runtime << std::endl;

    cleanup();

    return 0;

}


void cleanup(){
    std::cout << "Goodbye" << std::endl;
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
    std::clock_t begin = clock();
    for(int i = 0; i < vectorSize; i++)
        outputVector[i] = vectorA[i] + vectorB[i];
    std::clock_t end = clock();
    runtime += double(end - begin) / CLOCKS_PER_SEC;
}

void vectorElemMult(float *vectorA, float *vectorB, float *outputVector, int vectorSize){
    std::clock_t begin = clock();
    for(int i = 0; i < vectorSize; i++)
        outputVector[i] = vectorA[i] * vectorB[i];
    std::clock_t end = clock();
    runtime += double(end - begin) / CLOCKS_PER_SEC;
}

void vectorSelectiveAdd(float *vectorA, float *outputVector, int rows, int cols){
    std::clock_t begin = clock();
    for(int i = 0; i < cols; i++){
        for(int j = 0; j < rows; j++){
            if(vectorA[j * (cols + 1)] == 1)
                outputVector[i] += vectorA[j * (cols +1) + (i+1)];
        }
    }
    std::clock_t end = clock();
    runtime += double(end - begin) / CLOCKS_PER_SEC;
}

void snpComputeNetGain(int n, int m, float *stateVector, float *lossVector, float *gainVector, float *netGainVector){
    std::clock_t begin = clock();
    for(int i = 0; i < m; i++)
        netGainVector[i] = gainVector[i] * stateVector[i] + lossVector[i]; 
    std::clock_t end = clock();
    runtime += double(end - begin) / CLOCKS_PER_SEC;
}

void snpDetermineRules(int n, int m,  float *configVector, float *spikingVector, float *rules, float *lhs){
    std::clock_t begin = clock();
    for(int i = 0; i < n; i++){
        if((configVector[ (int)rules[i * 3] - 1 ] == lhs[i] || lhs[i] == 0) &&
            configVector[(int)rules[i * 3] - 1] + rules[i * 3 + 2] >= 0){

            spikingVector[i] = 1; 

         }else{
            spikingVector[i] = 0;
        }
    }
    std::clock_t end = clock();
    runtime += double(end - begin) / CLOCKS_PER_SEC;
}

void snpPostCompute(int n, int m,  float *rules, float *transitionVector){
    std::clock_t begin = clock();
    for(int i = 0; i < n;  i++){
        if(rules[i * 3 + 1] > -1)
            rules[i * 3 + 1] -= 1;
        transitionVector[i * (m + 1)] = 0;
    }
    std::clock_t end = clock();
    runtime += double(end - begin) / CLOCKS_PER_SEC;
}

void snpReset(int n, int m,  float *lossVector, float *gainVector, float *netGainVector){
    std::clock_t begin = clock();
    for(int i = 0; i < m; i++){
        lossVector[i] = 0;
        gainVector[i] = 0;
        netGainVector[i] = 0;
    }
    std::clock_t end = clock();
    runtime += double(end - begin) / CLOCKS_PER_SEC;
}


void snpSetStates(int n, int m,  float *configVector, float *spikingVector, float* rules,  float* delays,  float* lossVector,
        float* stateVector,  float* transitionVector){

    std::clock_t begin = clock();
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
    std::clock_t end = clock();
    runtime += double(end - begin) / CLOCKS_PER_SEC;

}
