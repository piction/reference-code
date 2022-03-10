#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <cmath>

#include "mqttData.h"
#include "motorsHandler.h"
#include "testMqttMotor.h"
#include "log.h"

TEST(MotorsHandler,construct) {
    MotorsHandler sut;
    bool hasCorrectSubscriberStr = false;
    for ( auto &s : sut.getSubscribeStrs()) {
        hasCorrectSubscriberStr = hasCorrectSubscriberStr || s.compare("rbus/#") == 0;
    }
    EXPECT_TRUE(hasCorrectSubscriberStr) << "Should not mis the correct subscribe str ";
}

TEST(MotorsHandler,startup ){
    Log::Init();
    MotorsHandler sut;
    
    auto testMotor1 = std::make_shared<TestMqttMotor>("1");
    auto testMotor2 = std::make_shared<TestMqttMotor>("2");
    sut.addMotor(testMotor1);
    sut.addMotor(testMotor2);
    testMotor1->clearCommandBuffer();
    testMotor2->clearCommandBuffer();
    sut.start();    
    EXPECT_TRUE(testMotor1->verifyCommandCalled("onMotorConnected",1)) << "Expect to call motor (1) function OnMotorConnected";
    EXPECT_TRUE(testMotor2->verifyCommandCalled("onMotorConnected",1)) << "Expect to call motor (2) function OnMotorConnected";
    sut.stop();    
}
TEST(MotorsHandler,onInput ){
    Log::Init();
    MotorsHandler sut;
    
    auto testMotor1 = std::make_shared<TestMqttMotor>("0628252/0000000000001");
    auto testMotor2 = std::make_shared<TestMqttMotor>("0628252/0000000000002");
    sut.addMotor(testMotor1);
    sut.addMotor(testMotor2);
    testMotor1->clearCommandBuffer();
    testMotor2->clearCommandBuffer();
    sut.start();    
    MqttData d1("rbus/0628252/0000000000001/rbus.get.position.mm/result","payload");
    MqttData d2("rbus/0628252/0000000000002/rbus.get.position.mm/result","payload");
    MqttData nonValidData("rbus/0628252/0000000000003/rbus.get.position.mm/result","payload");
    sut.handleNewInput(d1);
    sut.handleNewInput(d2);
    sut.handleNewInput(nonValidData);
    EXPECT_TRUE(testMotor1->verifyCommandCalled("onMotorInput",1)) << "Expect to call motor (1) function onMotorInput";
    EXPECT_TRUE(testMotor2->verifyCommandCalled("onMotorInput",1)) << "Expect to call motor (2) function onMotorInput";

    sut.stop();    
}

TEST(MotorsHandler,startStop ){
    Log::Init();
    MotorsHandler sut;    
    auto testMotor1 = std::make_shared<TestMqttMotor>("0628252/0000000000001");
    sut.addMotor(testMotor1);

    sut.start();
    EXPECT_TRUE(sut.isRunning()) << "Expect properly started motorshandler";
    sut.stop();
    EXPECT_FALSE(sut.isRunning()) << "Expect properly stopped motorshandler";
}
