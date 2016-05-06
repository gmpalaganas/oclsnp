/*
 * SNP class containing information
 * of the SNP loaded from the binary file
 */

#include "snp.hpp"

SNP::SNP(){
    regexTree::construct_tree(tree);
}

SNP::~SNP(){
    delete[] initConfig;
    delete[] inputSpikeTrainSteps;
    delete[] inputSpikeTrainSpikes;
    delete[] inputSpikeTrain;
    delete[] ruleIds;
    delete[] ruleRegexs;
    delete[] ruleDelays;
    delete[] neuronLabels;
    destroySynapseMatrix();
    destroyInputSpikeTrain();
}

int SNP::loadSNPFromFile(std::string fileName){
    std::ifstream input(fileName, std::ios::binary);

    if(!input) return -1;

    input.seekg(0, std::ios::end);

    int intBuffer;
    unsigned char charBuffer;

    input.seekg(0, std::ios::beg);
    
    //Read Header
    for(int i = 0; i < 4; i++){
        readChar(input,&charBuffer);
    }

    //Read Sub-header
    readInt(input,&intBuffer);
    ruleCount = reverseEndianess(intBuffer);

    readInt(input,&intBuffer);
    neuronCount = reverseEndianess(intBuffer);

    initConfig = new int[neuronCount];
    for(int i = 0; i < neuronCount; i++){
        readInt(input, &intBuffer);
        initConfig[i] = reverseEndianess(intBuffer);
    }

    readInt(input, &intBuffer);
    inputSpikeTrainLen = intBuffer;
    
    if(inputSpikeTrainLen > 0){
        inputSpikeTrainSteps = new int[inputSpikeTrainLen];
        inputSpikeTrainSpikes = new int[inputSpikeTrainLen];
    }

    for(int i = 0; i < inputSpikeTrainLen; i++){
        readInt(input, &intBuffer);
        inputSpikeTrainSteps[i] = reverseEndianess(intBuffer);
        readInt(input, &intBuffer);
        inputSpikeTrainSpikes[i] = reverseEndianess(intBuffer);
 
        int maxStep = inputSpikeTrainSteps[0];
        inputSpikeTrain = new int[maxStep];

        for(int i = 0; i < maxStep; i++){
            inputSpikeTrain[inputSpikeTrainSteps[i]] = inputSpikeTrainSpikes[i];
            std::cout << i << std::endl;
        }
    }

    ruleIds = new int[ruleCount];
   
    //Read Rules
    for(int i = 0; i < ruleCount; i++){
        readInt(input, &intBuffer);
        ruleIds[i] = reverseEndianess(intBuffer);
    }

    ruleRegexs = new std::string[ruleCount];
    ruleConsumedSpikes = new int[ruleCount];
    ruleProducedSpikes = new int[ruleCount];
    ruleDelays = new int[ruleCount];

    for(int i = 0; i < ruleCount; i++){
        std::string decoded;
        readInt(input, &intBuffer);
        int len = reverseEndianess(intBuffer);
        int nBytes = (int)(ceil((double)len/8));
        if(nBytes == 0){
            input.ignore(1);
            decoded = "NONE";
        }else{
            std::string bits;

            for(int j = 0; j < nBytes; j++){
                readChar(input, &charBuffer);
                bits += std::bitset<8>(charBuffer).to_string();
            }

            int offset = bits.length() - len;
            decoded = tree.decode(bits.substr(offset,len));
        }

        readInt(input, &intBuffer);
        
        ruleRegexs[i] = decoded;
        ruleConsumedSpikes[i] = reverseEndianess(intBuffer);
    }

    for(int i = 0; i < ruleCount; i++){
        readInt(input, &intBuffer);
        ruleProducedSpikes[i] = reverseEndianess(intBuffer);
        readInt(input, &intBuffer);
        ruleDelays[i] = reverseEndianess(intBuffer);
    }

    //Read neuron Labels
    neuronLabels = new std::string[neuronCount];

    for(int i = 0; i < neuronCount; i++){
        readInt(input, &intBuffer);
        int labelLen = reverseEndianess(intBuffer);
        for(int j = 0; j < labelLen; j++)
            neuronLabels[i] += input.get();
    }
    
    //Read synapses as graph adjacency matrix
    int matrixSize = (neuronCount + 1) * (neuronCount + 1);

    int nBytes = (int)(ceil((double)matrixSize / 8));

    std::string linearMatrix;

    for(int i = 0; i < nBytes; i++){
        readChar(input,&charBuffer);
        linearMatrix += std::bitset<8>(charBuffer).to_string();
    }

    int offset = linearMatrix.length() - matrixSize;
    linearMatrix = linearMatrix.substr(offset,matrixSize);

    synapseMatrix = new int*[neuronCount + 1];
    
    for(int i = 0; i < neuronCount + 1; i++)
        synapseMatrix[i] = new int[neuronCount + 1];

    int index = 0;
    for(int i = 0; i < neuronCount + 1; i++){
        for(int j = 0; j < neuronCount + 1; j++){
            synapseMatrix[i][j] = (linearMatrix[index] == '1')?1:0;
            index++;
        }
    }

    input.close();
    
    return 0;
}

