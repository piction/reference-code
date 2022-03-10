#include <gtest/gtest.h>
#include <memory>
#include <iostream>
#include <fstream>
#include "log.h"
#include "utils.h"
#include "motorizedWindow.h"
#include "movingWindow.h"
#include "testMotorMotionManager.h"

TEST(motorizedWindow,basics ){
    Log::Init();
    auto motionManager = std::make_shared<TestMotorMotionManager>(2000);
    auto sut = std::make_shared<MotorizedWindow>(2000,motionManager);
    EXPECT_EQ(0,sut->getPosition()) << "Motorized window should initialize fully closed (pos=0)";
    
    motionManager->updateWithFakePosition(10);    
    EXPECT_EQ(10,sut->getPosition()) << "Motorized window should be pushed open ";
    
    motionManager->updateWithFakePosition(20);    
    motionManager->updateWithFakePosition(30);    
    EXPECT_EQ(30,sut->getPosition()) << "Motorized window should be pushed open ";
    sut->stopWindow();    
}

 TEST(motorizedWindow,passiveSlaves ){
    Log::Init();
    int windowLength  = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto sut = std::make_shared<MotorizedWindow>(windowLength ,motionManager);
    auto slave = std::make_shared<PassiveWindow>(windowLength);
    sut->addSlave(slave,SlaveType::Passive);

    EXPECT_EQ(0,sut->getPosition()) << "Motorized window should initialize fully closed (pos=0)";
    EXPECT_EQ(0,slave->getPosition()) << "Passive slave window should initialize fully closed (pos=0)";

    sut->push(PushType::PushToOpen);
    motionManager->updateWithFakePosition(20); 
    EXPECT_EQ(0,slave->getPosition()) <<  "Passive slave is not yet touched";

    int passedDistance = SystemSettings::getInstance().getSlowdownDist()/2;    
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    motionManager->updateWithFakePosition(windowLength + passedDistance); 
    EXPECT_EQ(windowLength + passedDistance,sut->getPosition()) << "Motorized window should opened passed length";
    EXPECT_EQ(passedDistance,slave->getPosition()) <<  "Passive slave should be pushed open also";
    EXPECT_TRUE(sut->getMovementFreedom()==MovementFreedom::Slow)  <<  "Passive slave should be pushed open also but slow";
    
    // push even further
    std::dynamic_pointer_cast<Verifier>(motionManager)->clearCommandBuffer();
    motionManager->updateWithFakePosition(windowLength + 3* passedDistance); 
    EXPECT_EQ(3* passedDistance,slave->getPosition()) <<  "Passive slave should be pushed further";
    EXPECT_TRUE(sut->getMovementFreedom()==MovementFreedom::Fast)  <<   "Expect to increase speed when passive is cached";
    sut->stopWindow();
    sut->push(PushType::PushToClose);
    // pull back
    motionManager->updateWithFakePosition(windowLength + passedDistance); 
    EXPECT_EQ(3* passedDistance,slave->getPosition()) <<  "Passive slave should not be pulled yet";
    // pull further back
    motionManager->updateWithFakePosition(passedDistance*3 + 100); 
    EXPECT_EQ(3* passedDistance,slave->getPosition()) <<  "Passive slave should not be pulled yet";
    // catch passive window
    motionManager->updateWithFakePosition(passedDistance); 
    EXPECT_EQ(passedDistance,slave->getPosition()) << "Passive slave should be pulled";
    // pull closed
    motionManager->updateWithFakePosition(0); 
    EXPECT_EQ(0,slave->getPosition()) << "Passive slave should be pulled close";
}

