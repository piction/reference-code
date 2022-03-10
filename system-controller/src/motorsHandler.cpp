#include "motorsHandler.h"
#include "log.h"

MotorsHandler::MotorsHandler() : TopicHandler({"rbus/#"}) {

}
// add a motor to the list to be hanlded with from MQTT
void MotorsHandler::addMotor(const std::shared_ptr<IMqttMotor>& mqttMotor){
    LOG_TRACE("Added new motor to motorshandler " + mqttMotor->getId());
    std::lock_guard<std::mutex> guard(_motorsMap_mutex);
    _motors.insert(std::pair<std::string,std::shared_ptr<IMqttMotor>>(mqttMotor->getId(),mqttMotor));
    mqttMotor->setDelegateMotorOutput([&](const MqttData & data) {handleOutput(data);});
}

void MotorsHandler::handleNewInput ( const MqttData & inputData) {
    
    //LOG_DEBUG("New Input for motor: " + inputData.getTopic() + "[" + inputData.getPayload() + "]");
    std::lock_guard<std::mutex> guard(_motorsMap_mutex);
    auto motorData = MotorData(inputData);
    if(_motors.find(motorData.getId()) != _motors.end()) {
        _motors[motorData.getId()]->onMotorInput(inputData);
    }        
}

void MotorsHandler::handleOutput(const MqttData & data) {
    if (std::string::npos == data.getTopic().find("get.status")) { // only log non status messages
        LOG_TRACE("Motor request: " + std::string(data));
    }
    _pOutTypeBuffer->QueueNewMessage(data);
}
void MotorsHandler::start() { 
    TopicHandler::start() ;
    for ( auto &m: _motors) {
        m.second->onMotorConnected();
    }
    LOG_DEBUG("MotorsHandler started");
}

void MotorsHandler::stop() { 
    TopicHandler::stop() ;
    LOG_DEBUG("MotorsHandler stopped");
}