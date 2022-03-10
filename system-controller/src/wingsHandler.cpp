#include "wingsHandler.h"
#include "wingData.h"
#include "log.h"





WingsHandler::WingsHandler(const std::string & configId , std::shared_ptr<IWingInputTranslator> inputTranslator) 
: TopicHandler({"systemcontroller/"+configId+ "/wing/#"}) , _configId(configId) , _inputTranslator(inputTranslator) {

}
// add a wing to the list to be hanlded with from MQTT
std::string WingsHandler::addWing(const std::shared_ptr<IWing>& wing){
    std::lock_guard<std::mutex> guard(_wingsMap_mutex);
    if(_wings.find(wing->getWingId()) != _wings.end()) {
        LOG_ERROR("Wingshandler contains already wing " + wing->getWingId());
    }
    _wings.insert(std::pair<std::string,std::shared_ptr<IWing>>(wing->getWingId(),wing));
    LOG_TRACE("Added new wing to wingshandler: " + wing->getWingId());
    wing->setDelegateWingPublishOutput([&](const MqttData & data) {handleOutput(data);});
    return wing->getWingId();
}

void WingsHandler::handleOutput(const MqttData & data) {
    LOG_TRACE("Send of for output ..." + std::string(data));
    _pOutTypeBuffer->QueueNewMessage(data);
}
void WingsHandler::handleNewInput ( const MqttData & inputData) {
    std::lock_guard<std::mutex> guard(_wingsMap_mutex);


    auto wingData = WingData(inputData);
    if ( wingData.getWingCommand() != WingComandType::Ignore) {
        LOG_DEBUG("New Input for wing: " + inputData.getTopic() + "[" + inputData.getPayload() + "]");
    } else {
        return;
    }
    //lookup if we have a wing registerd with that GUID
    std::string receivedWingId = wingData.getId();    
    if(_wings.find(receivedWingId) != _wings.end()) {
        bool commandFound = true;
        auto wingSharedPtr = _wings[receivedWingId];
        _inputTranslator->translateInputToAction(wingSharedPtr->getMasterWindow()->getMotionManager()->getMotorStatusData(),
                                                wingSharedPtr->getPosition(),
                                                wingSharedPtr->waslastMovementOpening(),                                                
                                                wingData.getWingCommand(),
                                                [& wingSharedPtr]() {wingSharedPtr->open();},
                                                [& wingSharedPtr]() {wingSharedPtr->stop();},
                                                [& wingSharedPtr]() {wingSharedPtr->close();},
                                                [& wingSharedPtr]() {wingSharedPtr->startCalibrate();},
                                                [& wingSharedPtr]() {wingSharedPtr->clearCalibration();},
                                                [& wingSharedPtr]() {
                                                    LOG_WARNING("LOCK is not implemented yet"); 
                                                },
                                                [& wingData, & commandFound, & wingSharedPtr]() {
                                                    // position
                                                    double posPerc = -1;
                                                    int posMm = -1;
                                                    wingData.getPosition(posPerc, posMm);
                                                    if ( posPerc > 0) {
                                                        wingSharedPtr->setPositionPerc(posPerc);    
                                                    }
                                                    else if ( posMm > 0) {
                                                        wingSharedPtr->setPositionMm(posMm);    
                                                    } else {
                                                        LOG_WARNING("Missing valid positin parameters to do a SetPosition");
                                                        commandFound = false;
                                                    }
                                                },
                                                [& commandFound]() {commandFound = false;} // action done when no command found
        );

      
        if ( commandFound) {
            _pOutTypeBuffer->QueueNewMessage(wingData.getAck());
        }
    }        
}
// regulary check all wings, one wing can cause another wing to be able to move!
void WingsHandler::evaluateAllWings() {
    LOG_DEBUG("Wingshandler updateMovement evaluation started");
    while(_isUpdateWingMovementRunning) {
        {
            std::lock_guard<std::mutex> guard(_wingsMap_mutex);
            for ( auto &w : _wings) {
                w.second->updateWingMovement();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void WingsHandler::start() { 
    TopicHandler::start() ;
    _isUpdateWingMovementRunning = true;
    _workerThreadWingMovement = std::thread(&WingsHandler::evaluateAllWings, this);

    LOG_DEBUG("WingsHandler started");
    // show all connected wings on start 
    for ( auto &w : _wings) {
        std::string json ="{";
        json.append("\"id\":");
        json.append("\""); json.append(w.first); json.append("\"");
        json.append("}");
        MqttData data( "systemcontroller/"+ _configId + "/info",json);
        _pOutTypeBuffer->QueueNewMessage(data);
    }
}

void WingsHandler::stop() { 
     if ((!_isUpdateWingMovementRunning)  && (!TopicHandler::isRunning())) {
            LOG_WARNING("Wingshandler stop called when already completely stopped");
    } else {
        _isUpdateWingMovementRunning = false;
        if (_workerThreadWingMovement.joinable()) {
            _workerThreadWingMovement.join();
        }
        LOG_DEBUG("Wingshandler updateMovement worker stopped");
    }

    TopicHandler::stop() ;
    LOG_DEBUG("WingsHandler stopped");
}