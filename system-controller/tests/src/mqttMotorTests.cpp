#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <thread>

#include "log.h"
#include "mqttMotor.h"
#include "motorData.h"

TEST(MqttMotor,basics ){
   Log::Init();
    std::string serial = "0000000000001", pn ="0268253";
    bool maxSpeedMessageSend = false, minSpeedMessageSend = false;
    MqttMotor sut(pn,serial);
    sut.setDelegateMotorOutput([& maxSpeedMessageSend,& minSpeedMessageSend](MqttData data){
        if (data.getTopic().find("get.maxspeed") != std::string::npos) {
            maxSpeedMessageSend = true;
        }
        if (data.getTopic().find("get.minspeed") != std::string::npos) {
            minSpeedMessageSend = true;
        }
    });
    sut.onMotorConnected();
    // no max and min speed set yet
    EXPECT_FALSE(sut.getIsConfigured()); 

    // fake that results come from motor regarding min and max speed
    MqttData mqttData_maxSpeed("rbus/" + pn + "/" + serial + "/rbus.get.maxspeed/result","{\"results\":120}") ;
    auto maxSpeedMotorData = MotorData(mqttData_maxSpeed);
    sut.onMotorInput(maxSpeedMotorData);
    MqttData mqttData_minSpeed("rbus/" + pn + "/" + serial + "/rbus.get.minspeed/result","{\"results\":20}") ;
    auto minSpeedMotorData = MotorData(mqttData_minSpeed);
    sut.onMotorInput(minSpeedMotorData);
    
    // be sure that it is evaluated by waiting double of the interval time of the onMotorConnected 
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(sut.getIsConfigured());
    EXPECT_TRUE(maxSpeedMessageSend);
    EXPECT_TRUE(minSpeedMessageSend);
    sut.onMotorDisconnected();  
}

TEST(MqttMotor,actions ){
    Log::Init();
    std::string serial = "0000000000001", pn ="0268253";
    MqttMotor sut(pn,serial);

    bool openSend = false, closeSend=false, stopSend= false;
    sut.setDelegateMotorOutput([& openSend,& closeSend, & stopSend](MqttData data){
        if (data.getTopic().find("open") != std::string::npos) {
            openSend = true;
        }
        if (data.getTopic().find("close") != std::string::npos) {
            closeSend = true;
        }
        if (data.getTopic().find("stop") != std::string::npos) {
            stopSend = true;
        }
    });
    sut.onMotorConnected();

    sut.open();     EXPECT_TRUE(openSend);
    sut.close();    EXPECT_TRUE(closeSend);
    sut.stop();     EXPECT_TRUE(stopSend);
    sut.onMotorDisconnected(); 
}

TEST(MqttMotor,isStopAcked ){
    Log::Init();
    std::string serial = "0000000000001", pn ="0268253";
    MqttMotor sut(pn,serial);
   
    sut.setDelegateMotorOutput([](MqttData data){ /*do noting*/   });
    sut.onMotorConnected();
    sut.stop();
    sut.open();
    EXPECT_FALSE(sut.getIsMotorStopped()) << "Motor can not be stopped when commanded to open";

    // todo fake stop_ack response and check again

    sut.onMotorDisconnected(); 
}

TEST(MqttMotor,polling ){
    Log::Init();
    std::string serial = "0000000000001", pn ="0268253";
    MqttMotor sut(pn,serial);
    std::vector<std::thread> workers;

    // reply with a small delay in other thread
    auto motorReply = [pn, serial,&sut]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        // this status has the IsInitialized(=isCalibratedForStroke) on true
         MqttData mqttData_status("rbus/" + pn + "/" + serial + "/rbus.get.status/result"
                        ,"{\"results\":\"12,1000,50,false,false,true,34,20,false,false,false,false,false,true,false\"}") ;
            auto statusMotorData = MotorData(mqttData_status);
        sut.onMotorInput(statusMotorData);
    };

    int statusSetCounter = 0; int strokeGetCounter=0;
    sut.setDelegateMotorOutput([&statusSetCounter, & strokeGetCounter, &sut, &workers,& motorReply](MqttData data){
        if (data.getTopic().find("status") != std::string::npos) {
            statusSetCounter++;
            // start execution in other thread and return already -> 
            std::thread w(motorReply);
            workers.push_back(std::move(w)); // threads can not be copied! use the move constructin here
        }
        if (data.getTopic().find("stroke") != std::string::npos) {
            strokeGetCounter++;
        }
    });
    sut.onMotorConnected();

    // fake that results come from motor regarding min and max speed to get it configured
    MqttData mqttData_maxSpeed("rbus/" + pn + "/" + serial + "/rbus.get.maxspeed/result","{\"results\":120}") ;
    auto maxSpeedMotorData = MotorData(mqttData_maxSpeed);
    sut.onMotorInput(maxSpeedMotorData);
    MqttData mqttData_minSpeed("rbus/" + pn + "/" + serial + "/rbus.get.minspeed/result","{\"results\":20}") ;
    auto minSpeedMotorData = MotorData(mqttData_minSpeed);
    sut.onMotorInput(minSpeedMotorData);
    int counter=0;
    while(!sut.getIsConfigured()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        EXPECT_LE(counter,20) <<"Should be configured within 100's of miliseconds!";
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    EXPECT_GE(statusSetCounter, 2) << "Within 400ms we should have more than 2 status updates due min 100ms sleep";
    EXPECT_GE(strokeGetCounter, 1) << "When 2 status updated after the first update (containing true for isCalibrated) a stroke get command should be send";

    sut.onMotorDisconnected(); 
    // don't forget to cleanup threads
    for ( auto & w : workers) {
        if ( w.joinable()) {
            w.join();
        }
    }
}

