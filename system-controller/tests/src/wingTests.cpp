#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include <future>


#include "log.h"
#include "wing.h"
#include "motorizedWindow.h"
#include "testMotorMotionManager.h"
#include "testWingStatusPublisher.h"

TEST(WingTests,checkIfCalibratedBeforeTriggerCmd ){
    Log::Init();
    int windowLength=2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(2000,motionManager);
      
    auto sut = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(),std::make_shared<TestWingStatusPublisher>());

    int startPos = sut->getPosition();
    EXPECT_TRUE(startPos == 0);

    // no actions should be performed 
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    sut->close();    
    EXPECT_EQ( std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("close"),0);
    sut->open();
    EXPECT_EQ( std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("open"),0);
    sut->setPositionMm(100);
    EXPECT_EQ( std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setPosition"),0);
    sut->setPositionPerc(1001);
    EXPECT_EQ( std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setPosition"),0);
}


TEST(WingTests,listMotors ){
    Log::Init();
    int windowLength = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength,motionManager);
    
    auto slave1=std::make_shared<PassiveWindow>(windowLength);
    auto slave2=std::make_shared<MotorizedWindow>(windowLength,motionManager);
    auto slave3=std::make_shared<PassiveWindow>(windowLength);
    auto slave4=std::make_shared<MotorizedWindow>(windowLength,motionManager);
    auto slave5=std::make_shared<MotorizedWindow>(windowLength,motionManager);

    slave4->addSlave(slave5,SlaveType::Motor);
    slave3->addSlave(slave4,SlaveType::Motor);
    slave2->addSlave(slave3,SlaveType::Passive);
    slave1->addSlave(slave2, SlaveType::Motor);
    motorizedWindow->addSlave(slave1,SlaveType::Passive);
    
    // *******************************
    // created on master with all slaves linked
    // ******************************* 
    
    auto sut = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(), std::make_shared<TestWingStatusPublisher>());

    std::vector<std::shared_ptr<MotorizedWindow>> motors = sut->getMotors();

    EXPECT_EQ(motors.size(),4) << "The wing should have 4 motors";
    EXPECT_TRUE (find (motors.begin(), motors.end(),motorizedWindow) != motors.end()) << "Master should be one of the motors";
    EXPECT_TRUE (find (motors.begin(), motors.end(),slave2) != motors.end()) << "slave2 should be one of the motors";
    EXPECT_TRUE (find (motors.begin(), motors.end(),slave4) != motors.end()) << "slave4 should be one of the motors";
    EXPECT_TRUE (find (motors.begin(), motors.end(),slave5) != motors.end()) << "slave5 should be one of the motors";
}

TEST(WingTests,listMotorsOnWingWithNotFullyBuildMasterWindow ){
    Log::Init();
    int windowLength = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength,motionManager);
    
    auto slave1=std::make_shared<PassiveWindow>(windowLength);
    auto slave2=std::make_shared<MotorizedWindow>(windowLength,motionManager);
    auto slave3=std::make_shared<PassiveWindow>(windowLength);
    auto slave4=std::make_shared<MotorizedWindow>(windowLength,motionManager);
    auto slave5=std::make_shared<MotorizedWindow>(windowLength,motionManager);
    // *******************************
    // created before slaves are added
    // ******************************* 
    auto sut = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(), std::make_shared<TestWingStatusPublisher>());
    
    slave4->addSlave(slave5,SlaveType::Motor);
    slave3->addSlave(slave4,SlaveType::Motor);
    slave2->addSlave(slave3,SlaveType::Passive);
    slave1->addSlave(slave2, SlaveType::Motor);
    motorizedWindow->addSlave(slave1,SlaveType::Passive);

    std::vector<std::shared_ptr<MotorizedWindow>> motors = sut->getMotors();

    EXPECT_EQ(motors.size(),4) << "The wing should have 4 motors";
    EXPECT_TRUE (find (motors.begin(), motors.end(),motorizedWindow) != motors.end()) << "Master should be one of the motors";
    EXPECT_TRUE (find (motors.begin(), motors.end(),slave2) != motors.end()) << "slave2 should be one of the motors";
    EXPECT_TRUE (find (motors.begin(), motors.end(),slave4) != motors.end()) << "slave4 should be one of the motors";
    EXPECT_TRUE (find (motors.begin(), motors.end(),slave5) != motors.end()) << "slave5 should be one of the motors";
}

TEST(WingTests, addSiblings)
{
    Log::Init();
    auto fakePublisher = std::make_shared<TestWingStatusPublisher>();
    std::vector<std::shared_ptr<Wing>> wings;
    for (int i = 0; i < 4; i++) {
        int windowLength = 2000;
        auto motionManager = std::make_shared<TestMotorMotionManagerFakeCalibration>(windowLength);
        auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength, motionManager);
        auto w = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(), fakePublisher);
        wings.push_back(w);
    }

    std::shared_ptr<Wing>& sut = (wings[0]);
    EXPECT_EQ((int) (sut->getSiblings()->size()) ,0) << "No siblings added yet";
    sut->addSibling(wings[1], WingSiblingType::Opposite);
    EXPECT_EQ((int) (sut->getSiblings()->size()) ,1) << "One sibling should be added";
    sut->addSibling(wings[1], WingSiblingType::Opposite);
    EXPECT_EQ((int) (sut->getSiblings()->size()) ,1) << "One sibling should be added, not twice because of same id1";
    sut->addSibling(wings[2], WingSiblingType::Opposite);
    EXPECT_EQ((int) (sut->getSiblings()->size()) ,2) << "Two siblings should be added";
    Log::GetLogger()->flush();
}

TEST(WingTests,calibrateFromWingQX ){
        Log::Init();
        // TODO: calibrateFromWingQX
        LOG_ERROR ("TODO:calibrateFromWingQX")
}

TEST(WingTests,cancelCalibration ){
        Log::Init();
        // TODO: cancelCalibration
        LOG_ERROR ("TODO:cancelCalibration")
}
