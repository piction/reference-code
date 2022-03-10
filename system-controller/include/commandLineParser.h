#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <ctype.h>
#include <unistd.h>
#include "pch.h"

class CommandLineParser
{
    public : 
        CommandLineParser(std::string defaultLogPath ) : _logPath(defaultLogPath) {};

        bool tryParse (int argc, char **argv);   
        int getPort () const {return _port;}
        std::string getLogPath() const {return _logPath;}
        bool generateScriptsOn () const {return _isGenerateScriptsOn;}
        std::string getConfigFilePath () const {return _filePathOfConfig;}
        std::stringstream errorMessage;
    private:
        bool _isGenerateScriptsOn = false;
        std::string _filePathOfConfig = "";
        std::string _logPath;
        
        int _port = 1883; 
            
};


#endif //COMMANDLINEPARSER_H