TEST ( motorizedWindow , GetLeastAllowedMovement) {
    EXPECT_EQ(MovingWindow::GetLeastAllowedMovement(MovementFreedom::None, MovementFreedom::Fast) , MovementFreedom::None) <<"None should be least allowd";
    EXPECT_EQ(MovingWindow::GetLeastAllowedMovement(MovementFreedom::None, MovementFreedom::Slow) , MovementFreedom::None) <<"None should be least allowd";
    EXPECT_EQ(MovingWindow::GetLeastAllowedMovement(MovementFreedom::Slow, MovementFreedom::None) , MovementFreedom::None) <<"None should be least allowd";
    EXPECT_EQ(MovingWindow::GetLeastAllowedMovement(MovementFreedom::Fast, MovementFreedom::None) , MovementFreedom::None) <<"None should be least allowd";
    EXPECT_EQ(MovingWindow::GetLeastAllowedMovement(MovementFreedom::None, MovementFreedom::None) , MovementFreedom::None) <<"None should be least allowd";
    EXPECT_EQ(MovingWindow::GetLeastAllowedMovement(MovementFreedom::Fast, MovementFreedom::Fast) , MovementFreedom::Fast) <<"Fast should be least allowd";
    EXPECT_EQ(MovingWindow::GetLeastAllowedMovement(MovementFreedom::Slow, MovementFreedom::Slow) , MovementFreedom::Slow) <<"Slow should be least allowd";
    EXPECT_EQ(MovingWindow::GetLeastAllowedMovement(MovementFreedom::Fast, MovementFreedom::Slow) , MovementFreedom::Slow) <<"Slow should be least allowd";
    EXPECT_EQ(MovingWindow::GetLeastAllowedMovement(MovementFreedom::Slow, MovementFreedom::Fast) , MovementFreedom::Slow) <<"Slow should be least allowd";
}

 TEST(motorizedWindow,LogPassiveSlaves ){
    Log::Init();
    int windowLength  = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto sut = std::make_shared<MotorizedWindow>(windowLength ,motionManager);
    auto slave1 = std::make_shared<PassiveWindow>(windowLength);
    auto slave2 = std::make_shared<PassiveWindow>(windowLength);
    slave1->addSlave(slave2,SlaveType::Passive);
    sut->addSlave(slave1,SlaveType::Passive);

    EXPECT_EQ(0,sut->getPosition()) << "Motorized window should initialize fully closed (pos=0)";
    EXPECT_EQ(0,slave1->getPosition()) << "Passive slave window should initialize fully closed (pos=0)";
    EXPECT_EQ(0,slave2->getPosition()) << "Passive slave window should initialize fully closed (pos=0)";

    sut->push(PushType::PushToOpen);
    std::ofstream logFile; 
    logFile.open(utils::getApplicationDirectory() + "/testData/SlaveOpenCloseTest.log");
    
    auto getMovmentType = [](MovementFreedom m) -> std::string {
        switch(m) {
            case MovementFreedom::None: return "none";
            case MovementFreedom::Slow: return "slow";
            case MovementFreedom::Fast: return "fast";
        };
        return "unknwon";
    };
    logFile<< "--- OPENING ---" << std::endl;
    for ( int i = 0; i< windowLength*3 ; i+=20)
    {
        motionManager->updateWithFakePosition(i);     
        logFile <<  sut->getPosition()<< ";" << getMovmentType(sut->getMovementFreedom()) << " - ";
        logFile <<  slave1->getPosition()<< ";" << getMovmentType(slave1->getMovementFreedom()) << " - ";
        logFile <<  slave2->getPosition()<< ";" << getMovmentType(slave2->getMovementFreedom()) << std::endl;
    }
    logFile<< "--- CLOSING ---" << std::endl;
    sut->push(PushType::PushToClose);
    for ( int i = windowLength*3; i>0 ; i-=20)
    {
        motionManager->updateWithFakePosition(i);     
        logFile <<  sut->getPosition()<< ";" << getMovmentType(sut->getMovementFreedom()) << " - ";
        logFile <<  slave1->getPosition()<<"["<< slave1->getLastStandStillPosition() << "];" << getMovmentType(slave1->getMovementFreedom()) << " - ";
        logFile <<  slave2->getPosition()<<"["<< slave2->getLastStandStillPosition() << "];" << getMovmentType(slave1->getMovementFreedom()) << std::endl;
    }


    logFile.close();
}

TEST(motorizedWindow,passiveSlavesNested ){
    Log::Init();
    int windowLength  = 2000;
    auto motionManager = std::make_shared<TestMotorMotionManager>(windowLength);
    auto sut = std::make_shared<MotorizedWindow>(windowLength ,motionManager);
    auto slaveOfMotor = std::make_shared<PassiveWindow>(windowLength);
    auto slaveOfPassive = std::make_shared<PassiveWindow>(windowLength);

    sut->addSlave(slaveOfMotor,SlaveType::Passive);
    slaveOfMotor->addSlave(slaveOfPassive,SlaveType::Passive);

    EXPECT_EQ(0,sut->getPosition()) << "Motorized window should initialize fully closed (pos=0)";
    EXPECT_EQ(0,slaveOfMotor->getPosition()) << "Passive slave of motor window should initialize fully closed (pos=0)";
    EXPECT_EQ(0,slaveOfPassive->getPosition()) << "Passive slave passive window should initialize fully closed (pos=0)";

    sut->push(PushType::PushToOpen);
    motionManager->updateWithFakePosition(20); 
    EXPECT_EQ(0,slaveOfMotor->getPosition()) <<  "Slaves is not yet touched";
    EXPECT_EQ(0,slaveOfPassive->getPosition()) <<  "Slaves is not yet touched";

    int passedDistance = 500; //one slave should open    
    motionManager->updateWithFakePosition(windowLength + passedDistance); 
    EXPECT_EQ(windowLength + passedDistance,sut->getPosition()) << "Motorized window should opened passed length";
    EXPECT_EQ(passedDistance,slaveOfMotor->getPosition()) <<  "Passive slave of motor should be pushed open also";
    EXPECT_EQ(0,slaveOfPassive->getPosition()) << "Passive slave passive window should remain closed";
    
    // push even further
    motionManager->updateWithFakePosition(2*windowLength +  passedDistance); 
    EXPECT_EQ(2*windowLength + passedDistance,sut->getPosition()) << "Motorized window should opened passed length";
    EXPECT_EQ(windowLength + passedDistance,slaveOfMotor->getPosition()) <<  "Passive slave of motor should be pushed open further";
    EXPECT_EQ(passedDistance,slaveOfPassive->getPosition()) << "Passive slave passive window should be pushed by slave of motor";
    
    sut->stopWindow();
    sut->push(PushType::PushToClose);
    
    // pull back
    motionManager->updateWithFakePosition(2*windowLength - passedDistance); 
    EXPECT_EQ(windowLength + passedDistance,slaveOfMotor->getPosition()) <<  "Passive slave of motor should not be pulled yet";
    EXPECT_EQ(passedDistance,slaveOfPassive->getPosition()) <<  "Passive slave of passive slave should not be pulled yet";
    // pull further back
    motionManager->updateWithFakePosition( passedDistance); 
    EXPECT_EQ(passedDistance,slaveOfMotor->getPosition()) <<  "Passive slave of motor should be pulled yet";
    EXPECT_EQ(passedDistance,slaveOfPassive->getPosition()) <<  "Passive slave of passive slave should not be pulled yet";
    // close
    motionManager->updateWithFakePosition( 0); 
    EXPECT_EQ(0,slaveOfMotor->getPosition()) <<  "Passive slave of motor should be closed";
    EXPECT_EQ(0,slaveOfPassive->getPosition()) <<  "Passive slave of passive slave should be closed";
}