#include "verifier.h"

Verifier::Verifier() {};
bool Verifier::verifyCommandCalled(std::string command, int count) {
    return ( verifyCommandCalled(command) == count);
}
int Verifier::verifyCommandCalled(std::string command) {
    
    std::regex words_regex("("+ command +")+");
    auto words_begin = std::sregex_iterator(_commandsCalledBuffer.begin(), _commandsCalledBuffer.end(), words_regex);
    int totalFound = std::distance(words_begin, std::sregex_iterator()) ;

    return totalFound;
}

