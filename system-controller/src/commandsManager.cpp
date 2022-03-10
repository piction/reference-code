#include "commandsManager.h"
#include "log.h"

CommandsManager::CommandsManager() : _lastMoveCommand(std::tuple<std::string,CommandsInfo>("", MqttData("Empty"))) {
}

void CommandsManager::stopEvaluating(){
    if (!_isRunning) {
        LOG_WARNING("CommandsManager tried to be stopped twice");
        return;
    }
    _isRunning = false;
    if (_workerThread.joinable()) {
        _workerThread.join();
    }
    LOG_DEBUG("CommandsManager stopped");
}
void CommandsManager::startEvaluating()  {
    if (_isRunning) {
        LOG_WARNING("CommandsManager tried to be restarted");
        return;
    }    
    _isRunning = true;
    _workerThread = std::thread(&CommandsManager::evaluateAllCommandsInBuffer, this);
    LOG_DEBUG("CommandsManager started");
}
// evaluate to check if we need to resend a message 
// evaluate if the resend counter!!
void CommandsManager::evaluateAllCommandsInBuffer() {

    auto checkCommand = [&](CommandsInfo & c, const std::string&  id) {
            if ( id.empty() ) {
                return;
            }
            if ( c.isAcked) {
                c.resendCounter =0;
                return;
            }
            // postpone resend based on previous attempts
            // extra wait time 
            int waitTime  =  400 + c.resendCounter * 150; // at least wait 200ms before sending again
            int extraTime = 0;
            
            if ( c.resendCounter >= _maxResendCountBeforeSlowDownOfSending) {                
                LOG_CRITICAL(std::to_string(c.resendCounter) + " fails of sending command " + (std::string)(c.data));
                extraTime = (c.resendCounter/4) * 200;                
            }   
            
            int timePassed =std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - c.timeOfPublish).count();
            if( timePassed > (waitTime + extraTime))
            {                
                if ( c.resendCounter > 5) {
                    LOG_DEBUG("Time passed:" + std::to_string(timePassed) + " ms  WaitTime: " + std::to_string(waitTime + extraTime) + " ms");
                }
                send(c.data);
                c.resendCounter++;
            }
    };

    while (_isRunning) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if ( _commandsBuffer.size() >1) {
                for ( auto & c : _commandsBuffer) {
                    checkCommand(c.second,c.first);
                }
            }            
            checkCommand(std::get<1>(_lastMoveCommand),std::get<0>(_lastMoveCommand));
        }        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    };
}
int CommandsManager::getNumberOfManagedCommands() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _commandsBuffer.size();
}
void CommandsManager::send(MqttData & message) const {
    if (!_sendDelegateIsSet) {
        LOG_CRITICAL_THROW("Send-delegate should be set for the commandsManager !")
    }
    //LOG_DEBUG("Send:" + (std::string)(message));
    _delegateSend(message);
}

void CommandsManager::pushCommandoToBeSend(MqttData message, CommandType commandType) {
    if (commandType ==  CommandType::Get) {
        send(message); 
        return;
    } 
    std::string topic = message.getTopic();
    std::size_t found = topic.find_last_of("/\\");
    std::string command = topic.substr(0,found);
    if (commandType == CommandType::SetParam) {       
        {   // make scope for lock_guard
            std::lock_guard<std::mutex> lock(_mutex);
            // check if in buffer 
            auto simularCommandItr = _commandsBuffer.find(command);
            if ( simularCommandItr == _commandsBuffer.end()) {
                // add to buffer and send  (the only place where the buffer grows -> new type of setParam- command)
                _commandsBuffer.insert(std::pair<std::string,CommandsInfo>(command,CommandsInfo(message)));
                LOG_DEBUG("Send :" + (std::string)(message));
                send(message);
                return;
            }

            // check if parameters are the same, if not update 
            // IF the same still resend the command if the previous message was old enough
            auto c =_commandsBuffer.at(command);
            bool sendAfterNewMoveCommand =  (std::get<1>(_lastMoveCommand).timeOfPublish > c.timeOfPublish) ;
            auto prevMessageIsOld =  ((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - c.timeOfPublish).count()) > 1000);
            if( sendAfterNewMoveCommand || c.data.getPayload() != message.getPayload() || prevMessageIsOld) {
                // remove 'old' setCommand from the buffer (no need to check any more)
                _commandsBuffer.erase(simularCommandItr); // this prevents the buffer from growing !! 
                // update by inserting new (=latest) command
                _commandsBuffer.insert(std::pair<std::string,CommandsInfo>(command,CommandsInfo(message)));
                LOG_DEBUG("Send :" + (std::string)(message));
                send(message);
                return;
            }
        }
        // if command with correct parameters is already in the buffer -> do noting (the evaluator will handle time-outs)
        return;
    }   
    if (commandType == CommandType::SetMovement) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (command == std::get<0>(_lastMoveCommand) ) {
            auto diff =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - std::get<1>(_lastMoveCommand).timeOfPublish).count();
            //don't resend if previous command was send 1 second before
            if( diff < 500) {        
                return;
            }   
        }       

        _lastMoveCommand = std::pair<std::string,CommandsInfo>(command, CommandsInfo(message));
        LOG_DEBUG("Send :" + (std::string)(message));
        send(message);
    }

}
void CommandsManager::handleAck(const MqttData & ackMessage) {

    std::string topic = ackMessage.getTopic();
    std::size_t found = topic.find_last_of("/\\");
    std::string command = topic.substr(0,found);
    
    {   // make scope for lock_guard
        std::lock_guard<std::mutex> lock(_mutex);
        if (command == std::get<0>(_lastMoveCommand)) {
            std::get<1>(_lastMoveCommand).isAcked = true;
            return;
        }
         // check if in buffer 
        auto simularCommandItr  = _commandsBuffer.find(command);
        if ( simularCommandItr != _commandsBuffer.end()) {
            _commandsBuffer.at(command).isAcked = true;
            return;
        } else {
            //LOG_TRACE("Acked message that was not send from this commandsManager! :" + (std::string)(ackMessage));
        }
    }
}
void CommandsManager::setSendHandler(std::function<void(MqttData)> delegateSend) {
    _delegateSend = delegateSend;
    _sendDelegateIsSet = true;
}
