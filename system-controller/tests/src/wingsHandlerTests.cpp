#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <cmath>

#include "mqttData.h"
#include "testMotorMotionManager.h"
#include "testWingStatusPublisher.h"
#include "wingInputTranslator.h"
#include "wingsHandler.h"
#include "wing.h"
#include "testWing.h"
#include "log.h"

TEST(WingsHandler,construct) {
    Log::Init();
    WingsHandler sut("dummyId", std::make_shared<WingInputTranslator>());
    bool hasCorrectSubscriberStr = false;
    for ( auto &s : sut.getSubscribeStrs()) {
        hasCorrectSubscriberStr = hasCorrectSubscriberStr || s.compare("systemcontroller/dummyId/wing/#") == 0;
    }
    EXPECT_TRUE(hasCorrectSubscriberStr) << "Should not mis the correct subscribe str ";
}

TEST(WingsHandler,basics ){
    Log::Init();
    WingsHandler sut("dummyConfigId", std::make_shared<WingInputTranslator>());
    
    auto wing1 = std::make_shared<TestWing>(2000,"wing1");
    auto wing2 = std::make_shared<TestWing>(2000,"wing2");

    std::string id1 = sut.addWing(wing1);
    std::string id2 = sut.addWing(wing2); // should not cause problems having multiple wings !



    EXPECT_TRUE(!id1.empty()) << "Should receive a WingName";
    EXPECT_TRUE(!id2.empty()) << "Should receive a different WingName";
    EXPECT_TRUE(id1.compare(id2)!=0) << "Wingnames should be different";

    MqttData mqttDataDummy("systemcontroller/dummyConfigId/wing/wing999/open","");
    // nothing should happen but also no execption because the wing id is not registerd
    sut.handleNewInput(mqttDataDummy);

    //send again but now with one wing that is 'subscribed'
    MqttData mqttDataOpen("systemcontroller/dummyConfigId/wing/"+ id1 + "/open","");
    sut.handleNewInput(mqttDataOpen);
    bool isCalledCorrect = std::dynamic_pointer_cast<Verifier>(wing1)->verifyCommandCalled("open",1);
    EXPECT_TRUE(isCalledCorrect) << "suspect open to be called ";

    MqttData mqttDataClose("systemcontroller/dummyConfigId/wing/"+ id1 + "/close","");
    sut.handleNewInput(mqttDataClose);
    isCalledCorrect = std::dynamic_pointer_cast<Verifier>(wing1)->verifyCommandCalled("close",1);
    EXPECT_TRUE(isCalledCorrect) << "suspect close to be called ";

    MqttData mqttDataPosMm("systemcontroller/dummyConfigId/wing/"+ id1 + "/setPosition","{\"parameters\":{\"positionType\":\"mm\",\"value\":25.36}}") ;
    sut.handleNewInput(mqttDataPosMm);
    isCalledCorrect = std::dynamic_pointer_cast<Verifier>(wing1)->verifyCommandCalled("setPositionMm",1);
    EXPECT_TRUE(isCalledCorrect) << "suspect setPositionPerc to be called ";

    MqttData mqttDataPosPerc("systemcontroller/dummyConfigId/wing/"+ id1 + "/setPosition","{\"parameters\":{\"positionType\":\"perc\",\"value\":25.36}}") ;
    sut.handleNewInput(mqttDataPosPerc);
    isCalledCorrect = std::dynamic_pointer_cast<Verifier>(wing1)->verifyCommandCalled("setPositionPerc",1);
    EXPECT_TRUE(isCalledCorrect) << "suspect setPositionPerc to be called ";

    int buffSize = sut.getOutputBuffer()->GetSize();
    EXPECT_TRUE(buffSize == 4) << "Expect all messages with correct wing Id to be acked in the output buffer";


    // check that only the one with the correct ID is called
    MqttData mqttDataOpen_id1("systemcontroller/dummyConfigId/wing/"+ id1 + "/open","");
    MqttData mqttDataOpen_id2("systemcontroller/dummyConfigId/wing/"+ id2 + "/open","");
    std::dynamic_pointer_cast<Verifier>(wing1)->clearCommandBuffer();
    std::dynamic_pointer_cast<Verifier>(wing2)->clearCommandBuffer();
    sut.handleNewInput(mqttDataOpen_id1);
    sut.handleNewInput(mqttDataOpen_id2);
    bool isCalledCorrect_id1 = std::dynamic_pointer_cast<Verifier>(wing1)->verifyCommandCalled("open",1);
    EXPECT_TRUE(isCalledCorrect_id1) << "suspect open to be called only once and not twice (other id was used!) ";
    bool isCalledCorrect_id2 = std::dynamic_pointer_cast<Verifier>(wing2)->verifyCommandCalled("open",1);
    EXPECT_TRUE(isCalledCorrect_id2) << "suspect open to be called only once and not twice (other id was used!) ";    
}

TEST(WingsHandler,startStop ){
    Log::Init();
    WingsHandler sut("dummyId", std::make_shared<WingInputTranslator>());
    auto wing1 = std::make_shared<TestWing>(2000,"wing1");
    auto id = sut.addWing(wing1);

    sut.start();
    EXPECT_TRUE(sut.isRunning()) << "Expect properly started wingshandler";
    sut.stop();
    EXPECT_FALSE(sut.isRunning()) << "Expect properly stopped wingshandler";


}

