#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <cmath>

#include "log.h"
#include "wing.h"
#include "motorizedWindow.h"
#include "testMotorMotionManager.h"
#include "testWingStatusPublisher.h"
#include "wingCalibrationHandler.h"

class wingCalibrationHandlerTestsProvider {
    public:


    // simulate (X)X-X(X)
    static  std::vector<std::shared_ptr<Wing>> getXx_xX(std::shared_ptr<IWingStatusPublisher> publisher) {

        std::vector<std::shared_ptr<Wing>> wings;
        for (int i=0;i<4;i++) {
            int windowLength = 2000;
            auto motionManager = std::make_shared<TestMotorMotionManagerFakeCalibration>(windowLength);
            auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength,motionManager);
            auto w = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(), publisher);
            wings.push_back(w);    
        }
        // simulate (X)X-X(X)

        wings[0]->addSibling(wings[1],WingSiblingType::Opposite);
        wings[1]->addSibling(wings[0],WingSiblingType::Opposite);
        wings[1]->addSibling(wings[2],WingSiblingType::MiddleFemale);
        wings[2]->addSibling(wings[1],WingSiblingType::MiddleMale);
        wings[2]->addSibling(wings[3],WingSiblingType::Opposite);
        wings[3]->addSibling(wings[2],WingSiblingType::Opposite);
        return wings;
    };


    // simulate QX-X(X)
    static  std::vector<std::shared_ptr<Wing>> getQx_xX(std::shared_ptr<IWingStatusPublisher> publisher) {

        std::vector<std::shared_ptr<Wing>> wings;
        for (int i=0;i<3;i++) {
            int windowLength = 2000;
            auto motionManager = std::make_shared<TestMotorMotionManagerFakeCalibration>(windowLength);
            auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength,motionManager);
            auto w = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(), publisher);
            wings.push_back(w);    
        }
        // simulate QX-X(X)

        wings[0]->addSibling(wings[1],WingSiblingType::MiddleFemale);
        wings[1]->addSibling(wings[0],WingSiblingType::MiddleMale);
        wings[1]->addSibling(wings[2],WingSiblingType::Opposite);
        wings[2]->addSibling(wings[1],WingSiblingType::Opposite);
        return wings;
    };

        // simulate QX
    static  std::vector<std::shared_ptr<Wing>> getQx(std::shared_ptr<IWingStatusPublisher> publisher) {

        std::vector<std::shared_ptr<Wing>> wings;
        for (int i=0;i<1;i++) {
            int windowLength = 2000;
            auto motionManager = std::make_shared<TestMotorMotionManagerFakeCalibration>(windowLength);
            auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength,motionManager);
            auto w = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(), publisher);
            wings.push_back(w);    
        }
        // simulate QX
        return wings;
    };

    // simulate QXvXQ
    static  std::vector<std::shared_ptr<Wing>> getQxvXQ(std::shared_ptr<IWingStatusPublisher> publisher) {
     
        std::vector<std::shared_ptr<Wing>> wings;
        for (int i=0;i<2;i++) {
            int windowLength = 2000;
            auto motionManager = std::make_shared<TestMotorMotionManagerFakeCalibration>(windowLength);
            auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength,motionManager);
            auto w = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(), publisher);
            wings.push_back(w);    
        }
        // simulate QXvXQ

        wings[0]->addSibling(wings[1],WingSiblingType::MiddleFemale);
        wings[1]->addSibling(wings[0],WingSiblingType::MiddleMale);  
        return wings;
    };

      // simulate X(X)
    static  std::vector<std::shared_ptr<Wing>> getXx(std::shared_ptr<IWingStatusPublisher> publisher) {
     
        std::vector<std::shared_ptr<Wing>> wings;
        for (int i=0;i<2;i++) {
            int windowLength = 2000;
            auto motionManager = std::make_shared<TestMotorMotionManagerFakeCalibration>(windowLength);
            auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength,motionManager);
            auto w = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(), publisher);
            wings.push_back(w);    
        }
        // simulate X(X)

        wings[0]->addSibling(wings[1],WingSiblingType::Opposite);
        wings[1]->addSibling(wings[0],WingSiblingType::Opposite);  
        return wings;
    };
};


TEST(wingCalibrationHandlerTests,siblingtreeBasic ){
    Log::Init();
    auto fakePublisher = std::make_shared<TestWingStatusPublisher>();

    auto wings = wingCalibrationHandlerTestsProvider::getQx(fakePublisher);
    auto QX = WingCalibrationHandler::getAllWings(wings[0]);
    EXPECT_EQ(QX.size(),1) << "QX should have 1 wing listed";
    
    wings = wingCalibrationHandlerTestsProvider::getQxvXQ(fakePublisher);
    auto QXvXQ =  WingCalibrationHandler::getAllWings(wings[0]);
    EXPECT_EQ(QXvXQ.size(),2) << "QXvXQ should have 2 wings listed";

    wings = wingCalibrationHandlerTestsProvider::getXx(fakePublisher);
    auto Xx =  WingCalibrationHandler::getAllWings(wings[0]);
    EXPECT_EQ(Xx.size(),2) << "Xx should have 2 wings listed";
}

