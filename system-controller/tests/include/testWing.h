#ifndef TESTWING_H
#define TESTWING_H

#include "wing.h"
#include "verifier.h"
#include "windowPushZone.h"

class TestWing : public IWing, public Verifier {
    public:
        TestWing(int fakeStroke, std::string wingName);
        void open() override;
        void close() override;
        void stop() override;
        bool hasOpeningLeft() override {return true;};
        void setPositionMm(int positionMm) override;
        void setPositionPerc(double positionPerc)override;
        void startCalibrate() override;
        bool isBusyCalibrating() const override { return false;};
        bool calibrateOpen() override{return true;}
        bool calibrateClose() override{return true;}
        bool isCalibrated()override {return false;}        
        void waitOnCalibration() override {return;}
        bool hasCalibratedMotors() override {return false;}
        void clearCalibration() override {}
        const virtual std::string getWingId() const override {return _wingName;}
        int getPosition() override;
        int getTarget() override;
        bool waslastMovementOpening() const override { return _wasLastMovementOpening;}
        void addSibling(std::shared_ptr<IWing> sibling, WingSiblingType siblingType) override;
        std::shared_ptr<MasterMotorizedWindow> getMasterWindow() const override ;
        const std::vector<std::shared_ptr<MotorizedWindow>> getMotors() const override;
        const std::shared_ptr<IWindowPushZone> getCornerPushZone() const override {return _currentCornerPushZone;}
        const std::shared_ptr<IWindowPushZone> getOppositePushZone() const override {return _currentOppositePushZone;}
        const virtual std::shared_ptr<std::vector<std::tuple<std::shared_ptr<IWing>,WingSiblingType>>> getSiblings() const override ;
        void setDelegateWingPublishOutput(std::function<void(MqttData)> delegatePublishOutput) {};
        void updateWingMovement() override;
        void SetFullSetupCalibDone() override{};
    private : 
        int _fakePos=0;
        int _fakeStroke=0;
        std::string _wingName;
        std::vector<std::shared_ptr<MotorizedWindow>> _motors;
        std::shared_ptr<IWindowPushZone> _currentCornerPushZone;
        std::shared_ptr<IWindowPushZone> _currentOppositePushZone;
        bool _wasLastMovementOpening = true;

};

#endif //TESTWING_H