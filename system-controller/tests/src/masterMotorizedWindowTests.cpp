#include <gtest/gtest.h>
#include <memory>
#include <future>

#include "motorData.h"
#include "masterMotorizedWindow.h"
#include "motorizedWindow.h"
#include "testMotorMotionManager.h"
#include "log.h"


TEST(masterMotorizedWindow,basics ){
    Log::Init();
    auto motionManager = std::make_shared<TestMotorMotionManager>(2000);
    auto sut = std::make_shared<MasterMotorizedWindow>(2000,motionManager);
    EXPECT_EQ(0,sut->getPosition()) << "Motorized window should initialize fully closed (pos=0)";
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    sut->setTarget(2000); // target to open window 
    motionManager->updateWithFakePosition(10);  
    // setTarget and updatePosition will call setPosition  
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setPosition"),2) 
        << "setposition on the motion manager should be called because of the target to open";
    EXPECT_EQ(10,sut->getPosition()) << "Motorized window should be pushed open ";
    
    motionManager->updateWithFakePosition(20);  
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setPosition"),3) 
    << "setposition on the motion manager should be called again because the position was updated";
    motionManager->updateWithFakePosition(30);    
    EXPECT_EQ(30,sut->getPosition()) << "Motorized window should be pushed open ";     
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setPosition"),4) 
        << "setposition on the motion manager should be called again because the position was updated";
}

TEST (masterMotorizedWindow,setTarget) {
    Log::Init();
    int stroke = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(stroke);
    MotorStatusData fakeStatus;
    fakeStatus.isCalibrated=true;
    motionManager->SetFakeMotorStatusData(fakeStatus) ;
    auto sut = std::make_shared<MasterMotorizedWindow>(stroke,motionManager);
    sut->setTarget(stroke + 500) ;
    EXPECT_LE(sut->getTarget(),stroke) << "Stroke should be the maximum target value";

   int nextTarget = stroke - 500;
   sut->setTarget(nextTarget);
   EXPECT_EQ(sut->getTarget(),nextTarget) << "When target is smaller than stroke, setting should be without change";

   sut->setTarget(-22); //negative target
   EXPECT_TRUE(sut->getTarget() == 0) << "Target should always be positive";
}

 TEST(masterMotorizedWindow,passiveSlaves ){
     Log::Init();
    int windowLength  = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength * 2 );
    auto sut = std::make_shared<MasterMotorizedWindow>(windowLength ,motionManager);
    auto slave = std::make_shared<PassiveWindow>(windowLength);
    sut->addSlave(slave,SlaveType::Passive);

    EXPECT_EQ(0,sut->getPosition()) << "Motorized window should initialize fully closed (pos=0)";
    EXPECT_EQ(0,slave->getPosition()) << "Passive slave window should initialize fully closed (pos=0)";
    
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    sut->setTarget(2*windowLength); // target to open window including the passive slave
    motionManager->updateWithFakePosition(10);   
    EXPECT_EQ(0,slave->getPosition()) <<  "Passive slave is not yet touched";

    int passedDistance = 500;    
    motionManager->updateWithFakePosition(windowLength + passedDistance); 
    EXPECT_EQ(windowLength + passedDistance,sut->getPosition()) << "Motorized window should opened passed length";
    EXPECT_EQ(passedDistance,slave->getPosition()) <<  "Passive slave should be pushed open also";
    // push even further
    motionManager->updateWithFakePosition(windowLength + 2* passedDistance); 
    EXPECT_EQ(2* passedDistance,slave->getPosition()) <<  "Passive slave should be pushed further";
    
    sut->stopWindow();
    bool isCalledCorrect = std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("stop",1);
    EXPECT_TRUE(isCalledCorrect) << "The stop of the motionmanager should be called when stopping the window";

    sut->setTarget(0); // target to close window including the passive slave
    // pull back
    motionManager->updateWithFakePosition(windowLength + passedDistance); 
    EXPECT_EQ(2* passedDistance,slave->getPosition()) <<  "Passive slave should not be pulled yet";
    // pull further back
    motionManager->updateWithFakePosition(passedDistance*2 + 100); 
    EXPECT_EQ(2* passedDistance,slave->getPosition()) <<  "Passive slave should not be pulled yet";
    // catch passive window
    motionManager->updateWithFakePosition(passedDistance); 
    EXPECT_EQ(passedDistance,slave->getPosition()) << "Passive slave should be pulled";
    // pull closed
    motionManager->updateWithFakePosition(0); // this will result in arriving on target!

    isCalledCorrect = std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("stop",2);
    EXPECT_TRUE(isCalledCorrect) << "When on target a stop should be called";
}


 TEST(masterMotorizedWindow,calibrateClose ){
    Log::Init();
    int windowLength  = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto slaveMotionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->clearCommandBuffer();
    auto sut = std::make_shared<MasterMotorizedWindow>(windowLength ,motionManager);
    auto slaveMotor = std::make_shared<MotorizedWindow>(windowLength, slaveMotionManager);
    sut->addSlave(slaveMotor,SlaveType::Motor);
   
    auto isClosed = sut->calibrateClose(); // async call 

    // slave-distance = 100 , lowspeed = 30 => 3.333 sec between slaves => 2 slavelevels  -> 6.66 sec
    std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    LOG_DEBUG("Set fake close");
    motionManager->SetFakeMotorStatus(MotorStatus::Closed);
    
    // block main thread and wait on completed calibration
    bool calibrationDone = isClosed.get();
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("stop"),1) << "stop before calibration";
    EXPECT_TRUE(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("stop")>1) << "stop before calibration";

    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setLowSpeed"),1) << "Everything should be in slow speed";
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("setLowSpeed"),1) << "Slave should be instructed to move";

    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("close"),1) << "Master should be instructed to move closing";
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("close"),1) << "Slave should be instructed to move closing";

    EXPECT_TRUE(calibrationDone) << "Expect a successfull calibration";

 }

