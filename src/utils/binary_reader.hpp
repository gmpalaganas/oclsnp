#include <fstream>

namespace utils{
    int reverseEndianess(int smallEndian);
    unsigned char reverseEndianess(unsigned char smallEndian);
    void readInt(std::ifstream *file, int *intBuffer);
    void readChar(std::ifstream *file, unsigned char *charBuffer);
};
