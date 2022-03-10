#include <gtest/gtest.h>
#include <memory>


#include "log.h"
#include "positionTrack.h"
#include "wingRelationManager.h"
#include "testMotorMotionManager.h"
#include "verifier.h"

class FakePositionTrack : public IPositionTrack {
    public: 
        FakePositionTrack(int currentPos, int target): _currentPos(currentPos),_target(target){}
        int _target;
        int _currentPos;
        int getTarget() override  {return _target;}
        int getPosition() override {return _currentPos;}
        bool waslastMovementOpening() const override {return false;}

};

class FakePushZone : public IWindowPushZone, public Verifier {
         public:
            void setZone(int minOpen, int maxOpen) override {
                _internalPushZone.setZone(minOpen,maxOpen);
                _commandsCalledBuffer.append("setZone,");
            }
            void setMinOpening(int minOpen)override{
                _internalPushZone.setMinOpening(minOpen);
                _commandsCalledBuffer.append("setMinOpening,");
            }
            void setMaxOpening(int maxOpen)override{
                _internalPushZone.setMaxOpening(maxOpen);
                _commandsCalledBuffer.append("setMaxOpening,");
            }
            void inActivate()override{
                _internalPushZone.inActivate();
                _commandsCalledBuffer.append("inActivate,");
            }
            int getMaxOpen() const override{
                return _internalPushZone.getMaxOpen();
            }
            int getMinOpen() const override{
                return _internalPushZone.getMinOpen();
            }
            bool isActive() const override {
                return _internalPushZone.isActive();
            }
            bool isMinLimActive() const override {
                return _internalPushZone.isMinLimActive();
            }
            bool isMaxLimActive() const override {
                return _internalPushZone.isMaxLimActive();
            }
            bool isInZone(int position) const override {
                return _internalPushZone.isInZone(position);
            }
            bool shouldOpen(int position) const override{
                return _internalPushZone.shouldOpen(position);
            }
            bool shouldClose(int position) const override {
                return _internalPushZone.shouldClose(position);
            }
            std::string toString() override {
                return "No to string implementation";
            };
            bool isEqual (const std::shared_ptr<IWindowPushZone> otherZone) const override{
            return (otherZone->getMaxOpen() == _internalPushZone.getMaxOpen()) && (otherZone->getMinOpen() == _internalPushZone.getMinOpen());
        }   
        private:
            WindowPushZone _internalPushZone;
};

