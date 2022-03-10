#include <gtest/gtest.h>


#include "testMotorMotionManager.h"
#include "log.h"




TEST(motorMotionManagerTests,OnCalibratedHandlerTest ) {
    Log::Init();
    auto sut = TestMotorMotionManager(0);
    
    int onCalibCalledCount =0;
    sut.addOnMotorCalibratedhandler([&]() {
        onCalibCalledCount++;
    });

    MotorStatusData fakeMotorData;    
    sut.SetFakeMotorStatusData(fakeMotorData);
    fakeMotorData.isCalibrated = true;
    sut.SetFakeMotorStatusData(fakeMotorData);

    EXPECT_EQ(onCalibCalledCount,0) << "No stroke set yet, so no calibcompleted";

    sut.ManipulateStroke(2000); // due calibration a stroke is known
    sut.SetFakeMotorStatusData(fakeMotorData); // trigger a new status update 

    EXPECT_EQ(onCalibCalledCount,1) << "Stoke and Calib are ok, on calib should be called once";

    
    sut.SetFakeMotorStatus(MotorStatus::Closed); // trigger a new status update 
    sut.SetFakeMotorStatusData(fakeMotorData); // trigger a new status update 
    EXPECT_EQ(onCalibCalledCount,1) << "Stoke and Calib are ok, on calib should be called once and NOT MORE THAN ONCE";
}