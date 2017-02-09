#include "regex.hpp"

using namespace std;

// Expands a powerset regex into a regular regex
// i.e. a^5 -> aaaaa
string utils::expandRegex(const string powerRegex){

    string capture;
    string newString = powerRegex;

    re2::RE2 re("a\\s*\\^?\\s*(\\d+)");
    re2::StringPiece input(powerRegex);

    while(re2::RE2::FindAndConsume(&input, re, &capture)){
        int count = boost::lexical_cast<int>(capture);
        string expandedRegex(count, 'a');
        re2::RE2::GlobalReplace(&newString, "a\\s*\\^?\\s*" + capture, expandedRegex);
    }

    return newString.length() > 0?newString:" ";

}