TEST(masterMotorizedWindow,calibrateOpen ){
    Log::Init();
    int windowLength  = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto slaveMotionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->clearCommandBuffer();
    auto sut = std::make_shared<MasterMotorizedWindow>(windowLength ,motionManager);
    auto slaveMotor = std::make_shared<MotorizedWindow>(windowLength, slaveMotionManager);
    sut->addSlave(slaveMotor,SlaveType::Motor);
    // force open to start calibration mode 
    auto isOpened = sut->calibrateOpen(); // async call 
    // slave-distance = 100 , lowspeed = 30 => 3.333 sec between slaves => 2 slavelevels  -> 6.66 sec
    std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    LOG_DEBUG("Set fake open");
    motionManager->SetFakeMotorStatus(MotorStatus::Open);
    
    bool calibrationDone = isOpened.get();

    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("stop"),1) << "stop before calibration";
    EXPECT_TRUE(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("stop")>1) << "stop before calibration";

    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setLowSpeed"),1) << "Everything should be in slow speed";
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("setLowSpeed"),1) << "Slave should be instructed to move";

    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("open"),1) << "Master should be instructed to move closing";
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("open"),1) << "Slave should be instructed to move closing";

    EXPECT_TRUE(calibrationDone) << "Expect a successfull calibration";
 }


 TEST(masterMotorizedWindow,calibrateOpenTimeout ){
    Log::Init();
    bool skipThisTest = true;
    if (skipThisTest) {
        LOG_WARNING("long running test -masterMotorizeWindow/calibrateOpenTimeout- disabled because of realy long during test" );
    }
        else {
        int windowLength  = 2000;
        auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
        auto slaveMotionManager = std::make_shared<TestMotorMotionManager>(windowLength);
        std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
        std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->clearCommandBuffer();
        auto sut = std::make_shared<MasterMotorizedWindow>(windowLength ,motionManager);
        auto slaveMotor = std::make_shared<MotorizedWindow>(windowLength, slaveMotionManager);
        sut->addSlave(slaveMotor,SlaveType::Motor);
        // force open to start calibration mode 
        auto isOpened = sut->calibrateOpen(); // async call 
        int timeout = (sut->maximumStrokeForCalibration / motionManager->getLowSpeed()) + 2;
        LOG_DEBUG("Wait for " + std::to_string(timeout) + " seconds before timeout");
        std::this_thread::sleep_for(std::chrono::seconds(timeout));
        LOG_DEBUG("Should fail to end calibration due to timeout");
        bool calibrationDone = isOpened.get();

        EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("stop"),1) << "stop before calibration";
        EXPECT_TRUE(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("stop")>1) << "stop before calibration";

        EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setLowSpeed"),1) << "Everything should be in slow speed";
        EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("setLowSpeed"),1) << "Slave should be instructed to move";

        EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("open"),1) << "Master should be instructed to move closing";
        EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("open"),1) << "Slave should be instructed to move closing";

        EXPECT_FALSE(calibrationDone) << "Expect a failed calibration due timeout";
    }
 }


 TEST(masterMotorizedWindow,calibrateCloseCancel ){
    Log::Init();
    int windowLength  = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto slaveMotionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->clearCommandBuffer();
    auto sut = std::make_shared<MasterMotorizedWindow>(windowLength ,motionManager);
    auto slaveMotor = std::make_shared<MotorizedWindow>(windowLength, slaveMotionManager);
    sut->addSlave(slaveMotor,SlaveType::Motor);

    auto calibratedClose = sut->calibrateClose();
    // slave-distance = 100 , lowspeed = 30 => 3.333 sec between slaves => 2 slavelevels  -> 6.66 sec
    // Don't give enough time to complete calibration -> cancel will be called 
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    sut->stop();  
    bool isCalibrated = calibratedClose.get();  
    
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setLowSpeed"),1) << "Everything should be in slow speed";
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("close"),1) << "Master should be instructed to move closing";

    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("stop"),2) << "Master should be instructed to stop due cancel, and was stopped before even started";
    EXPECT_TRUE(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("stop")>2) << "Slave should be instructed to stop due cancel, and was stopped before even started";

    EXPECT_FALSE(isCalibrated) << "Expect a failed calibration";
 }

 TEST(masterMotorizedWindow,calibrateOpenCancel ){
    Log::Init();
    int windowLength  = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto slaveMotionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->clearCommandBuffer();
    auto sut = std::make_shared<MasterMotorizedWindow>(windowLength ,motionManager);
    auto slaveMotor = std::make_shared<MotorizedWindow>(windowLength, slaveMotionManager);
    sut->addSlave(slaveMotor,SlaveType::Motor);

    auto calibratedOpen = sut->calibrateOpen();
    // slave-distance = 100 , lowspeed = 30 => 3.333 sec between slaves => 2 slavelevels  -> 6.66 sec
    // Don't give enough time to complete calibration -> cancel will be called 
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    sut->stop();  
    bool isCalibrated = calibratedOpen.get();  
    
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setLowSpeed"),1) << "Everything should be in slow speed";
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("open"),1) << "Master should be instructed to move opening";

    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("stop"),2) << "Master should be instructed to stop due cancel, and was stopped before even started";
    EXPECT_TRUE(std::dynamic_pointer_cast<Verifier>(slaveMotionManager)->verifyCommandCalled("stop")>2) << "Slave should be instructed to stop due cancel, and was stopped before even started";

    EXPECT_FALSE(isCalibrated) << "Expect a failed calibration";
 }

