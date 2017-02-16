#include "main.hpp"

int main(int argc, char **argv){
    
    std::ofstream outputFile;
    std::vector<programFlags::ProgramFlags> flags;
    std::stringstream outputStream;

    if(argc == 3){
        outputFile = std::ofstream(argv[2]);
    }else if(argc < 2){
        std::cout << "Usage: ./oclsnp <input_binary_file> [output_file]" << std::endl;
        std::cout << "I.e ./oclsnp ../inputs/2input_sort.bin outputs/2input_sort_out.txt" << std::endl;
        exit(1);
    }else{
        for(int i = 3; i < argc; i++)
            flags.push_back(checkFlag(argv[i]));
    }

    SNPEmulator emulator(argv[1]);
    
    emulator.execute(&outputStream);
   
    double vm;
    double rss;
    getMemUsage(vm, rss);

    outputStream << "Execution time: " << emulator.getRuntime() << " ns" << std::endl;
    outputStream << "Memory Usage: " << rss << " kb" << std::endl;
    
    if(std::find(flags.begin(), flags.end(), programFlags::ProgramFlags::SILENT) == flags.end()){
        std::cout << outputStream.str();
    }else{
        std::cout << "Execution time: " << emulator.getRuntime() << " ns" << std::endl;
        std::cout << "Memory Usage: " << rss << " kb" << std::endl;
    }

    if(outputFile){
        outputFile << outputStream.str();
    }

    return 0;

}

programFlags::ProgramFlags checkFlag(std::string flag){
    programFlags::ProgramFlags progFlag;
    if(flag == "--silent")
        progFlag = programFlags::ProgramFlags::SILENT;
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
