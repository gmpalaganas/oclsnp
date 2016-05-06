#include "regex.hpp"

// Expands a powerset regex into a regular regex
// i.e. a^5 -> aaaaa
std::string expandRegex(const std::string powerRegex){

    std::string capture;
    std::string newString = powerRegex;

    re2::RE2 re("a\\s*\\^\\s*(\\d+)");
    re2::StringPiece input(powerRegex);

    while(re2::RE2::FindAndConsume(&input, re, &capture)){
        int count = boost::lexical_cast<int>(capture);
        std::string expandedRegex(count, 'a');
        re2::RE2::GlobalReplace(&newString, "a\\s*\\^\\s*" + capture, expandedRegex);
    }

    return newString.length() > 0?newString:" ";

}
