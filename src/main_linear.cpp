#include "main_linear.hpp"

std::chrono::microseconds runtime = std::chrono::microseconds(0);

// To be configured to accept input file name
// and number of steps
int main(int argc, char **argv){

    std::vector<programFlags::ProgramFlags> flags;
    std::stringstream outputStream;

    if(argc < 2){
        std::cout << "Usage: ./oclsnp <input_binary_file> [output_file]" << std::endl;
        std::cout << "I.e ./oclsnp ../inputs/2input_sort.bin outputs/2input_sort_out.txt" << std::endl;
        exit(1);
    } else{
        for(int i = 2; i < argc; i++)
            if(argv[i] == "-o"){
                outputFile = std::ofstream(argv[i + 1]);
                i++;
            }else
                flags.push_back(checkFlag(argv[i]));
    }

    int err;

    if(std::find(flags.begin(), flags.end(), programFlags::ProgramFlags::TEXT) != flags.end())
        err = snp.loadSNPFromTextFile(argv[1]);
    else 
        err = snp.loadSNPFromFile(argv[1]);

    checkError(err, "Invalid Binary file", __FUNCTION__);

    if(std::find(flags.begin(), flags.end(), programFlags::ProgramFlags::PRINT_SNP) != flags.end())
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
    utils::copyIntArrIntoFloatArr(snp.initConfig, configVector, m); 

    //Copy snp rule delays into delays vector
    utils::copyIntArrIntoFloatArr(snp.ruleDelays, delays, n);

    std::fill_n(stateVector, m, 1);

    for(int i = 0; i < n; i++){
        int index = i * 3;
        rules[index] = snp.ruleIds[i];
        rules[index + 1] = -1;
        rules[index + 2] = -(snp.ruleConsumedSpikes[i]);
    }

    for(int i = 0; i < n; i++){
        for(int j = 1; j < m + 1; j++){
            if(snp.synapseMatrix[snp.ruleIds[i]][j] == 1)
                transitionVector[i * (m + 1) + j] = snp.ruleProducedSpikes[i];
        }
    }

    for(int i = 0; i < n; i++){
        regexs[i] = utils::expandRegex(snp.getRuleRegex(i));
    }

    
    int step = 1;

    //Simulation Proper
    do{

        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        matchRulesRegex(regexs, rules, configVector, spikingVector, n);
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        runtime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
        if(step == 1)
            std::cout << runtime.count() << std::endl;

        if(!areRulesApplicable(spikingVector,n))
                break;
    
        outputStream << "************************************" << std::endl;
        outputStream << "At step " << step << ":" << std::endl;

        begin = std::chrono::high_resolution_clock::now();
        snpSetStates(n, m, configVector, spikingVector, rules, delays, lossVector, stateVector, transitionVector);
        vectorSelectiveAdd(transitionVector, gainVector, n, m);
        snpComputeNetGain(n, m, stateVector, lossVector, gainVector, netGainVector);
        vectorAdd(netGainVector,configVector,configVector,m);
        snpPostCompute(n, m, rules, transitionVector);
        snpReset(n, m, lossVector, gainVector, netGainVector);
        end = std::chrono::high_resolution_clock::now();

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
 
    double vm;
    double rss;
    getMemUsage(vm, rss);

    outputStream << "Execution time: " << float(runtime.count()) << " ms" << std::endl;
    outputStream << "Memory Usage: " << rss << " kb" << std::endl;

    if(std::find(flags.begin(), flags.end(), programFlags::ProgramFlags::SILENT) == flags.end())
        std::cout << outputStream.str();
    std::cout << "Execution time: " << float(runtime.count()) << " ms" << std::endl;
    std::cout << "Memory Usage: " << rss << " kb" << std::endl;

    if(outputFile){
        outputFile << outputStream.str();
    }

    if(std::find(flags.begin(), flags.end(), programFlags::ProgramFlags::SILENT) == flags.end())
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
    for(int i = 0; i < vectorSize; i++)
        outputVector[i] = vectorA[i] + vectorB[i];
}

void vectorElemMult(float *vectorA, float *vectorB, float *outputVector, int vectorSize){
    for(int i = 0; i < vectorSize; i++)
        outputVector[i] = vectorA[i] * vectorB[i];
}

void vectorSelectiveAdd(float *vectorA, float *outputVector, int rows, int cols){
    for(int i = 0; i < cols; i++){
        for(int j = 0; j < rows; j++){
            if(vectorA[j * (cols + 1)] == 1)
                outputVector[i] += vectorA[j * (cols +1) + (i+1)];
        }
    }
}

void snpComputeNetGain(int n, int m, float *stateVector, float *lossVector, float *gainVector, float *netGainVector){
    for(int i = 0; i < m; i++)
        netGainVector[i] = gainVector[i] * stateVector[i] + lossVector[i]; 
}

void snpDetermineRules(int n, int m,  float *configVector, float *spikingVector, float *rules, float *lhs){
    for(int i = 0; i < n; i++){
        if((configVector[ (int)rules[i * 3] - 1 ] == lhs[i] || lhs[i] == 0) &&
            configVector[(int)rules[i * 3] - 1] + rules[i * 3 + 2] >= 0){

            spikingVector[i] = 1; 

         }else{
            spikingVector[i] = 0;
        }
    }
}

void snpPostCompute(int n, int m,  float *rules, float *transitionVector){
    for(int i = 0; i < n;  i++){
        if(rules[i * 3 + 1] > -1)
            rules[i * 3 + 1] -= 1;
        transitionVector[i * (m + 1)] = 0;
    }
}

void snpReset(int n, int m,  float *lossVector, float *gainVector, float *netGainVector){
    for(int i = 0; i < m; i++){
        lossVector[i] = 0;
        gainVector[i] = 0;
        netGainVector[i] = 0;
    }
}


void snpSetStates(int n, int m,  float *configVector, float *spikingVector, float* rules,  float* delays,  
        float* lossVector, float* stateVector,  float* transitionVector){

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

}

void matchRuleRegex(std::string regex, std::string str, float* isMatch){
    re2::StringPiece input(str);
    *(isMatch) = re2::RE2::FullMatch(input,regex);
}

void matchRulesRegex(std::string *regexVector, float* rules, float* configVector, float* spikingVector, int vectorSize){

    std::thread threads[vectorSize];

    for(int i = 0; i < vectorSize; i++){
        std::string expandedRegex = regexVector[i];
        std::string spikeString = utils::expandRegex("a^"+ boost::lexical_cast<std::string>(configVector[(int)rules[3 * i] - 1]));
        float *ruleMarkPtr = spikingVector + i;
        threads[i] = std::thread(matchRuleRegex, expandedRegex, spikeString, ruleMarkPtr);
    }
    
    for(int i = 0; i < vectorSize; i++){
        threads[i].join();
    }
}

programFlags::ProgramFlags checkFlag(std::string flag){
    programFlags::ProgramFlags progFlag;
    if(flag == "--silent")
        progFlag = programFlags::ProgramFlags::SILENT;
    else if(flag == "--txt")
        progFlag = programFlags::ProgramFlags::TEXT;
    else if(flag == "--print-snp")
        progFlag = programFlags::ProgramFlags::PRINT_SNP;
    return progFlag;
}

void getMemUsage(double& vmUsage, double& residentSet){

    vmUsage     = 0.0;
    residentSet = 0.0;

    // 'file' stat seems to give the most reliable results
    //
    std::ifstream stat_stream("/proc/self/stat",std::ios_base::in);

    // dummy vars for leading entries in stat that we don't care about
    //
    std::string pid, comm, state, ppid, pgrp, session, tty_nr;
    std::string tpgid, flags, minflt, cminflt, majflt, cmajflt;
    std::string utime, stime, cutime, cstime, priority, nice;
    std::string O, itrealvalue, starttime;

    // the two fields we want
    //
    unsigned long vsize;
    long rss;

    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
        >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
        >> utime >> stime >> cutime >> cstime >> priority >> nice
        >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

    stat_stream.close();

    long pageSizeKb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    vmUsage     = vsize / 1024.0;
    residentSet = rss * pageSizeKb;
}
