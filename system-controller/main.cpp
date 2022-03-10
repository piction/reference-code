#include "pch.h"
#include "commandLineParser.h"
#include "motorizedWindow.h"
#include "passiveWindow.h"
#include "motorMotionManager.h"
#include "mqttManager.h"
#include "wingsHandler.h"
#include "motorsHandler.h"
#include "configBuilder.h"
#include "wingInputTranslator.h"
#include "systemSettingsParser.h"


using namespace std;


// Run this program with at least one command option -c containing the path of the configuration file
// Ex: ./systemController  -c ../../mqtt-simulated-motor/monitor/output/simulatedConfig.json -p 1883 -s

void enableLogging(std::string logFolder) {
    Log::Init(logFolder);
    
    LOG_INFO("System controller started... logs saved @" + logFolder);    
    LOG_CRITICAL("[LOG CHECK]--- critical logging is on");
    LOG_ERROR("[LOG CHECK]--- error logging is on");
    LOG_WARNING("[LOG CHECK]--- warning logging is on");
    LOG_INFO("[LOG CHECK]--- info logging is on");
    LOG_DEBUG("[LOG CHECK]--- debug logging is on");
    LOG_TRACE("[LOG CHECK]--- trace logging is on");    
}

int main (int argc , char **argv) {
    
    CommandLineParser cmdParser("logs");
    bool validCmds = cmdParser.tryParse(argc,argv);
    enableLogging(cmdParser.getLogPath());
            
    if (!validCmds) {
        LOG_ERROR("Nothing started: Failed to parse command line parameters");
        LOG_ERROR(cmdParser.errorMessage.str());
        return -1;
    }
        
    ifstream f(cmdParser.getConfigFilePath());
    ostringstream ss;
    ss << f.rdbuf(); // reading data
    
    std::string json = ss.str();
    SystemSettingsParser::parseJsonToSettings(json);
    std::string configurationId;

    auto writeWingCommandsToFiles = [&](std::string guid, int counter ) {
        auto writeToFile = [&](std::string command ) {
            std::ofstream out( command+ "-"+std::to_string(counter)+ ".sh");
            out << "mosquitto_pub -t \"systemcontroller/"+ configurationId +"/wing/" +guid+ "/"+ command + "\" -m \"\"" +  " -p " + std::to_string(cmdParser.getPort());
            out.close();
        };
        writeToFile("stop"); writeToFile("open");writeToFile("close");writeToFile("calibrate");
    };


    

    std::vector<std::shared_ptr<IWing>> wings;
    //get wings;    
    ConfigBuilder::parseFromJson(json,wings,configurationId);
    LOG_INFO("parsed " + std::to_string(wings.size()) + " wings from JSON configuration '" +cmdParser.getConfigFilePath()+"'" );
    Log::GetLogger()->flush();
    

    auto wingsHandler = make_shared<WingsHandler>(configurationId, std::make_shared<WingInputTranslator>());    
    auto motorsHandler = make_shared<MotorsHandler>();     
    std::vector<std::shared_ptr<TopicHandler>> topicHandlers;
    int wingCounter=0;

    for(auto &  wing : wings) {
        wingCounter++;
        
        std::string wingGuid = wingsHandler->addWing(wing);
        LOG_DEBUG("Added wing " + std::to_string(wingCounter) + " with guid " + wingGuid + " to wing handler");   
        if ( cmdParser.generateScriptsOn()) {
            writeWingCommandsToFiles(wingGuid,wingCounter);
        }
        LOG_DEBUG("Number of motors on the wing:" + std::to_string(wing->getMotors().size()));
        for ( auto & m : wing->getMotors())  {
            LOG_INFO("Adding motor " + m->getMotionManager()->getId());
            motorsHandler->addMotor(std::dynamic_pointer_cast<IMqttMotor>(m->getMotionManager()));
        }
    }
  
    topicHandlers.push_back(motorsHandler);
    topicHandlers.push_back(wingsHandler);

    MqttManager mqtt("localhost",cmdParser.getPort(),"systemcontroller",topicHandlers);
    const bool connected = mqtt.waitForConnection(std::chrono::milliseconds(500));
    if (connected == false) {
        LOG_WARNING("Did not connect to Mqtt broker at port " +  std::to_string(cmdParser.getPort()));
        return 1;
    }

    try {
        mqtt.start();
        Log::GetLogger()->flush();
        mqtt.waitForDisconnection();
        mqtt.stop();
    } catch (const std::exception& e) {
        LOG_CRITICAL(e.what());
        Log::GetLogger()->flush();
    }

    LOG_INFO("EXIT PROGRAM SYSTEM CONTROLLER")
    Log::GetLogger()->flush();
    return EXIT_SUCCESS;
}