TEST(WingRelationManagerTests,opposite ){
    
    Log::Init();
    
    auto pushZone = std::make_shared<FakePushZone>();
    WingRelationManager sut;

    int unblockCounter=0;int blockCounter = 0;
    int wingCurrentPos=0; int wingTargetPos=0;  int tailDistanceToFullyClose= 0; 
    bool otherWingFullyClosed=false; bool isWingOnTarget = false;
    bool shouldBlock = false; bool shouldPush=false;

    auto testOpposite = [&]() {
        pushZone->inActivate();
        auto wing = std::make_shared<FakePositionTrack>(wingCurrentPos, wingTargetPos);
        sut.ManageOpposite(
            [&blockCounter]() {blockCounter++;   },
            [&unblockCounter]() {unblockCounter++;},
            pushZone,
            wing,            
            tailDistanceToFullyClose,
            otherWingFullyClosed,
            std::fabs(wingTargetPos-wingCurrentPos) < 1) ;            
    };
    auto validate = [&]( bool shouldBlock,bool shouldPush,std::string description){
        LOG_DEBUG("--->" + description);
        unblockCounter=0;   blockCounter = 0;
        pushZone->clearCommandBuffer();
        testOpposite();
        EXPECT_EQ(blockCounter,shouldBlock ? 1 : 0) << "block : " << description;
        EXPECT_EQ(unblockCounter,shouldBlock ? 0 : 1) << "unblock : " << description;
        EXPECT_EQ(shouldPush,pushZone->isActive()) << (shouldPush ? "Push zone should be active, " : "Push zone should not be active, ") << description;
    };
    std::string testMessage;
    EXPECT_LT(SystemSettings::getInstance().getOppositeZone() +10 , SystemSettings::getInstance().getTriggerPushWingDistance()) << "Expect that triggerzone is bigger than opposite zone";
    
    testMessage = "test 1 : all closed, no targets";
    wingCurrentPos=0; wingTargetPos=0; tailDistanceToFullyClose= SystemSettings::getInstance().getTriggerPushWingDistance() * 5; otherWingFullyClosed=true;
    shouldBlock = false;  shouldPush=false; 
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 2 : ";
    wingCurrentPos=0; wingTargetPos=1000; tailDistanceToFullyClose= wingTargetPos +  SystemSettings::getInstance().getTriggerPushWingDistance() * 5; otherWingFullyClosed=true;
    shouldBlock = false;  shouldPush=false; 
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 3 : ";
    wingCurrentPos=600; wingTargetPos=1000; tailDistanceToFullyClose=wingTargetPos + SystemSettings::getInstance().getTriggerPushWingDistance() * 5; otherWingFullyClosed=true;
    shouldBlock = false;  shouldPush=false; 
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 4 :";
    wingCurrentPos=600; wingTargetPos=3000  ; tailDistanceToFullyClose = wingCurrentPos + SystemSettings::getInstance().getTriggerPushWingDistance() + 5; otherWingFullyClosed=false;
    shouldBlock = false;  shouldPush=false; 
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 5 : ";
    wingCurrentPos=600; wingTargetPos=3000  ; tailDistanceToFullyClose = wingCurrentPos + SystemSettings::getInstance().getTriggerPushWingDistance() - 5; otherWingFullyClosed=false;
    shouldBlock = false;  shouldPush=true; 
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 6 : ";
    wingCurrentPos=600; wingTargetPos=3000  ; tailDistanceToFullyClose = wingCurrentPos + SystemSettings::getInstance().getOppositeZone() + 5; otherWingFullyClosed=false;
    shouldBlock = false;  shouldPush=true; 
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 7 : ";
    wingCurrentPos=600; wingTargetPos=3000  ; tailDistanceToFullyClose = wingCurrentPos + SystemSettings::getInstance().getOppositeZone() - 5; otherWingFullyClosed=false;
    shouldBlock = true;  shouldPush=true; 
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 8 : ";
    wingCurrentPos=600; wingTargetPos=3000 ;  tailDistanceToFullyClose = wingTargetPos + SystemSettings::getInstance().getOppositeZone() + 5; otherWingFullyClosed=false;
    shouldBlock = false;  shouldPush=false; 
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 9 : move to away from opposite";otherWingFullyClosed=false;
    wingCurrentPos=6000; wingTargetPos=5 ;  tailDistanceToFullyClose = wingCurrentPos + 2 ; otherWingFullyClosed=false;
    shouldBlock = false;  shouldPush=false; 
    validate(shouldBlock,shouldPush,testMessage);

}


