#include <gtest/gtest.h>

#include "passiveWindow.h"
#include "motorizedWindow.h"
#include "testMotorMotionManager.h"
#include "log.h"

TEST(movingWindowTests,updatePanelLengthOnPassiveTest ){
    Log::Init();
    // no length is given -> update will be called when calibrated 
    auto sut = std::make_shared<PassiveWindow>(0);
    int newPanelLength = 3333;
    sut->updateAllPanelLenghts(newPanelLength);
    ASSERT_EQ(sut->getLength(), newPanelLength) << "PanelLength of passive Should be updated";

    sut->updateAllPanelLenghts(66666);
    ASSERT_EQ(sut->getLength(), newPanelLength) << "PanelLength of passive Should still be the same ";
}
TEST(movingWindowTests,updatePanelLengthOnMotorizedTest ){
    Log::Init();
    // no length is given -> update will be called when calibrated 
    auto sut = std::make_shared<MotorizedWindow>(0,std::make_shared<TestMotorMotionManager>(0));
    int newPanelLength = 3333;
    sut->updateAllPanelLenghts(newPanelLength);
    ASSERT_EQ(sut->getLength(), newPanelLength) << "PanelLength of Motorized Should be updated";

    sut->updateAllPanelLenghts(66666);
    ASSERT_EQ(sut->getLength(), newPanelLength) << "PanelLength of Motorized Should still be the same ";
}

TEST(movingWindowTests,updatePanelLengthWithSlavesTest ){
    Log::Init();
    int panelLenghtSlave2 = 1000;
    // no length is given -> update will be called when calibrated 
    auto sut = std::make_shared<MotorizedWindow>(0,std::make_shared<TestMotorMotionManager>(0));
    auto slave1 = std::make_shared<PassiveWindow>(0);
    auto slave2 = std::make_shared<PassiveWindow>(panelLenghtSlave2);
    slave1->addSlave(slave2,SlaveType::Passive);
    sut->addSlave(slave1,SlaveType::Passive);

    int newPanelLength = 3333;
    sut->updateAllPanelLenghts(newPanelLength);
    ASSERT_EQ(sut->getLength(), newPanelLength) << "SUT should have a new PanelLength";
    ASSERT_EQ(slave1->getLength(), newPanelLength) << "Slave1 should have a new PanelLength";
    ASSERT_EQ(slave2->getLength(), panelLenghtSlave2) << "Slave2 should still have its current panelLength";
    ASSERT_FALSE(slave2->getLength()== newPanelLength) << "Slave2 should NOT have a new PanelLength";
}