TEST(WingsHandler,publishWingIdsOnStart) {

    Log::Init();
    WingsHandler sut("dummyId", std::make_shared<WingInputTranslator>());
    auto wing1 = std::make_shared<TestWing>(2000,"wing1");
    auto wing2 = std::make_shared<TestWing>(2000,"wing2");
    auto id1 = sut.addWing(wing1);
    auto id2 = sut.addWing(wing2);

    sut.start();

    // check if a publish is done of the GUID of the registerd wings
    MqttData data;
    EXPECT_EQ(sut.getOutputBuffer()->GetSize(),2) << "Size of the outputbuffer of the handler should contain 2 messages to publish"; 
    
    bool foundId1 = false;
    bool foundId2 = false;
    while(sut.getOutputBuffer()->UnqueueMessage(data)) {
        LOG_DEBUG("Payload" + data.getPayload());
        std::size_t found = data.getPayload().find(id1);
        foundId1 = foundId1 || (found!=std::string::npos);
        found = data.getPayload().find(id2);
        foundId2 = foundId2 || (found!=std::string::npos);
    }
    EXPECT_TRUE(foundId1 && foundId2) << "Expect that a the ID's of the wings are published on startup";

    sut.stop();
}

TEST(WingsHandler,CalibrationWing ){ 
    Log::Init();
    int windowLength = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength,motionManager);
    auto wingStatusPublisher = std::make_shared<WingStatusPublisher>("id");

    std::vector<std::string> postedPayloads; 
    
    auto wing = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(),wingStatusPublisher);
    WingsHandler sut("dummyId", std::make_shared<WingInputTranslator>());
    sut.addWing(wing);
    sut.start();

    int counter=0;
    int maxCount=40;
    auto fakeCalibProcedure= std::async(std::launch::async, [& counter, & maxCount, & motionManager]( ){
        bool openIsFaked = false;
    
        while(true) {
            counter++;

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
            LOG_DEBUG("COUNTER:" + std::to_string(counter));
            if(!openIsFaked && motionManager->verifyCommandCalled("open",1)) {
                LOG_DEBUG("Open was called on masterWindow");
                std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // sleep to allow for open
                motionManager->SetFakeMotorStatus(MotorStatus::Open);
                openIsFaked=true;
                LOG_DEBUG("Fully open was faked");
                break;
            }          
            if (counter >= maxCount) {
                LOG_ERROR("Failed to calibrate in time");
                break;  // should not take 2 seconds to simulate a calibration with only one motor (and a passive one)
            }
        }
    });

    wing->calibrateOpen();
    fakeCalibProcedure.get();
    LOG_DEBUG("done calibration");
    sut.stop();
    MqttData data;
    EXPECT_TRUE(sut.getOutputBuffer()->GetSize() > 0) << "Size of the outputbuffer of the handler should contain messages to publish"; 
     
     bool foundStr = false;
    while(sut.getOutputBuffer()->UnqueueMessage(data)) {
        LOG_DEBUG("Payload" + data.getPayload());
        std::size_t found = data.getPayload().find("calibrateOpenFinishedSuccess");
        foundStr = (found!=std::string::npos);
    } 
    EXPECT_TRUE(foundStr) << "Expect that a success was published";
    
}



TEST(WingsHandler,calibFromMessage ){ 
    Log::Init();
    int windowLength = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength,motionManager);
    auto wingStatusPublisher = std::make_shared<WingStatusPublisher>("id");

    std::vector<std::string> postedPayloads; 
    
    auto wing = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(),wingStatusPublisher);
    WingsHandler sut("dummyId", std::make_shared<WingInputTranslator>());
    auto id = sut.addWing(wing);
    wing->clearCalibration();
    sut.start();
    

    int counter; int maxCount;
    auto fakeCalibProcedure = [& counter, & maxCount, & motionManager](){
        LOG_DEBUG("Started fake calibration");
        counter=0;
        maxCount=80;
        bool openIsFaked = false;
        bool closeIsFaked = false;
        motionManager->clearCommandBuffer();
        while(true) {
            counter++;

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
            LOG_DEBUG("COUNTER:" + std::to_string(counter));
            if(!openIsFaked && motionManager->verifyCommandCalled("open",1)) {
                LOG_DEBUG("Open was called on masterWindow");
                std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // sleep to allow for open
                motionManager->SetFakeMotorStatus(MotorStatus::Open);
                openIsFaked=true;
                LOG_DEBUG("Fully open was faked");
                std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // all slaves needs to be started before checks are done by the masterwindow -> give time

            }
            if ( !closeIsFaked && motionManager->verifyCommandCalled("close",1)) {
                LOG_DEBUG("Close was called on masterWindow");
                std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // sleep to allow for close
                motionManager->SetFakeMotorStatus(MotorStatus::Closed);
                LOG_DEBUG("Fully closed was faked");
                closeIsFaked = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(5002)); // all slaves needs to be started before checks are done by the masterwindow -> give time
                break;
            }
            if (counter >= maxCount) {
                break;  // fallback to be sure that there is a way out!
            }
        }
    };
    auto fakeCalibProcedure_fut1 = std::async(std::launch::async, fakeCalibProcedure);
    MqttData data_startCalib("systemcontroller/dummyConfigId/wing/"+ id +"/calibrate");
    sut.handleNewInput(std::move(data_startCalib));

    EXPECT_FALSE(wing->isCalibrated()) << "Handle input should not wait on calibration";

    fakeCalibProcedure_fut1.get();
    wing->waitOnCalibration();

    LOG_DEBUG("done calibration");
    sut.stop();
    MqttData data;
    EXPECT_TRUE(sut.getOutputBuffer()->GetSize() > 0) << "Size of the outputbuffer of the handler should contain messages to publish"; 
     
    bool foundStr = false;
    while(sut.getOutputBuffer()->UnqueueMessage(data)) {
        LOG_DEBUG("Payload" + data.getPayload());
        std::size_t found = data.getPayload().find("calibrateOpenFinishedSuccess");
        foundStr =foundStr || (found!=std::string::npos);
    } 
    EXPECT_TRUE(foundStr) << "Expect that a success was published";

    
}