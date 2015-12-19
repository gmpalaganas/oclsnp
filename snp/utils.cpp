namespace snp_utils{
    int smallToBigEndian(int smallEndian){
        int bigEndian;

        unsigned char *bigEndianBytes = (unsigned char *)&bigEndian;
        unsigned char *smallEndianBytes = (unsigned char *)&smallEndian;

        for(int i = 0; i < sizeof(int); i++)
            bigEndianBytes[i] = smallEndianBytes[sizeof(int) - i - 1];

        return bigEndian;
    }
    unsigned char smallToBigEndian(unsigned char smallEndian){
        unsigned char bigEndian;

        unsigned char *bigEndianBytes = (unsigned char *)&bigEndian;
        unsigned char *smallEndianBytes = (unsigned char *)&smallEndian;

        for(int i = 0; i < sizeof(int); i++)
            bigEndianBytes[i] = smallEndianBytes[sizeof(int) - i - 1];

        return bigEndian;
    }

    void readInt(std::ifstream &file, int *intBuffer){
        file.read(reinterpret_cast<char *>(intBuffer),sizeof(int));
    }

void readChar(std::ifstream &file, unsigned char *charBuffer){
        file.read(reinterpret_cast<char *>(charBuffer),sizeof(char));
    }
}
