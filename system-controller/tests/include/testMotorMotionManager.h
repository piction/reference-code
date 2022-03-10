#ifndef TESTMOTORMOTIONMANAGER_H
#define TESTMOTORMOTIONMANAGER_H

#include "motorMotionManager.h"
#include "verifier.h"

class TestMotorMotionManager : public MotorMotionManager, public Verifier {
    public :
        TestMotorMotionManager(int fakeStroke);        
        void setHighSpeed() override;
        void setLowSpeed() override;
        int getLowSpeed() const override ;
        
        void stop() override;
        void close() override;
        void open() override;
        MotorStatusData getMotorStatusData() const override;
        void setPosition(int position) override;
        int getStroke() const override {return isCalibrated() ? _fakeStroke : -1;}        
        std::future<bool> clearCalibration() override;
        void updateWithFakePosition(int position);
        std::string getId() const {return "TestMotorMotionManager";};
        virtual bool isCalibrated() const override ;
        virtual bool getIsConfigured() const override ;        


        void SetFakeMotorStatus( MotorStatus fakeStatus) ;
        void SetFakeMotorStatusData( MotorStatusData data) ;
        void ManipulateStroke(int newStroke) { _fakeStroke = newStroke;}

    protected:
        MotorStatusData _fakeStatus;
        int _fakeStroke;
};

class TestMotorMotionManagerFakeCalibration : public TestMotorMotionManager {
    public :
        TestMotorMotionManagerFakeCalibration(int fakeStroke);

        void stop() override;
        void close() override;
        void open() override;        
        void setPosition(int position) override;
        std::future<bool> clearCalibration() override;
    protected:
        bool isMovedOpenOnce=false;
        bool isMovedCloseOnce=false;
};

#endif 