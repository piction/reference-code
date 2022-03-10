#ifndef COMMANDSMANAGER_H
#define COMMANDSMANAGER_H

#include "pch.h"
#include <mqttData.h>

enum class CommandType {
    SetParam,
    SetMovement,
    Get
};

// this class manages all 'set' commands to prevent double sending
// when a message is not acked within time this should also resend 
// the 'set' message
class ICommandsManager {
    public:
    virtual ~ICommandsManager(){}
    virtual void stopEvaluating() = 0;
    virtual void startEvaluating() = 0;
    virtual void pushCommandoToBeSend(MqttData message, CommandType commandType) =0;
    virtual void handleAck(const MqttData & ackMessage) = 0;
    virtual void setSendHandler(std::function<void(MqttData)> delegateSend) = 0;
};

class CommandsInfo {
    public:
        CommandsInfo(MqttData mqttData ) : data(mqttData), timeOfPublish(std::chrono::system_clock::now()){}
        MqttData data;
        std::chrono::time_point<std::chrono::system_clock> timeOfPublish;
        bool isAcked = false;
        int resendCounter = 0;
};

class CommandsManager : public ICommandsManager {
           
    public:
    CommandsManager();
    ~CommandsManager () {}
    void stopEvaluating() override;
    void startEvaluating() override;    
    void pushCommandoToBeSend(MqttData message, CommandType commandType) override; 
    void handleAck(const MqttData & ackMessage)  override;
    void setSendHandler(std::function<void(MqttData)> delegateSend) override;
    int getNumberOfManagedCommands() const;

    private:
    std::map<std::string, CommandsInfo> _commandsBuffer;
    std::tuple<std::string,CommandsInfo> _lastMoveCommand;
    std::function<void(MqttData &)> _delegateSend ;
    void evaluateAllCommandsInBuffer();
    void send(MqttData & message) const;
    bool _sendDelegateIsSet = false;
    bool _isRunning = false;
    std::thread _workerThread;
    mutable std::mutex _mutex; // mark mutable to allow hold of mutex during const methods
    static const int _maxResendCountBeforeSlowDownOfSending=5;
};

#endif // COMMANDSMANAGER_H