TEST(masterMotorizedWindow,calibratCloseNoSlaves ){
    Log::Init();
    int windowLength  = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    auto sut = std::make_shared<MasterMotorizedWindow>(windowLength ,motionManager);
    
    auto calibrateClose = sut->calibrateClose();
    // slave-distance = 100 , lowspeed = 30 => 3.333 sec between slaves => 0 slavelevels  -> # loop seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    LOG_DEBUG("Set fake close");
    motionManager->SetFakeMotorStatus(MotorStatus::Closed);
    bool calibrationDone = calibrateClose.get();

    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("stop"),1) << "stop before calibration";
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setLowSpeed"),1) << "Everything should be in slow speed";
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("close"),1) << "Master should be instructed to move close";
    EXPECT_TRUE(calibrationDone) << "Expect a successfull calibration";
 }

TEST(masterMotorizedWindow,calibratOpenNoSlaves ){
    Log::Init();
    int windowLength  = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    auto sut = std::make_shared<MasterMotorizedWindow>(windowLength ,motionManager);
    
    auto calibrateOpen = sut->calibrateOpen();
    // slave-distance = 100 , lowspeed = 30 => 3.333 sec between slaves => 0 slavelevels  -> # loop seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    LOG_DEBUG("Set fake open");
    motionManager->SetFakeMotorStatus(MotorStatus::Open);
    bool calibrationDone = calibrateOpen.get();

    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("stop"),1) << "stop before calibration";
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("setLowSpeed"),1) << "Everything should be in slow speed";
    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(motionManager)->verifyCommandCalled("open"),1) << "Master should be instructed to move open";
    EXPECT_TRUE(calibrationDone) << "Expect a successfull calibration";
 }