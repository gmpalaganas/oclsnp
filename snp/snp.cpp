#include "regex_tree.cpp" 
#include "utils.cpp"

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

void SNP::loadSNPFromFile(std::string fileName){
    std::ifstream input(fileName, std::ios::binary);

    input.seekg(0, std::ios::end);
    int length = input.tellg();

    short shortBuffer;
    int intBuffer;
    unsigned char charBuffer;

    input.seekg(0, std::ios::beg);
    
    //Read Header
    for(int i = 0; i < 4; i++){
        snp_utils::readChar(input,&charBuffer);
    }

    //Read Sub-header
    snp_utils::readInt(input,&intBuffer);
    ruleCount = snp_utils::smallToBigEndian(intBuffer);

    snp_utils::readInt(input,&intBuffer);
    neuronCount = snp_utils::smallToBigEndian(intBuffer);

    initConfig = new int[neuronCount];
    for(int i = 0; i < neuronCount; i++){
        snp_utils::readInt(input, &intBuffer);
        initConfig[i] = snp_utils::smallToBigEndian(intBuffer);
    }

    snp_utils::readInt(input, &intBuffer);
    inputSpikeTrainLen = intBuffer;
    
    if(inputSpikeTrainLen > 0){
        inputSpikeTrainSteps = new int[inputSpikeTrainLen];
        inputSpikeTrainSpikes = new int[inputSpikeTrainLen];
    }

    for(int i = 0; i < inputSpikeTrainLen; i++){
        snp_utils::readInt(input, &intBuffer);
        inputSpikeTrainSteps[i] = snp_utils::smallToBigEndian(intBuffer);
        snp_utils::readInt(input, &intBuffer);
        inputSpikeTrainSpikes[i] = snp_utils::smallToBigEndian(intBuffer);
 
        int maxStep = inputSpikeTrainSteps[0];
        inputSpikeTrain = new int[maxStep];

        for(int i = 0; i < maxStep; i++){
            inputSpikeTrain[inputSpikeTrainSteps[i]] = inputSpikeTrainSpikes[i];
            std::cout << i << std::endl;
        }
    }



    ruleIds = new int[ruleCount];
    
    for(int i = 0; i < ruleCount; i++){
        snp_utils::readInt(input, &intBuffer);
        ruleIds[i] = snp_utils::smallToBigEndian(intBuffer);
    }

    ruleRegexs = new std::string[ruleCount];
    ruleConsumedSpikes = new int[ruleCount];
    ruleProducedSpikes = new int[ruleCount];
    ruleDelays = new int[ruleCount];

    for(int i = 0; i < ruleCount; i++){
        std::string decoded;
        snp_utils::readInt(input, &intBuffer);
        int len = snp_utils::smallToBigEndian(intBuffer);
        int nBytes = (int)(ceil((double)len/8));
        if(nBytes == 0){
            input.ignore(1);
            decoded = "NONE";
        }else{
            std::string bits;

            for(int j = 0; j < nBytes; j++){
                snp_utils::readChar(input, &charBuffer);
                bits += std::bitset<8>(charBuffer).to_string();
            }

            int offset = bits.length() - len;
            decoded = tree.decode(bits.substr(offset,len));
        }

        snp_utils::readInt(input, &intBuffer);
        
        ruleRegexs[i] = decoded;
        ruleConsumedSpikes[i] = snp_utils::smallToBigEndian(intBuffer);
    }

    for(int i = 0; i < ruleCount; i++){
        snp_utils::readInt(input, &intBuffer);
        ruleProducedSpikes[i] = snp_utils::smallToBigEndian(intBuffer);
        snp_utils::readInt(input, &intBuffer);
        ruleDelays[i] = snp_utils::smallToBigEndian(intBuffer);
    }

    neuronLabels = new std::string[neuronCount];

    for(int i = 0; i < neuronCount; i++){
        snp_utils::readInt(input, &intBuffer);
        int labelLen = snp_utils::smallToBigEndian(intBuffer);
        for(int j = 0; j < labelLen; j++)
            neuronLabels[i] += input.get();
    }

    int matrixSize = (neuronCount + 1) * (neuronCount + 1);

    int nBytes = (int)(ceil((double)matrixSize / 8));

    std::string linearMatrix;

    for(int i = 0; i < nBytes; i++){
        snp_utils::readChar(input,&charBuffer);
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

    std::cout << "Synapse Matrix" << std::endl;
    for(int i = 0; i < neuronCount + 1; i++){
        for(int j = 0; j < neuronCount + 1; j++){
            std::cout << synapseMatrix[i][j];
            if(j < neuronCount)
                std::cout << ", ";
            else
                std::cout << "\n";
        }
    }

}

int SNP::getRuleRegexCode(int ruleId){
    int retVal = -1;

    std::string r = ruleRegexs[ruleId];

    if(r == "a+")
        retVal = 0;
    else if(r.length() > 0){
        boost::regex expr("\\D");
        r = boost::regex_replace(r, expr, "");
        retVal = std::stoi(r);
    }else{
        retVal = ruleConsumedSpikes[ruleId];
    }

    return retVal;

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