TEST(wingCalibrationHandlerTests,siblingtree ){
    Log::Init();

    auto wings = wingCalibrationHandlerTestsProvider::getQx_xX(std::make_shared<TestWingStatusPublisher>());

    auto tree_0 =WingCalibrationHandler::getAllWings(wings[0]);
    auto tree_1 =WingCalibrationHandler::getAllWings(wings[1]);
    auto tree_2 =WingCalibrationHandler::getAllWings(wings[2]);

    EXPECT_EQ(tree_0.size(),3) << "tree_0 should have 3 wings listed";
    EXPECT_EQ(tree_1.size(),3) << "tree_1 should have 3 wings listed";
    EXPECT_EQ(tree_2.size(),3) << "tree_2 should have 3 wings listed";

    // check if in all listed wings of tree_0 all siblingTypes are found
    auto checkTypeExistsOnList = [](std::vector<std::shared_ptr<IWing>> list, WingSiblingType siblingType) -> bool {
            auto result = std::find_if(std::begin(list), std::end(list), [& siblingType](std::shared_ptr<IWing> &x) {
                auto &siblings = x->getSiblings();
                if (std::find_if(std::begin(*siblings), std::end(*siblings), [& siblingType] (const std::tuple<std::shared_ptr<IWing>, WingSiblingType> &y) {
                        return siblingType == std::get<1>(y);
                    }) != std::end(*siblings))
                {
                    return true;
                };
                return false;
            });
        return (result != list.end());
    };

    auto checkOnAllTypes = [&](std::vector<std::shared_ptr<IWing>> list, std::string listName) {
        EXPECT_TRUE(checkTypeExistsOnList(list, WingSiblingType::MiddleFemale)) << listName << " should contain sibling with type Female";
        EXPECT_TRUE(checkTypeExistsOnList(list, WingSiblingType::MiddleMale)) << listName << " should contain sibling with type Male";
        EXPECT_TRUE(checkTypeExistsOnList(list, WingSiblingType::Opposite)) << listName << " should contain sibling with type Opposite";
    };

    checkOnAllTypes(tree_0,"tree_0");
    checkOnAllTypes(tree_1,"tree_1");
    checkOnAllTypes(tree_2,"tree_2");
}

TEST(wingCalibrationHandlerTests,clearCalibration ){
    Log::Init();
    auto fakePublisher = std::make_shared<TestWingStatusPublisher>();
    auto wings = wingCalibrationHandlerTestsProvider::getQx_xX(fakePublisher);

    std::dynamic_pointer_cast<Verifier>(fakePublisher)->clearCommandBuffer();

    WingCalibrationHandler::clearCalibration(wings[0], fakePublisher);

    EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(fakePublisher)->verifyCommandCalled("publishCalibrationCleared"),wings.size()) << "Expect each wing to be cleared" ;

}


TEST(wingCalibrationHandlerTests,areAllWingscalibrated) {
    Log::Init();
    auto fakePublisher = std::make_shared<TestWingStatusPublisher>();
    
    std::vector<std::shared_ptr<Wing>> wings;
    std::vector<std::shared_ptr<MotorMotionManager>> motionManagers;
    for (int i=0;i<2;i++) {
        int windowLength = 2000;
        auto motionManager = std::make_shared<TestMotorMotionManagerFakeCalibration>(windowLength);
        auto motorizedWindow = std::make_shared<MasterMotorizedWindow>(windowLength,motionManager);
        auto w = std::make_shared<Wing>(motorizedWindow, std::make_shared<WingRelationManager>(), fakePublisher);
        wings.push_back(w);   
        motionManagers.push_back(motionManager); 
    }
    // simulate QXvXQ
    wings[0]->addSibling(wings[1],WingSiblingType::MiddleFemale);
    wings[1]->addSibling(wings[0],WingSiblingType::MiddleMale);  
    
    
    EXPECT_FALSE(WingCalibrationHandler::areAllWingsCalibrated(wings[0]) ) << "not all wings are calibrated because none of the wings is calibrated";
    
    motionManagers[0]->open(); motionManagers[0]->close(); // should calibrate one wing
    LOG_DEBUG("check motionManager 0");
    EXPECT_TRUE(motionManagers[0]->isCalibrated()) << "first wing should be calibrated";
    LOG_DEBUG("check all wings");
    EXPECT_FALSE(WingCalibrationHandler::areAllWingsCalibrated(wings[0]) ) << "not all wings are calibrated because only one of the two is calibrated";

    
    motionManagers[1]->open(); motionManagers[1]->close(); // should calibrate other wing
    LOG_DEBUG("check motionManager 1");
    EXPECT_TRUE(motionManagers[1]->isCalibrated()) << "second wing should be calibrated";
    LOG_DEBUG("check all wings");
    EXPECT_TRUE(WingCalibrationHandler::areAllWingsCalibrated(wings[0]) ) << "Should return that all wings are calibrated";
}


