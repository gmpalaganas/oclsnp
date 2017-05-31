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
#include <thread>
#include <chrono>

#include <re2/re2.h>
#include <boost/lexical_cast.hpp>

#include <snp.hpp>
#include <array.hpp>
#include <regex.hpp>

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

void matchRuleRegex(std::string regex, std::string input, float* isMatch);
void matchRulesRegex(std::string *regexVector, float* rules, float* configVector, float* spikingVector, int vectorSize);

namespace programFlags{
    enum ProgramFlags { SILENT, TEXT, PRINT_SNP };
}

programFlags::ProgramFlags checkFlag(std::string flag);

void getMemUsage(double& vmUsage, double& residentSet);

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

std::string *regexs;

std::ofstream outputFile;
