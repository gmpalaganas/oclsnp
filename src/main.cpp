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

    outputStream << "Execution time: " << emulator.getRuntime() << " ns" << std::endl;
    
    if(std::find(flags.begin(), flags.end(), programFlags::ProgramFlags::SILENT) == flags.end()){
        std::cout << outputStream.str();
    }else{
        std::cout << "Execution time: " << emulator.getRuntime() << " ns" << std::endl;
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