TEST(wingCalibrationHandlerTests , getWingWithSpecificTypeAsSibling) {
    Log::Init();
    
    auto fakePublisher = std::make_shared<TestWingStatusPublisher>();
    auto wings = wingCalibrationHandlerTestsProvider::getQx_xX(fakePublisher);

    auto allWings =WingCalibrationHandler::getAllWings(wings[0]);

    auto femaleItt = WingCalibrationHandler::getWingWithSpecificTypeAsSibling(allWings,[](WingSiblingType t){ return t == WingSiblingType::CornerFemale || t == WingSiblingType::MiddleFemale;});
    EXPECT_FALSE(femaleItt==allWings.end()) << "Should find a female relation";
    EXPECT_TRUE((*femaleItt)->getSiblings()->size() == 1) << "The male has only the female as relation";
    EXPECT_TRUE( (*femaleItt) == wings[0]);

    auto maleItt = WingCalibrationHandler::getWingWithSpecificTypeAsSibling(allWings,[](WingSiblingType t){ return t == WingSiblingType::CornerMale || t == WingSiblingType::MiddleMale;});
    EXPECT_FALSE(maleItt==allWings.end()) << "Should find a male relation";
    EXPECT_TRUE((*maleItt)->getSiblings()->size() == 2) << "The female has two relations";
    EXPECT_TRUE( (*maleItt) == wings[1]);

    auto oppItt = WingCalibrationHandler::getWingWithSpecificTypeAsSibling(allWings,[](WingSiblingType t){ return t == WingSiblingType::Opposite; });
    EXPECT_FALSE(oppItt==allWings.end()) << "Should find an opposite relation";
    EXPECT_TRUE((*oppItt)->getSiblings()->size() > 0) << "The opposite can have one or two relations depending on wich opposite is found ";
    EXPECT_TRUE( (*oppItt) == wings[1] ||(*oppItt) == wings[2] );
}




TEST(wingCalibrationHandlerTests,calibration ){
    Log::Init();

    auto fakePublisher = std::make_shared<TestWingStatusPublisher>();


    auto evaluateCalibration = [](std::vector<std::shared_ptr<Wing>> & wings,std::shared_ptr<TestWingStatusPublisher> fakePublisher ) {

        auto _cancelCalibrationWorkerSignal = std::promise<void>();
        std::future<void> cancelObj = _cancelCalibrationWorkerSignal.get_future();

        std::dynamic_pointer_cast<Verifier>(fakePublisher)->clearCommandBuffer();
        WingCalibrationHandler::startCalibration(cancelObj, wings[0],fakePublisher);

        EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(fakePublisher)->verifyCommandCalled("publishCalibrationCleared"),wings.size()) << "Expect each wing to be cleared" ;
        EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(fakePublisher)->verifyCommandCalled("publishCalibrateOpenFinished"),wings.size()) << "Expect each wing to be Open calib finished" ;
        EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(fakePublisher)->verifyCommandCalled("publishCalibrateCloseFinished"),wings.size()) << "Expect each wing to be  Close calib finished " ;

        EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(fakePublisher)->verifyCommandCalled("publishCalibrated"),1) << "calibration success should be triggered once";
        EXPECT_EQ(std::dynamic_pointer_cast<Verifier>(fakePublisher)->verifyCommandCalled("publishCurrentMeasurmentFinished"),1) << "current measure success should be triggered once";

        EXPECT_TRUE(std::dynamic_pointer_cast<Verifier>(fakePublisher)->verifyCommandCalled("publishFullyOpen")>= 2*wings.size() ) << "Open finished at least twice for every wing" ;
        EXPECT_TRUE(std::dynamic_pointer_cast<Verifier>(fakePublisher)->verifyCommandCalled("publishFullyClosed")>= 2*wings.size() ) << "Close finished at least twice for every wing";
    };

    LOG_INFO("TEST calibartion on QX");
    auto wings = wingCalibrationHandlerTestsProvider::getQx(fakePublisher);
    evaluateCalibration(wings,fakePublisher);

    LOG_INFO("TEST calibartion on QXvXQ");
    wings = wingCalibrationHandlerTestsProvider::getQxvXQ(fakePublisher);
    evaluateCalibration(wings,fakePublisher);

    LOG_INFO("TEST calibartion on X(X)");
    wings = wingCalibrationHandlerTestsProvider::getXx(fakePublisher);
    evaluateCalibration(wings,fakePublisher);
    
    LOG_INFO("TEST calibartion on QX-X(X)");
    wings = wingCalibrationHandlerTestsProvider::getQx_xX(fakePublisher);
    evaluateCalibration(wings,fakePublisher);

    LOG_INFO("TEST calibartion on (X)X-X(X)");
    wings = wingCalibrationHandlerTestsProvider::getXx_xX(fakePublisher);
    evaluateCalibration(wings,fakePublisher);

    
}


