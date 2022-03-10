#include "commandLineParser.h"
#include "log.h"

bool CommandLineParser::tryParse(int argc, char **argv) {
    bool isGenerateScriptsOn= false;
    char *configValue = NULL;
    char *logValue = NULL;
    char *portValue = NULL;  
    int cmdLineArgument;

     std::cout << "You have entered " << argc 
         << " arguments:" << "\n"; 
  
    for (int i = 0; i < argc; ++i) 
        std::cout << argv[i] << "\n"; 
    
    char* errorMsg;
    while ((cmdLineArgument = getopt (argc, argv, "sl:c:p:")) != -1){
        switch (cmdLineArgument)
        {
        case 's':
            isGenerateScriptsOn = true;
            break;
        case 'c':
            configValue = optarg;
            break;
        case 'l':
            logValue = optarg;
            break;
        case 'p':
            portValue = optarg;
            break;
        case ':':
            sprintf(errorMsg, "Missing ??? %c", optopt);
            if ( errorMessage.gcount() >1) { errorMessage << "\n";}
            errorMessage << errorMsg;            
            break;

        case '?':
            if (optopt == 'c' || optopt == 'p')
                sprintf (errorMsg,"Option -%c requires an argument.", optopt );
            else if (isprint (optopt))
                sprintf (errorMsg, "Unknown option `-%c'.", optopt);
            else 
                sprintf (errorMsg,"Unknown option character `\\x%x'.",optopt);
            
            if ( errorMessage.gcount() >1) { errorMessage << "\n";}
            errorMessage <<  errorMsg;
            return false; 
            
        default:
            abort ();
            return false;
        }
    }
    // check on valid port
    if(portValue != NULL ) {
        _port = atoi(portValue);
        if (_port == 0) return false;
    } 
    // check if file exists
    if (configValue == NULL || access( configValue, F_OK ) == -1 )
    {
        if ( errorMessage.gcount() >1) { errorMessage << "\n";}
        errorMessage << "No valid config file is given";
        return false;
    } 
    // check if file is a JSON file
    _filePathOfConfig = std::string(configValue);
    if(_filePathOfConfig.substr(_filePathOfConfig.find_last_of(".") + 1) != "json" && _filePathOfConfig.substr(_filePathOfConfig.find_last_of(".") + 1) != "JSON"  ) {
        if ( errorMessage.gcount() >1) { errorMessage << "\n";}
        errorMessage<< "The configuration file '" << _filePathOfConfig << "' is not a JSON file.";
        return false;
    } 

    if ( logValue !=NULL){
        _logPath = std::string(logValue);
    }
    
    _isGenerateScriptsOn = (bool) isGenerateScriptsOn;

    std::string generateScriptsStr = _isGenerateScriptsOn ? "ON" : "OFF";
    if ( errorMessage.gcount() >1) { errorMessage << "\n";}
    errorMessage << "Valid Cmd arguments: configFilePath -> " << _filePathOfConfig << ", port -> " << _port << " option generated scripts " <<  generateScriptsStr;
    return true;
}