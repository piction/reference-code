#ifndef WINGSHANDLER_H
#define WINGSHANDLER_H

#include "pch.h"
#include "topicHandler.h"
#include "wing.h"
#include "wingInputTranslator.h"




class WingsHandler : public TopicHandler {
    public:
        WingsHandler(const std::string & configId , std::shared_ptr<IWingInputTranslator> inputTranslator);
        void handleNewInput ( const MqttData & inputData) override;
        std::string addWing(const std::shared_ptr<IWing>& wing);
        void start();
        void stop();
        std::string getType() const override {return "WING";};        
    private :
        std::map<std::string,std::shared_ptr<IWing>> _wings;
        std::string _configId;
        std::mutex _wingsMap_mutex;
        bool _isUpdateWingMovementRunning = false;
        std::thread _workerThreadWingMovement;
        void evaluateAllWings();
        void handleOutput(const MqttData & data);
        std::shared_ptr<IWingInputTranslator> _inputTranslator;
};

#endif //WINGSHANDLER_H