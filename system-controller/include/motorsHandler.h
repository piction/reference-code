#ifndef MOTORSHANDLER_H
#define MOTORSHANDLER_H

#include "pch.h"
#include "topicHandler.h"
#include "motorizedWindow.h"
#include "motorData.h"
#include "mqttMotor.h"

class MotorsHandler : public TopicHandler {
    public:
        MotorsHandler();
        ~MotorsHandler(){};
        void handleNewInput ( const MqttData & inputData) override;
        void handleOutput(const MqttData & data);
        void addMotor(const std::shared_ptr<IMqttMotor>& motionHanlder);
        void start() override;
        void stop() override;
        std::string getType() const override {return "MOTORS";};        
    private :
        std::map<std::string,std::shared_ptr<IMqttMotor>> _motors;
        std::mutex _motorsMap_mutex;
};

#endif //MOTORSHANDLER_H