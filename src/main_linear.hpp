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

#include "snp/snp.hpp"
#include "utils/array.hpp"

#define EXECUTE_FAILURE -1

inline void checkError(int err, std::string msg, std::string fncName);
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

std::ofstream outputFile;
