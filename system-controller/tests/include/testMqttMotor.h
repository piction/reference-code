#ifndef TESTMQTTMOTOR_H
#define TESTMQTTMOTOR_H

#include "verifier.h"
#include "mqttMotor.h"

class TestMqttMotor: public IMqttMotor , public Verifier {
    public: 
    TestMqttMotor( std::string name) {
        id = name;
    }
    void onMotorInput(const MotorData & data) override {
        _commandsCalledBuffer.append("onMotorInput,");
    }
    void onMotorConnected() override {
        _commandsCalledBuffer.append("onMotorConnected,");
    }
    void onMotorDisconnected() override {
        _commandsCalledBuffer.append("onMotorDisconnected,");
    }
    void setDelegateMotorOutput(std::function<void(MqttData)> delegateMotorOutput) override {
        _commandsCalledBuffer.append("setDelegateMotorOutput,");
    }
    std::string getId() const override {
        return (id);
    }
    std::string id;
};

#endif //TESTMQTTMOTOR_H