#include <gtest/gtest.h>
#include <memory>

#include "passiveWindow.h"

TEST(passiveWindow,basics ){
    
    auto sut = std::make_shared<PassiveWindow>(2000);
    EXPECT_EQ(0,sut->getPosition()) << "Passive window should initialize fully closed (pos=0)";
    
    sut->setPositionDuePushingOrPulling(10);    
    EXPECT_EQ(10,sut->getPosition()) << "Passive window should be pushed open ";
    
    EXPECT_EQ(0,sut->getLastStandStillPosition()) << "From close position standstill";
    sut->setPositionDuePushingOrPulling(20);    
    sut->setPositionDuePushingOrPulling(30);    
    sut->setPositionDuePushingOrPulling(40);    
    // change direction !
    sut->setPositionDuePushingOrPulling(30);    
    EXPECT_EQ(40,sut->getLastStandStillPosition()) << "Standstill due change direction!";
}

TEST(passiveWindow,passiveSlaves ){
    int windowLength = 2000;
    auto sut = std::make_shared<PassiveWindow>(windowLength);
    auto slave = std::make_shared<PassiveWindow>(windowLength);
    sut->addSlave(slave,SlaveType::Passive);

    EXPECT_EQ(0,sut->getPosition()) << "Passive window should initialize fully closed (pos=0)";
    EXPECT_EQ(0,slave->getPosition()) << "Passive slave window should initialize fully closed (pos=0)";
    
    sut->setPositionDuePushingOrPulling(10);    
    EXPECT_EQ(10,sut->getPosition()) << "Passive window should be pushed open ";
    EXPECT_EQ(0,slave->getPosition()) << "Passive slave is not yet touched";

    EXPECT_EQ(0,sut->getLastStandStillPosition()) << "From start position standstill";
    EXPECT_EQ(0,slave->getLastStandStillPosition()) << "From slave start position standstill";

    int passedDistance = 500;    
    sut->setPositionDuePushingOrPulling(windowLength + passedDistance);    
    EXPECT_EQ(windowLength + passedDistance,sut->getPosition()) << "Passive window should be pushed open ";
    EXPECT_EQ(passedDistance,slave->getPosition()) << "Passive slave should be pushed open also";

    // push even further
    sut->setPositionDuePushingOrPulling(windowLength + 2*passedDistance);   
    EXPECT_EQ(0,sut->getLastStandStillPosition()) << "From start position standstill, not yet direction change";
    EXPECT_EQ(0,slave->getLastStandStillPosition()) << "From slave start position standstill, not yet direction change";

    
    sut->stopWindow();

    // pull back 
    sut->setPositionDuePushingOrPulling(windowLength + passedDistance);
    EXPECT_EQ(passedDistance*2,slave->getPosition()) << "Passive slave should not be pulled yet";
    
    sut->setPositionDuePushingOrPulling(passedDistance*2 + 100);
    EXPECT_EQ(passedDistance*2,slave->getPosition()) << "Passive slave should not be pulled yet";
    // pull even further
    sut->setPositionDuePushingOrPulling(passedDistance);
    EXPECT_EQ(passedDistance,slave->getPosition()) << "Passive slave should be pulled";
    // pull close
    sut->setPositionDuePushingOrPulling(0);
    EXPECT_EQ(0,slave->getPosition()) << "Passive slave should be closed";

    sut->stopWindow();
}
