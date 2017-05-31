#include "main.hpp"


int main(int argc, char **argv){
    
    std::ofstream outputFile;
    std::vector<programFlags::ProgramFlags> flags;
    std::stringstream outputStream;

    if(argc < 2){
        std::cout << "Usage: ./oclsnp <input_binary_file> -o [output_file]" << std::endl;
        std::cout << "I.e ./oclsnp ../inputs/2input_sort.bin outputs/2input_sort_out.txt" << std::endl;
        exit(1);
    }else{
        for(int i = 2; i < argc; i++)
            if(argv[i] == "-o"){
                outputFile = std::ofstream(argv[i + 1]);
                i++;
            }else
                flags.push_back(checkFlag(argv[i]));
    }

    bool isBinary = true;
    if(std::find(flags.begin(), flags.end(), programFlags::ProgramFlags::TEXT) != flags.end())
        isBinary = false;
    

    SNPEmulator emulator(argv[1],isBinary);

    if(std::find(flags.begin(), flags.end(), programFlags::ProgramFlags::PRINT_SNP) != flags.end())
        emulator.snp.printSNPContents();
    
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
    else if(flag == "--txt")
        progFlag = programFlags::ProgramFlags::TEXT;
    else if(flag == "--print-snp")
        progFlag = programFlags::ProgramFlags::PRINT_SNP;
    return progFlag;
}
