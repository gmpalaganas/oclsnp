
#include "regex_tree.hpp" 
#include "utils.hpp"

#include <string>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

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

        void loadSNPFromFile(std::string fileName);
        void printSNPContents();

        int getRuleRegexCode(int ruleId);
        std::string getRule(int ruleId);

    private:
        void destroySynapseMatrix();
        void destroyInputSpikeTrain();
        RegexTree tree;

};