TEST(MqttMotor,pollingWithoutResponse ){
    Log::Init();
    std::string serial = "0000000000001", pn ="0268253";
    MqttMotor sut(pn,serial);

    int statusSetCounter = 0; int strokeGetCounter =0;
    sut.setDelegateMotorOutput([&statusSetCounter,& strokeGetCounter, &sut, pn, serial](MqttData data){
        if (data.getTopic().find("status") != std::string::npos) {
            statusSetCounter++;
        }
        if (data.getTopic().find("stroke") != std::string::npos) {
            strokeGetCounter++;
        }
    });
    sut.onMotorConnected();

    // fake that results come from motor regarding min and max speed to get it configured
    MqttData mqttData_maxSpeed("rbus/" + pn + "/" + serial + "/rbus.get.maxspeed/result","{\"results\":120}") ;
    auto maxSpeedMotorData = MotorData(mqttData_maxSpeed);
    sut.onMotorInput(maxSpeedMotorData);
    MqttData mqttData_minSpeed("rbus/" + pn + "/" + serial + "/rbus.get.minspeed/result","{\"results\":20}") ;
    auto minSpeedMotorData = MotorData(mqttData_minSpeed);
    sut.onMotorInput(minSpeedMotorData);
    int counter=0;
    while(!sut.getIsConfigured()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        EXPECT_LE(counter,20) <<"Should be configured within 100's of miliseconds!";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(550));
    EXPECT_EQ(statusSetCounter, 2) << "Within 550 ms we should have no more than 2 status updates due 250ms max wait time";
    EXPECT_EQ(strokeGetCounter, 0) << "Within 550 ms we should have no requests for the stroke due no calibrated motor";
    
    sut.onMotorDisconnected(); 
}

TEST(MqttMotor,clearCalibration ){
    Log::Init();
    std::string serial = "0000000000001", pn ="0268253";
    std::thread worker;
    MqttMotor sut(pn,serial);
    bool isClearPublished =false;

  // reply with a small delay in other thread
    auto motorReply = [pn, serial,&sut](bool isCalibrated) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        std::string isCalib = (isCalibrated ? "true" : "false") ;
         MqttData mqttData_status("rbus/" + pn + "/" + serial + "/rbus.get.status/result"
                        ,"{\"results\":\"12,1000,50,false,false,true,34,20,false,false,false,false,false,"+ isCalib +",false\"}") ;
            auto statusMotorData = MotorData(mqttData_status);
        sut.onMotorInput(statusMotorData);
    };

    sut.setDelegateMotorOutput([&](const MqttData & data) {
        // check every time this function is called of it is a clearPublish (and don't overwrite result with newer command)
        if  (data.getTopic().find("calib.clear") != std::string::npos) {
            isClearPublished=true;
            // fake Status with -> no calibration
            worker  = std::thread (motorReply,false);            
        }
    });
    sut.onMotorConnected();
    
    // ----- fake a calibration to be cleared


    // first configure motor otherwise all status info's are ignored
    MqttData mqttData_maxSpeed("rbus/" + pn + "/" + serial + "/rbus.get.maxspeed/result","{\"results\":120}") ;
    auto maxSpeedMotorData = MotorData(mqttData_maxSpeed);
    sut.onMotorInput(maxSpeedMotorData);
    MqttData mqttData_minSpeed("rbus/" + pn + "/" + serial + "/rbus.get.minspeed/result","{\"results\":20}") ;
    auto minSpeedMotorData = MotorData(mqttData_minSpeed);
    sut.onMotorInput(minSpeedMotorData);
    int counter=0;
    while(!sut.getIsConfigured()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        EXPECT_LE(counter,20) <<"Should be configured within 100's of miliseconds!";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(550));

    // fake that results come from motor regarding min and max speed to get it configured
    motorReply(true);


    EXPECT_TRUE(sut.isCalibrated()) <<"Expect to be calibrated with fake status message";    

    EXPECT_TRUE(sut.clearCalibration().get())<<"should return a succesfull clear calibration";
    sut.onMotorDisconnected();
    EXPECT_TRUE(isClearPublished) <<"Clear message should be published by the command manager";    
    if ( worker.joinable()) {
        worker.join();
    }
    
}