#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <cmath>

#include "log.h"
#include "wingData.h"

TEST(WingData,basics ){
    Log::Init();

    auto testInput = [](const std::string &cmd ,const WingComandType & cmdType ) {
        std::string wingAndId = "systemcontroller/wing/wing1/";
        MqttData mqttData_open(wingAndId + cmd) ;
        auto sut_open = WingData(mqttData_open);
        EXPECT_TRUE(sut_open.getId().compare("wing1") == 0);
        EXPECT_TRUE(sut_open.getWingCommand() == cmdType) << cmd << " should produce correct WingComandType";
    };
    testInput("open",  WingComandType::Open);
    testInput("Open",  WingComandType::Open);
    testInput("openOrStop",  WingComandType::OpenOrStop);
    testInput("OpenOrStop",  WingComandType::OpenOrStop);
    testInput("Close",  WingComandType::Close);
    testInput("close",  WingComandType::Close);
    testInput("closeOrStop",  WingComandType::CloseOrStop);
    testInput("CloseOrStop",  WingComandType::CloseOrStop);
    testInput("pulse",  WingComandType::Pulse);
    testInput("Pulse",  WingComandType::Pulse);
    testInput("pulseOrStop",  WingComandType::PulseOrStop);
    testInput("PulseOrStop",  WingComandType::PulseOrStop);
    testInput("lock",  WingComandType::Lock);
    testInput("Lock",  WingComandType::Lock);
    testInput("calibrate",  WingComandType::Calibrate);
    testInput("Calibrate",  WingComandType::Calibrate);
    testInput("cancel",  WingComandType::Cancel);
    testInput("Cancel",  WingComandType::Cancel);
    testInput("stop",  WingComandType::Stop);
    testInput("Stop",  WingComandType::Stop);
  

    MqttData mqttData_posMm("systemcontroller/wing/wing1/setPosition" , "{\"parameters\":{\"positionType\":\"mm\",\"value\":250.36}}") ;
    auto sut_posMm = WingData(mqttData_posMm);
    EXPECT_TRUE(sut_posMm.getId().compare("wing1") == 0);
    EXPECT_TRUE(sut_posMm.getWingCommand() == WingComandType::SetPosition);
    
    int sut_posMm_posMm = 0;
    double sut_posMm_posPerc = 0;
    sut_posMm.getPosition(sut_posMm_posPerc, sut_posMm_posMm);
    
    EXPECT_TRUE(250 == sut_posMm_posMm);
}