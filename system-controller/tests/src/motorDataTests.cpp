#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <cmath>

#include "motorData.h"
#include "log.h"

TEST(MotorData,basics ){
    
    Log::Init();
    std::string motorId = "0628252/0000000000001";
    std::string motorAndId = "rbus/"+motorId+"/";
    std::string motorCommand = "rbus.get.status";
    MqttData mqttData_status(motorAndId + motorCommand + "/result","{\"results\":\"12,1000,50,false,false,true,34,20,false,false,false,false,true,false\"}") ;
    auto sut = MotorData(mqttData_status);

    EXPECT_TRUE(sut.getId().compare(motorId) == 0);
    EXPECT_TRUE(sut.getCommand().compare("rbus.get.status") == 0);

    MotorStatusData  motorStatus;
    
    sut.parseStatusData(motorStatus);
    EXPECT_TRUE(motorStatus.posMm == 12);
    EXPECT_TRUE(motorStatus.speedMm == 50);
    EXPECT_FALSE(motorStatus.isLocked) << "lock should be parsed correct";
    EXPECT_FALSE(motorStatus.isOpen) << "isOpen should be parsed correct";
    EXPECT_TRUE(motorStatus.isClosed)<< "isClosed should be parsed correct";
}


TEST(MotorData,getOnevalue ){
    Log::Init();
    std::string motorId = "0628252/0000000000001";
    std::string motorAndId = "rbus/"+motorId+"/";
    std::string motorCommand = "rbus.get.maxspeed";
    MqttData mqttData_status(motorAndId + motorCommand + "/result","{\"results\":999}") ;
    auto sut = MotorData(mqttData_status);

    EXPECT_TRUE(sut.getId().compare(motorId) == 0);
    EXPECT_TRUE(sut.getCommand().compare("rbus.get.maxspeed") == 0);
    
    int maxSpeed = 0;
    sut.parseOneValue(maxSpeed);

    EXPECT_TRUE(maxSpeed == 999);
}

TEST(MotorData,checkResponse ){
    Log::Init();
    std::string motorId = "0628252/0000000000001";
    std::string motorAndId = "rbus/"+motorId+"/";
    std::string motorCommand = "rbus.stop";
    MqttData mqttData_stopWithAck(motorAndId + motorCommand + "/result","{\"results\":\"stop_ack\"}") ;
    auto sut = MotorData(mqttData_stopWithAck);

    EXPECT_TRUE(sut.getId().compare(motorId) == 0);
    EXPECT_TRUE(sut.getCommand().compare("rbus.stop") == 0);
    
    EXPECT_TRUE(sut.isAck()) << "ack is present, should result true";


    MqttData mqttData_stopNoAck(motorAndId + motorCommand + "/result","{\"results\":\"stop\"}") ;
    sut = MotorData(mqttData_stopNoAck);

    EXPECT_TRUE(sut.getId().compare(motorId) == 0);
    EXPECT_TRUE(sut.getCommand().compare("rbus.stop") == 0);
    
    EXPECT_TRUE(sut.isAck()) << "Also ack because a result is an acknowledgment";


    MqttData mqttData_stopTriggerNoAck(motorAndId + motorCommand + "/trigger","") ;
    sut = MotorData(mqttData_stopTriggerNoAck);

    EXPECT_TRUE(sut.getId().compare(motorId) == 0);
    EXPECT_TRUE(sut.getCommand().compare("rbus.stop") == 0);
    
    EXPECT_FALSE(sut.isAck()) << "No ack is present, should result false";
}