
#include <regex_tree.hpp> 
#include <binary_reader.hpp>
#include <regex.hpp>

#include <string>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <re2/re2.h>
#include <boost/lexical_cast.hpp>


typedef struct {
    float k;
    float j;
} regex_repr;


class SNP{
    public:
        SNP();
        ~SNP();

        int neuronCount;
        int ruleCount;
        int *initConfig;
        int inputSpikeTrainLen;
        int *inputSpikeTrainSteps;
        int *inputSpikeTrainSpikes;
        int *inputSpikeTrain;
        int *ruleIds;
        std::string  *ruleRegexs;
        int *ruleConsumedSpikes;
        int *ruleProducedSpikes;
        int *ruleDelays;
        std::string *neuronLabels;
        int **synapseMatrix = NULL;

        int loadSNPFromFile(std::ifstream *input);
        int loadSNPFromFile(std::string fileName);
        void printSNPContents();

        int getRuleRegexCode(int ruleId);
        void getRuleRegexRepr(int ruleId, regex_repr *repr);
        std::string getRuleRegex(int ruleId);
        std::string getRule(int ruleId);

    private:
        void destroySynapseMatrix();
        void destroyInputSpikeTrain();
        RegexTree tree;

};