TEST(WingRelationManagerTests,female ){
    Log::Init();

    auto pushZone = std::make_shared<FakePushZone>();
    WingRelationManager sut;

    int unblockCounter=0;int blockCounter = 0;
    int maleCurrentPos=0; int maleTargetPos=0; int femaleCurrentPos = 0; int femaleTargetPos=0;
    bool shouldBlock = false; bool shouldPush=false;

    auto testFemale = [&]() {

        auto maleWing = std::make_shared<FakePositionTrack>(maleCurrentPos, maleTargetPos);
        auto femaleWing = std::make_shared<FakePositionTrack>(femaleCurrentPos, femaleTargetPos);

        sut.ManageFemaleCorner(
            [&blockCounter]() {blockCounter++;   },
            [&unblockCounter]() {unblockCounter++;},
            pushZone,
            maleWing,
            femaleWing);            
    };
    auto validate = [&]( bool shouldBlock,bool shouldPush,std::string description){
        unblockCounter=0;   blockCounter = 0;
        pushZone->clearCommandBuffer();
        testFemale();
        EXPECT_EQ(blockCounter,shouldBlock ? 1 : 0) << "block : " << description;
        EXPECT_EQ(unblockCounter,shouldBlock ? 0 : 1) << "unblock : " << description;
        EXPECT_EQ(shouldPush,pushZone->isActive()) << (shouldPush ? "Push zone should be active, " : "Push zone should not be active, ") << description;

    };
    std::string testMessage;

    
    testMessage = "test 1 : all closed, no targets";
    maleCurrentPos=0;  maleTargetPos=0;   femaleCurrentPos = 0;femaleTargetPos=0;    
    shouldBlock = false;  shouldPush=true; // keep female closed by pushing close
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = " test 2 : female closed , male opening";
    maleCurrentPos=0;  maleTargetPos=SystemSettings::getInstance().getCornerZone() + 100;   femaleCurrentPos = 0; femaleTargetPos=0;    
    shouldBlock = false;  shouldPush=true; // keep female closed by pushing close
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = " test 2B : female closed , male opening passed cornersize";
    maleCurrentPos=SystemSettings::getInstance().getCornerZone() + 100;  maleTargetPos=SystemSettings::getInstance().getCornerZone() + 200;   femaleCurrentPos = 0; femaleTargetPos=0;    
    shouldBlock = false;  shouldPush=false; // keep female closed by pushing close
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 3 : female closed , male closing but not in corner zone";
    maleCurrentPos=SystemSettings::getInstance().getTriggerPushWingDistance() +100;  maleTargetPos=SystemSettings::getInstance().getCornerZone() + 100;   femaleCurrentPos = 0; femaleTargetPos=0;    
    shouldBlock = false;  shouldPush=false;
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 3B : female closed , male closing with target to close fully";
    maleCurrentPos=SystemSettings::getInstance().getTriggerPushWingDistance() +100;  maleTargetPos=0;   femaleCurrentPos = 0;  femaleTargetPos=0;    
    shouldBlock = false;  shouldPush=false;
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 3B : female closed , male closing with target to close fully and aproaching corner";
    maleCurrentPos=SystemSettings::getInstance().getCornerZone() * 2;  maleTargetPos=0;   femaleCurrentPos = 0;  femaleTargetPos=0;    
    shouldBlock = false;  shouldPush=true;
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 4 : female in corner zone but not at closed , Male not yet in zone and moving away";
    maleCurrentPos=SystemSettings::getInstance().getCornerZone() + 10;  maleTargetPos=SystemSettings::getInstance().getCornerZone() + 100;   femaleCurrentPos = SystemSettings::getInstance().getCornerZone()/2;  femaleTargetPos=femaleCurrentPos;    
    shouldBlock = false;  shouldPush=false;
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 4B : female in corner zone but not at closed but closing, Male not yet in zone and moving away";
    maleCurrentPos=SystemSettings::getInstance().getCornerZone() + 10;  maleTargetPos=SystemSettings::getInstance().getCornerZone() + 100;   femaleCurrentPos = SystemSettings::getInstance().getCornerZone()/2;  femaleTargetPos=0;    
    shouldBlock = false;  shouldPush=false;
    validate(shouldBlock,shouldPush,testMessage);
    
    testMessage = "test 5 : female not in corner and not planning to be, male is in corner";
    maleCurrentPos=SystemSettings::getInstance().getCornerZone()/2 ;  maleTargetPos=SystemSettings::getInstance().getCornerZone() + 100;   femaleCurrentPos = SystemSettings::getInstance().getCornerZone()*2;  femaleTargetPos=femaleCurrentPos;    
    shouldBlock = false;  shouldPush=true; // make sure that female can not come to corner
    validate(shouldBlock,shouldPush,testMessage);
    
    testMessage = "test 6 : female in corner but not closed and no intent , male is closing (not yet entering cornerzone)";
    maleCurrentPos=SystemSettings::getInstance().getCornerZone()*2 ;  maleTargetPos=0;   femaleCurrentPos = SystemSettings::getInstance().getCornerZone()/2;  femaleTargetPos=femaleCurrentPos;    
    shouldBlock = false;  shouldPush=true;
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 6B : female in corner but not closed and no intent , male is closing (not yet entering cornerzone)";
    maleCurrentPos=SystemSettings::getInstance().getCornerZone()*2 ;  maleTargetPos=SystemSettings::getInstance().getCornerZone()/2;   femaleCurrentPos = SystemSettings::getInstance().getCornerZone()/2;  femaleTargetPos=femaleCurrentPos;    
    shouldBlock = false;  shouldPush=true;
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 7 : female in corner but not closed and no intent, male is closing in cornerzone";
    maleCurrentPos=SystemSettings::getInstance().getCornerZone()/2 ;  maleTargetPos=0;   femaleCurrentPos = SystemSettings::getInstance().getCornerZone()/2;  femaleTargetPos=0;    
    shouldBlock = true;  shouldPush=true;
    validate(shouldBlock,shouldPush,testMessage);
   
    testMessage = "test 7 : female in and closed, male is closing in cornerzone";
    maleCurrentPos=SystemSettings::getInstance().getCornerZone()/2 ;  maleTargetPos=0;   femaleCurrentPos = 0;  femaleTargetPos=0;    
    shouldBlock = false;  shouldPush=true;
    validate(shouldBlock,shouldPush,testMessage);

    testMessage = "test 8 : female in corner but not closed and no intent, male is closing in cornerzone";
    maleCurrentPos=SystemSettings::getInstance().getCornerZone()/2 ;  maleTargetPos=SystemSettings::getInstance().getCornerZone()/3;   femaleCurrentPos = SystemSettings::getInstance().getCornerZone()/2;  femaleTargetPos=femaleCurrentPos;    
    shouldBlock = true;  shouldPush=true;
    validate(shouldBlock,shouldPush,testMessage);

}
    