void SNP::destroySynapseMatrix(){
    for(int i = 0; i < neuronCount + 1; i++)
        delete[] synapseMatrix[i];

    delete[] synapseMatrix;
}

void SNP::destroyInputSpikeTrain(){
    if(inputSpikeTrainLen > 0){
        delete[] inputSpikeTrainSteps;
        delete[] inputSpikeTrainSpikes;
    }
}

void SNP::printSNPContents(){

    std::cout << "INPUT SNP DETAILS" << std::endl;
    std::cout << "Neuron Count: " << neuronCount << std::endl;
    std::cout << "Rule Count: " << ruleCount << std::endl;
    
    std::cout << "Init Config: [";
    for(int i = 0; i < neuronCount; i++){
        std::cout << initConfig[i];
        if(i < neuronCount - 1)
            std::cout << ", ";
        else
            std::cout << "]\n";
    }
    
    if(inputSpikeTrainLen > 0){
        std::cout << "Input Spike Train: [";
        for(int i = 0; i < inputSpikeTrainSteps[inputSpikeTrainLen - 1]; i++){
            std::cout << inputSpikeTrain[i];
            if(i < inputSpikeTrainSteps[inputSpikeTrainLen - 1] - 1)
                std::cout << ", ";
            else
                std::cout << "]\n";
        }
    }

    std::cout << "Rule Neuron IDs: [";
    for(int i = 0; i < ruleCount; i++){
        std::cout << ruleIds[i];
        if(i < ruleCount - 1)
            std::cout << ", ";
        else
            std::cout << "]\n";
    }

    std::cout << "Rules [";
    for(int i = 0; i < ruleCount; i++){
        std::stringstream ss;
        std::string regex = (ruleRegexs[i].length() > 0)?ruleRegexs[i] + "/":"";
        ss << regex; 
        
        if(ruleConsumedSpikes[i] > 1)
            ss << "a^" << ruleConsumedSpikes[i];
        else
            ss << "a";

        ss << "-->";

        if(ruleProducedSpikes[i] > 0)
            ss << "a^" << ruleProducedSpikes[i]; 
        else
            ss << "#";
       
        if(ruleDelays[i] > 0)
            ss << ";" << ruleDelays[i];

        std::cout << ss.str();

        if(i < ruleCount - 1)
            std::cout << ", ";
        else
            std::cout << "]\n";
    }

    std::cout << "Neuron Labels: [";
    for(int i = 0; i < neuronCount; i++){
        std::cout << neuronLabels[i];
        if(i < neuronCount - 1)
            std::cout << ", ";
        else
            std::cout << "]\n";
    }

}

int SNP::getRuleRegexCode(int ruleId){
    int retVal = -1;

    std::string r = ruleRegexs[ruleId];

    if(r == "a+")
        retVal = 0;
    else if(r.length() > 0){
        re2::RE2 expr("\\D");
        re2::RE2::GlobalReplace(&r, expr, "");
        retVal = boost::lexical_cast<int>(r);
    }else{
        retVal = ruleConsumedSpikes[ruleId];
    }

    return retVal;

}

std::string SNP::getRuleRegex(int ruleId){
    std::string regex = (ruleRegexs[ruleId].length() > 0)?
        ruleRegexs[ruleId]:
        "a^" + boost::lexical_cast<std::string>(ruleConsumedSpikes[ruleId]);
    return regex;
}

std::string SNP::getRule(int ruleId){
    std::stringstream ss;
    std::string regex = (ruleRegexs[ruleId].length() > 0)?ruleRegexs[ruleId] + "/":"";
    ss << regex; 

    if(ruleConsumedSpikes[ruleId] > 1)
        ss << "a^" << ruleConsumedSpikes[ruleId];
    else
        ss << "a";

    ss << "-->";

    if(ruleProducedSpikes[ruleId] > 0)
        ss << "a^" << ruleProducedSpikes[ruleId]; 
    else
        ss << "#";

    if(ruleDelays[ruleId] > 0)
        ss << ";" << ruleDelays[ruleId];

    return ss.str();
}
