#ifndef VERIFIER_H
#define VERIFIER_H

#include "pch.h"

class Verifier {
    public:
        virtual ~Verifier(){}
        Verifier();
        bool verifyCommandCalled(std::string command, int count);
        int verifyCommandCalled(std::string command);
        void clearCommandBuffer() { _commandsCalledBuffer="";}

    protected:
        mutable std::string _commandsCalledBuffer;

};

#endif //VERIFIER_H