TEST(WingRelationManagerTests,male ){
    Log::Init();
    auto pushZone = std::make_shared<FakePushZone>();
    WingRelationManager sut;

    int unblockCounter=0;int blockCounter = 0;
    int maleCurrentPos=0; int maleTargetPos=0; int femaleCurrentPos = 0; int femaleTargetPos=0;
    bool shouldBlockFemale = false; bool shouldPushMale=false;

    auto testMale = [&]() {

        auto maleWing = std::make_shared<FakePositionTrack>(maleCurrentPos, maleTargetPos);
        auto femaleWing = std::make_shared<FakePositionTrack>(femaleCurrentPos, femaleTargetPos);
        
        sut.ManageMaleCorner(
            [&blockCounter]() {blockCounter++;   },
            [&unblockCounter]() {unblockCounter++;},
            pushZone,
            maleWing,
            femaleWing);   
       
    };
    auto validate = [&]( bool shouldBlockFemale,bool shouldPushMale,std::string description){
        unblockCounter=0;   blockCounter = 0;
        pushZone->clearCommandBuffer();
        testMale();
        EXPECT_EQ(blockCounter,shouldBlockFemale ? 1 : 0) << "block : " << description;
        EXPECT_EQ(unblockCounter,shouldBlockFemale ? 0 : 1) << "unblock : " << description;
        EXPECT_EQ(shouldPushMale,pushZone->isActive()) << (shouldPushMale ? "Push zone should be active, " : "Push zone should not be active, ") << description;
    };

    std::string testMessage;
    
    testMessage = "test 1 : all closed";
    femaleCurrentPos=0;  femaleTargetPos=0;   maleCurrentPos = 0;  maleTargetPos=0;
    shouldBlockFemale = true;  shouldPushMale=false;
    validate(shouldBlockFemale,shouldPushMale,testMessage);
    
    testMessage = "test 2 : male closed , female opening";
    femaleCurrentPos=0;  femaleTargetPos=SystemSettings::getInstance().getCornerZone() + 100;   maleCurrentPos = 0;maleTargetPos=0;
    shouldBlockFemale = true;  shouldPushMale=true;
    validate(shouldBlockFemale,shouldPushMale,testMessage);

    testMessage = "test 3 : male closed , female closing but not in corner zone";
    femaleCurrentPos=SystemSettings::getInstance().getCornerZone() + 20;  femaleTargetPos=SystemSettings::getInstance().getCornerZone() + 10;   maleCurrentPos = 0;maleTargetPos=0;
    shouldBlockFemale = false;  shouldPushMale=false;
    validate(shouldBlockFemale,shouldPushMale,testMessage);
    
    testMessage = "test 3B : male closed , female closing with target to close fully";
    femaleCurrentPos=SystemSettings::getInstance().getCornerZone() + 5;  femaleTargetPos=0;   maleCurrentPos = 0;maleTargetPos=0;
    shouldBlockFemale = false;  shouldPushMale=true;
    validate(shouldBlockFemale,shouldPushMale,testMessage);

    testMessage = "test 3B : male closed , female closing with target in cornerzone";
    femaleCurrentPos=SystemSettings::getInstance().getCornerZone() + 10;  femaleTargetPos= SystemSettings::getInstance().getCornerZone() / 2;   maleCurrentPos = 0;maleTargetPos=0;
    shouldBlockFemale = false;  shouldPushMale=true;
    validate(shouldBlockFemale,shouldPushMale,testMessage);

    testMessage = "test 4 : male in corner zone but not at closed , female not yet in zone and moving away";
    femaleCurrentPos=SystemSettings::getInstance().getCornerZone() + 10;  femaleTargetPos=SystemSettings::getInstance().getCornerZone() + 100;   maleCurrentPos = SystemSettings::getInstance().getCornerZone()/2; maleTargetPos=maleCurrentPos;
    shouldBlockFemale = false;  shouldPushMale=false;
    validate(shouldBlockFemale,shouldPushMale,testMessage);

    testMessage = "test 4B : male in corner zone but not at closed but intented to close , female not yet in zone and moving away";
    femaleCurrentPos=SystemSettings::getInstance().getCornerZone() + 10;  femaleTargetPos=SystemSettings::getInstance().getCornerZone() + 100;   maleCurrentPos = SystemSettings::getInstance().getCornerZone()/2; maleTargetPos=0;
    shouldBlockFemale = false;  shouldPushMale=false;
    validate(shouldBlockFemale,shouldPushMale,testMessage);
    
    testMessage = "test 5 : male not in corner, female is in corner";
    femaleCurrentPos=SystemSettings::getInstance().getCornerZone()/2 ;  femaleTargetPos=SystemSettings::getInstance().getCornerZone() + 100;   maleCurrentPos = SystemSettings::getInstance().getCornerZone()*2;maleTargetPos=maleCurrentPos;
    shouldBlockFemale = false;  shouldPushMale=false;
    validate(shouldBlockFemale,shouldPushMale,testMessage);
    
    testMessage = "test 6 : male in corner but not closed , female is closing (not yet entering cornerzone)";
    femaleCurrentPos=SystemSettings::getInstance().getCornerZone()*2 ;  femaleTargetPos=0;   maleCurrentPos = SystemSettings::getInstance().getCornerZone()/2;maleTargetPos=0;
    shouldBlockFemale = false;  shouldPushMale=true;
    validate(shouldBlockFemale,shouldPushMale,testMessage);

    testMessage = "test 6B : male in corner but not closed , female is closing (not yet entering cornerzone)";
    femaleCurrentPos=SystemSettings::getInstance().getCornerZone()*2 ;  femaleTargetPos=SystemSettings::getInstance().getCornerZone()/2;   maleCurrentPos = SystemSettings::getInstance().getCornerZone()/2; maleTargetPos=maleCurrentPos;
    shouldBlockFemale = false;  shouldPushMale=true;
    validate(shouldBlockFemale,shouldPushMale,testMessage);

    testMessage = "test 7 : male in corner but not closed , female is closing in cornerzone";
    femaleCurrentPos=SystemSettings::getInstance().getCornerZone()/2 ;  femaleTargetPos=0;   maleCurrentPos = SystemSettings::getInstance().getCornerZone()/2;maleTargetPos=maleCurrentPos;
    shouldBlockFemale = true;  shouldPushMale=true;
    validate(shouldBlockFemale,shouldPushMale,testMessage);

    testMessage = "test 8 : male in corner but not closed , female is opening in cornerzone";
    femaleCurrentPos=SystemSettings::getInstance().getCornerZone()/2 ;  femaleTargetPos=SystemSettings::getInstance().getCornerZone()/3;   maleCurrentPos = SystemSettings::getInstance().getCornerZone()/2;maleTargetPos=maleCurrentPos;
    shouldBlockFemale = true;  shouldPushMale=true;
    validate(shouldBlockFemale,shouldPushMale,testMessage);
}
    