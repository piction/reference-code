#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include "log.h"
#include "windowPushZone.h"

TEST(windowPushZone,checkMinimum ){
    Log::Init();
    WindowPushZone sut;
    EXPECT_TRUE(sut.isInZone(10)) << "No zone set so should be in Zone";
    EXPECT_TRUE(sut.getMinOpen() <0) << "nothing set so min should be below zero";
    EXPECT_TRUE(sut.getMaxOpen() > 100000) << "nothing set so max should be super big";
    EXPECT_FALSE(sut.isActive()) << "nothing set so should be inactive";

    int testMin = 100;
    sut.setMinOpening(testMin);
    EXPECT_TRUE(sut.isActive()) << "Min filter is active";
    EXPECT_TRUE(sut.shouldOpen(testMin - 1)) << "Should go open to be above min of zone";
    EXPECT_TRUE(sut.isInZone(testMin + 1)) << "Should be above min of zone";
    EXPECT_TRUE(sut.isInZone(testMin)) << "Should be on min of zone";
    EXPECT_FALSE(sut.isInZone(testMin - 42)) << "Should be out of zone due to low";
    EXPECT_EQ(sut.getMinOpen(),testMin) << "Should return min that was set";    
    EXPECT_FALSE(sut.shouldClose(testMin + 10 )) << "Should not close, no limit on max of zone";
    EXPECT_FALSE(sut.shouldClose(testMin - 10 )) << "Should not close, no limit on max of zone";

    sut.inActivate();
    EXPECT_FALSE(sut.isActive()) << "incactivate was called so should be set inactive";
    EXPECT_TRUE(sut.isInZone(testMin - 42)) << "There should be no minimum, deacitvated";
    EXPECT_TRUE(sut.isInZone(testMin + 42)) << "There should be no minimum, deacitvated";
    EXPECT_TRUE(sut.getMinOpen() != testMin) << "Minimum should be removed";

    int newMin = 50;
    sut.setMinOpening(newMin);
    EXPECT_TRUE(sut.isInZone(testMin-1)) << "Should be in NEW zone";
    EXPECT_FALSE(sut.isInZone(newMin-1)) << "Should out of NEW zone";
}

TEST(windowPushZone,checkInactivate ){
    Log::Init();
    WindowPushZone sut;
    int maxOpening = 1000; int minOpening = 10;
    sut.setMaxOpening(maxOpening);
    sut.setMinOpening(minOpening);
    EXPECT_EQ(sut.getMaxOpen() , maxOpening) << "max opening should be set correct";
    EXPECT_EQ(sut.getMinOpen() , minOpening) << "min opening should be set correct";

    sut.inActivate();
    EXPECT_FALSE(sut.isActive()) << "After inactivate zone should be inactive";
    EXPECT_TRUE(sut.getMaxOpen() > maxOpening) << "once inactivated max should be MAX integer";
    EXPECT_TRUE(sut.getMinOpen() < minOpening) << "once inactivated min should be set negative or zero";


}
TEST(windowPushZone,checkMaximum ){
    
    WindowPushZone sut;
    EXPECT_TRUE(sut.isInZone(10)) << "No zone set so should be in Zone";
    int testMax = 100;
    sut.setMaxOpening(testMax);
    EXPECT_TRUE(sut.isActive()) << "Max filter is active";
    EXPECT_TRUE(sut.isInZone(testMax - 1)) << "Should be belowo max min of zone";
    EXPECT_TRUE(sut.isInZone(testMax)) << "Should be on max of zone";
    EXPECT_FALSE(sut.isInZone(testMax + 42)) << "Should be out of zone due to high";
    EXPECT_EQ(sut.getMaxOpen(),testMax) << "Should return min that was set";
    EXPECT_TRUE(sut.shouldClose(testMax + 1)) << "Should close to be below max of zone";
    EXPECT_FALSE(sut.shouldOpen(testMax - 10 )) << "Should not open, no limit on max of zone";
    EXPECT_FALSE(sut.shouldOpen(testMax + 10 )) << "Should not open, no limit on max of zone";

    sut.inActivate();
    EXPECT_TRUE(sut.isInZone(testMax + 42)) << "There should be no maximum, deacitvated";
    EXPECT_TRUE(sut.isInZone(testMax - 42)) << "There should be no maximum, deacitvated";
    EXPECT_TRUE(sut.getMinOpen() != testMax) << "Maximum should be removed";

    int newMax = 150;
    sut.setMaxOpening(newMax);
    EXPECT_TRUE(sut.isInZone(testMax-1)) << "Should be in NEW zone";
    EXPECT_FALSE(sut.isInZone(newMax+1)) << "Should out of NEW zone";
}

