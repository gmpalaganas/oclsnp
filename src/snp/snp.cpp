/*
 * SNP class containing information
 * of the SNP loaded from the binary file
 */

#include "snp.hpp"

using namespace std;

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

int SNP::loadSNPFromFile(ifstream *input){

    if(!input) return -1;

    input->seekg(0, ios::end);

    int intBuffer;
    unsigned char charBuffer;

    input->seekg(0, ios::beg);

    //Read Header
    for(int i = 0; i < 4; i++){
        utils::readChar(input,&charBuffer);
    }

    //Read Sub-header
    utils::readInt(input,&intBuffer);
    ruleCount = utils::reverseEndianess(intBuffer);

    utils::readInt(input,&intBuffer);
    neuronCount = utils::reverseEndianess(intBuffer);

    initConfig = new int[neuronCount];
    for(int i = 0; i < neuronCount; i++){
        utils::readInt(input, &intBuffer);
        initConfig[i] = utils::reverseEndianess(intBuffer);
    }

    utils::readInt(input, &intBuffer);
    inputSpikeTrainLen = intBuffer;

    if(inputSpikeTrainLen > 0){
        inputSpikeTrainSteps = new int[inputSpikeTrainLen];
        inputSpikeTrainSpikes = new int[inputSpikeTrainLen];
    }else{
        inputSpikeTrainSteps = NULL;
        inputSpikeTrainSpikes = NULL;
        inputSpikeTrain = NULL;
    }

    for(int i = 0; i < inputSpikeTrainLen; i++){
        utils::readInt(input, &intBuffer);
        inputSpikeTrainSteps[i] = utils::reverseEndianess(intBuffer);
        utils::readInt(input, &intBuffer);
        inputSpikeTrainSpikes[i] = utils::reverseEndianess(intBuffer);

        int maxStep = inputSpikeTrainSteps[0];
        inputSpikeTrain = new int[maxStep];

        for(int i = 0; i < maxStep; i++){
            inputSpikeTrain[inputSpikeTrainSteps[i]] = inputSpikeTrainSpikes[i];
            cout << i << endl;
        }
    }

    ruleIds = new int[ruleCount];

    //Read Rules
    for(int i = 0; i < ruleCount; i++){
        utils::readInt(input, &intBuffer);
        ruleIds[i] = utils::reverseEndianess(intBuffer);
    }

    ruleRegexs = new string[ruleCount];
    ruleConsumedSpikes = new int[ruleCount];
    ruleProducedSpikes = new int[ruleCount];
    ruleDelays = new int[ruleCount];

    for(int i = 0; i < ruleCount; i++){
        string decoded;
        utils::readInt(input, &intBuffer);
        int len = utils::reverseEndianess(intBuffer);
        int nBytes = (int)(ceil((double)len/8));
        if(nBytes == 0){
            input->ignore(1);
            decoded = "NONE";
        }else{
            string bits;

            for(int j = 0; j < nBytes; j++){
                utils::readChar(input, &charBuffer);
                bits += bitset<8>(charBuffer).to_string();
            }

            int offset = bits.length() - len;
            decoded = tree.decode(bits.substr(offset,len));
        }

        utils::readInt(input, &intBuffer);

        ruleRegexs[i] = decoded;
        ruleConsumedSpikes[i] = utils::reverseEndianess(intBuffer);
    }

    for(int i = 0; i < ruleCount; i++){
        utils::readInt(input, &intBuffer);
        ruleProducedSpikes[i] = utils::reverseEndianess(intBuffer);
        utils::readInt(input, &intBuffer);
        ruleDelays[i] = utils::reverseEndianess(intBuffer);
    }

    //Read neuron Labels
    neuronLabels = new string[neuronCount];

    for(int i = 0; i < neuronCount; i++){
        utils::readInt(input, &intBuffer);
        int labelLen = utils::reverseEndianess(intBuffer);
        for(int j = 0; j < labelLen; j++)
            neuronLabels[i] += input->get();
    }

    //Read synapses as graph adjacency matrix
    int matrixSize = (neuronCount + 1) * (neuronCount + 1);

    int nBytes = (int)(ceil((double)matrixSize / 8));

    string linearMatrix;

    for(int i = 0; i < nBytes; i++){
        utils::readChar(input,&charBuffer);
        linearMatrix += bitset<8>(charBuffer).to_string();
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

    input->close();

    return 0;

}

int SNP::loadSNPFromTextFile(ifstream *input){

    inputSpikeTrainLen = 0;
    inputSpikeTrainSteps = NULL;
    inputSpikeTrainSpikes = NULL;
    inputSpikeTrain = NULL;

    string line;
    stringstream ss(line);

    getline(*input,line);
    istringstream intss(line);
    intss >> ruleCount;

    getline(*input,line);
    intss = istringstream(line);
    intss >> neuronCount;

    initConfig = new int[neuronCount];
    getline(*input,line);
    intss = istringstream(line);

    for(int i = 0; i < neuronCount; i++)
        intss >> initConfig[i];

    ruleIds = new int[ruleCount];
    ruleRegexs = new string[ruleCount];
    ruleConsumedSpikes = new int[ruleCount];
    ruleProducedSpikes = new int[ruleCount];
    ruleDelays = new int[ruleCount];

    for(int i = 0; i < ruleCount; i++){
        getline(*input,line);
        ss = stringstream(line);
        string buffer;

        ss >> buffer;
        ruleIds[i] = atoi(buffer.c_str());
        ss >> ruleRegexs[i];
        ss >> buffer;
        ruleConsumedSpikes[i] = atoi(buffer.c_str());
        ss >> buffer;
        ruleProducedSpikes[i] = atoi(buffer.c_str());
        ss >> buffer;
        ruleDelays[i] = atoi(buffer.c_str());

    }

    neuronLabels = new string[neuronCount];

    for(int i = 0; i < neuronCount; i++)
        getline(*input, neuronLabels[i]);


    synapseMatrix = new int*[neuronCount + 1];
    
    for(int i = 0; i < neuronCount + 1; i++){
        synapseMatrix[i] = new int[neuronCount + 1];
        fill_n(synapseMatrix[i], neuronCount + 1, 0);
        getline(*input,line);
        intss = istringstream(line);

        int out_index;
        while(intss >> out_index){
            if(out_index == -1)
                break;
            synapseMatrix[i][out_index] = 1;
        }
    }

    input->close();

    return 0;
}

int SNP::loadSNPFromFile(string fileName){
    ifstream input(fileName, ios::binary);
    return loadSNPFromFile(&input);
}


int SNP::loadSNPFromTextFile(string fileName){
    ifstream input(fileName, ios::binary);
    return loadSNPFromTextFile(&input);
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

    cout << "INPUT SNP DETAILS" << endl;
    cout << "Neuron Count: " << neuronCount << endl;
    cout << "Rule Count: " << ruleCount << endl;
    
    cout << "Init Config: [";
    for(int i = 0; i < neuronCount; i++){
        cout << initConfig[i];
        if(i < neuronCount - 1)
            cout << ", ";
        else
            cout << "]\n";
    }
    
    if(inputSpikeTrainLen > 0){
        cout << "Input Spike Train: [";
        for(int i = 0; i < inputSpikeTrainSteps[inputSpikeTrainLen - 1]; i++){
            cout << inputSpikeTrain[i];
            if(i < inputSpikeTrainSteps[inputSpikeTrainLen - 1] - 1)
                cout << ", ";
            else
                cout << "]\n";
        }
    }

    cout << "Rule Neuron IDs: [";
    for(int i = 0; i < ruleCount; i++){
        cout << ruleIds[i];
        if(i < ruleCount - 1)
            cout << ", ";
        else
            cout << "]\n";
    }

    cout << "Rules [";
    for(int i = 0; i < ruleCount; i++){
        stringstream ss;
        string regex = (ruleRegexs[i].length() > 0)?ruleRegexs[i] + "/":"";
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

        cout << ss.str();

        if(i < ruleCount - 1)
            cout << ", ";
        else
            cout << "]\n";
    }

    cout << "Neuron Labels: [";
    for(int i = 0; i < neuronCount; i++){
        cout << neuronLabels[i];
        if(i < neuronCount - 1)
            cout << ", ";
        else
            cout << "]\n";
    }

    cout << "Synapse Matrix:" << endl;
    for(int i = 0; i < neuronCount + 1; i++){
        for(int j = 0; j < neuronCount + 1; j++)
            cout << synapseMatrix[i][j] << ' ';
        cout << endl;
    }

}

int SNP::getRuleRegexCode(int ruleId){
    int retVal = -1;

    string r = ruleRegexs[ruleId];

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

void SNP::getRuleRegexRepr(int ruleId, regex_repr *repr){
    string regex = getRuleRegex(ruleId);
    regex = utils::expandRegex(regex);

    float k = 0;
    float j = 0;
    
    for(size_t i = 0; i < regex.length(); i++){
        switch(regex[i]){
            case 'a':
                k++;
                break;
            case '(':
                i++;
                while(regex[i] != ')'){
                    j++;
                    i++;
                }
                break;
            case '+':
                if(k == 1.0f){
                    k = -1.0f;
                    j = 1.0f;
                }else
                    k = -((2 * k) + 1);
                break;
            case '*':
                if(k == 1.0f){
                    k = 0.0f;
                    j = 1.0f;
                }else
                    k = -(2 * k);
                break;
        }
    }

    repr->k = k;
    repr->j = j;

}

string SNP::getRuleRegex(int ruleId){
    string regex = (ruleRegexs[ruleId].length() > 0)?
        ruleRegexs[ruleId]:
        "a^" + boost::lexical_cast<string>(ruleConsumedSpikes[ruleId]);
    return regex;
}

string SNP::getRule(int ruleId){
    stringstream ss;
    string regex = (ruleRegexs[ruleId].length() > 0)?ruleRegexs[ruleId] + "/":"";
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
