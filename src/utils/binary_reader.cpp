#include "binary_reader.hpp"

using namespace std;

//Reverse the endianess of an integer
int utils::reverseEndianess(int smallEndian){
    int bigEndian;

    unsigned char *bigEndianBytes = (unsigned char *)&bigEndian;
    unsigned char *smallEndianBytes = (unsigned char *)&smallEndian;

    for(size_t i = 0; i < sizeof(int); i++)
        bigEndianBytes[i] = smallEndianBytes[sizeof(int) - i - 1];

    return bigEndian;
}

//Reverse the endianess of a character
unsigned char utils::reverseEndianess(unsigned char smallEndian){
    unsigned char bigEndian;

    unsigned char *bigEndianBytes = (unsigned char *)&bigEndian;
    unsigned char *smallEndianBytes = (unsigned char *)&smallEndian;

    for(size_t i = 0; i < sizeof(int); i++)
        bigEndianBytes[i] = smallEndianBytes[sizeof(int) - i - 1];

    return bigEndian;
}

void utils::readInt(ifstream *file, int *intBuffer){
    file->read(reinterpret_cast<char *>(intBuffer),sizeof(int));
}

void utils::readChar(ifstream *file, unsigned char *charBuffer){
    file->read(reinterpret_cast<char *>(charBuffer),sizeof(char));
}