TEST(windowPushZone,rangeCheck ){
    
    WindowPushZone sut;
    int testMax = 1000;
    int testMin = 10;
    int stepSize = 5;
    sut.setMaxOpening(testMax);
    sut.setMinOpening(testMin);
    EXPECT_TRUE(sut.isActive()) << "Filters are active";
    EXPECT_TRUE(sut.isInZone(testMax - stepSize)) << "Should be belowo max min of zone";
    EXPECT_TRUE(sut.isInZone(testMax)) << "Should be on max of zone";
    EXPECT_FALSE(sut.isInZone(testMax + stepSize)) << "Should be out of zone due to high";
    EXPECT_EQ(sut.getMaxOpen(),testMax) << "Should return min that was set";
    EXPECT_TRUE(sut.shouldClose(testMax + stepSize)) << "Should close to be below max of zone";
    EXPECT_FALSE(sut.shouldOpen(testMax - stepSize )) << "Should not open, no limit on max of zone";
    EXPECT_FALSE(sut.shouldOpen(testMax + stepSize )) << "Should not open, no limit on max of zone";

    EXPECT_TRUE(sut.shouldOpen(testMin - stepSize)) << "Should go open to be above min of zone";
    EXPECT_TRUE(sut.isInZone(testMin + stepSize)) << "Should be above min of zone";
    EXPECT_TRUE(sut.isInZone(testMin)) << "Should be on min of zone";
    EXPECT_FALSE(sut.isInZone(testMin - stepSize)) << "Should be out of zone due to low";
    EXPECT_EQ(sut.getMinOpen(),testMin) << "Should return min that was set";    
    EXPECT_FALSE(sut.shouldClose(testMin + stepSize )) << "Should not close, no limit on max of zone";
    EXPECT_FALSE(sut.shouldClose(testMin - stepSize )) << "Should not close, no limit on max of zone";

    sut.inActivate();
    EXPECT_TRUE(sut.isInZone(testMin - stepSize)) << "There should be no minimum, deacitvated";
    EXPECT_TRUE(sut.isInZone(testMin + stepSize)) << "There should be no minimum, deacitvated";
    EXPECT_TRUE(sut.getMinOpen() != testMin) << "Minimum should be removed";
    EXPECT_TRUE(sut.isInZone(testMax + stepSize)) << "There should be no maximum, deacitvated";
    EXPECT_TRUE(sut.isInZone(testMax - stepSize)) << "There should be no maximum, deacitvated";
    EXPECT_TRUE(sut.getMinOpen() != testMax) << "Maximum should be removed";


}

TEST(windowPushZone, copyCheck) {

    std::shared_ptr<IWindowPushZone> sut = std::make_shared<WindowPushZone>();
    sut->setMaxOpening(100);
    int currentRefCount = sut.use_count();
    WindowPushZone cpy(sut);

    EXPECT_EQ(currentRefCount,sut.use_count()) << "copy should not create extra reference";
    EXPECT_TRUE(cpy.getMinOpen()==sut->getMinOpen());
    EXPECT_TRUE(cpy.getMaxOpen()==sut->getMaxOpen());
    sut->inActivate();
    EXPECT_FALSE(cpy.getMaxOpen()==sut->getMaxOpen()) << "original should be loose from copy";


    sut->setMinOpening(300);
    WindowPushZone cpy2(sut);
    EXPECT_EQ(currentRefCount,sut.use_count()) << "copy should not create extra reference";
    EXPECT_TRUE(cpy2.getMinOpen()==sut->getMinOpen());
    EXPECT_TRUE(cpy2.getMaxOpen()==sut->getMaxOpen());


    // copy from pointer to pointer
    auto check2 = std::make_shared<WindowPushZone> (sut);
    EXPECT_TRUE(check2->getMinOpen()==sut->getMinOpen());
    EXPECT_TRUE(check2->getMaxOpen()==sut->getMaxOpen());
    check2->setMaxOpening(1000);
    EXPECT_FALSE(check2->getMaxOpen()==sut->getMaxOpen());

    // copy non active 
    std::shared_ptr<IWindowPushZone> sut_inactive = std::make_shared<WindowPushZone>();
    WindowPushZone cpy_incactive(sut_inactive);
    EXPECT_FALSE(sut_inactive->isActive()) << "Noting was set on inactive";
    EXPECT_FALSE(cpy_incactive.isActive()) << "Noting was set on inactive so don't create when copy";
}

TEST(windowPushZone, equalCheck) {

    std::shared_ptr<IWindowPushZone> sut1 = std::make_shared<WindowPushZone>();
    sut1->setMaxOpening(100);
    WindowPushZone cpy(sut1);
    EXPECT_TRUE(cpy.isEqual(sut1)) << "sopy Should be equal zones";
    

    sut1->inActivate();
    EXPECT_FALSE(cpy.isEqual(sut1)) << "Should not be equal zones because one is set to inactive";
    std::shared_ptr<IWindowPushZone> sut2 = std::make_shared<WindowPushZone>();
    sut2->setMaxOpening(100);   sut1->setMaxOpening(100);
    sut2->setMinOpening(20);    sut1->setMinOpening(20); 
    EXPECT_TRUE(sut2->isEqual(sut1)) << "Should be equal zones";
    EXPECT_TRUE(sut1->isEqual(sut2)) << "Should be equal zones";
}