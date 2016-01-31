#include "utils.hpp"

//Reverse the endianess of an integer
int snp_utils::reverseEndianess(int smallEndian){
    int bigEndian;

    unsigned char *bigEndianBytes = (unsigned char *)&bigEndian;
    unsigned char *smallEndianBytes = (unsigned char *)&smallEndian;

    for(int i = 0; i < sizeof(int); i++)
        bigEndianBytes[i] = smallEndianBytes[sizeof(int) - i - 1];

    return bigEndian;
}

//Reverse the endianess of a character
unsigned char snp_utils::reverseEndianess(unsigned char smallEndian){
    unsigned char bigEndian;

    unsigned char *bigEndianBytes = (unsigned char *)&bigEndian;
    unsigned char *smallEndianBytes = (unsigned char *)&smallEndian;

    for(int i = 0; i < sizeof(int); i++)
        bigEndianBytes[i] = smallEndianBytes[sizeof(int) - i - 1];

    return bigEndian;
}

void snp_utils::readInt(std::ifstream &file, int *intBuffer){
    file.read(reinterpret_cast<char *>(intBuffer),sizeof(int));
}

void snp_utils::readChar(std::ifstream &file, unsigned char *charBuffer){
    file.read(reinterpret_cast<char *>(charBuffer),sizeof(char));
}
