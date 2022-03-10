#ifndef WING_H
#define WING_H

#include "pch.h"
#include "movingWindow.h"
#include "wingSiblingType.h"
#include "positionTrack.h"
#include "canCalibrate.h"
#include "wingStatus.h"
#include "masterMotorizedWindow.h"
#include "windowPushZone.h"
#include "wingRelationManager.h"
#include "wingStatusPublisher.h"
#include "wingCalibrationHandler.h"


class IWing : public  IPositionTrack ,  public ICanCalibrate {
    public: 
        virtual ~IWing(){}
        virtual void open()=0;
        virtual void close()=0;
        virtual void stop()=0;
        virtual bool hasOpeningLeft()=0;
        virtual void setPositionMm(int positionMm)=0;
        virtual void setPositionPerc(double positionPerc)=0;        
        const virtual std::string getWingId() const=0;
        virtual void addSibling(std::shared_ptr<IWing> sibling, WingSiblingType siblingType)=0;        
        virtual std::shared_ptr<MasterMotorizedWindow> getMasterWindow() const =0;
        const virtual std::vector<std::shared_ptr<MotorizedWindow>> getMotors() const =0;        
        const virtual std::shared_ptr<IWindowPushZone> getCornerPushZone() const =0;
        const virtual std::shared_ptr<IWindowPushZone> getOppositePushZone() const =0;
        const virtual std::shared_ptr<std::vector<std::tuple<std::shared_ptr<IWing>,WingSiblingType>>> getSiblings() const =0; 
        virtual void updateWingMovement() =0;
        virtual void SetFullSetupCalibDone() = 0;
        virtual void setDelegateWingPublishOutput(std::function<void(MqttData)> delegatePublishOutput) = 0;
        virtual bool hasCalibratedMotors() =0 ;
};



class Wing : public IWing , public std::enable_shared_from_this<Wing>{    
    
    public:
        Wing(std::shared_ptr<MasterMotorizedWindow> masterWindow,
            std::shared_ptr<WingRelationManager> wingRelationManager, 
            std::shared_ptr<IWingStatusPublisher> wingStatusPublisher, 
            //const std::string & wingName,
            bool hasOpeningLeft =true);
        void open() override;
        void close() override;
        void stop() override;        
        bool hasOpeningLeft() override {return _hasOpeningLeft;}
        void setPositionMm(int positionMm) override;
        void setPositionPerc(double positionPerc)override;
        int getPosition() override;
        int getTarget() override {return _currentWingStatus->getTarget() ;}        
        bool waslastMovementOpening() const override {return _currentWingStatus->waslastMovementOpening();}
        bool isBusyCalibrating() const override { return _currentWingStatus->getTargetType() == WingTarget::Calibration;};
        bool calibrateOpen() override;
        bool calibrateClose() override;        
        void clearCalibration() override;
        void startCalibrate() override;
        void waitOnCalibration() override;
        bool hasCalibratedMotors() override;
        bool isCalibrated() override {return _isFullSetupCalibDone || hasCalibratedMotors();}
        const std::string getWingId() const override {return _wingName;}
        void addSibling(std::shared_ptr<IWing> sibling, WingSiblingType siblingType) override;
        std::shared_ptr<MasterMotorizedWindow> getMasterWindow() const override;
        const std::vector<std::shared_ptr<MotorizedWindow>> getMotors() const override;
        const std::shared_ptr<IWindowPushZone> getCornerPushZone() const override {return _currentWingStatus->getCornerPushZone();}
        const std::shared_ptr<IWindowPushZone> getOppositePushZone() const override {return _currentWingStatus->getOppositePushZone();}
        const std::shared_ptr<std::vector<std::tuple<std::shared_ptr<IWing>,WingSiblingType>>> getSiblings() const override {return _siblings;}
        void updateWingMovement() override;
        void SetFullSetupCalibDone() override {_isFullSetupCalibDone=true;}
        void setDelegateWingPublishOutput(std::function<void(MqttData)> delegatePublishOutput) override;
        

    protected:        
        std::shared_ptr<MasterMotorizedWindow> _masterWindow;
        std::shared_ptr<WingRelationManager> _wingRelationManager;
        std::shared_ptr<IWingStatusPublisher> _wingStatusPublisher;        
        std::shared_ptr<std::vector<std::tuple<std::shared_ptr<IWing>,WingSiblingType>>> _siblings; 
        void checkSiblingRelations();
        bool _hasOpeningLeft =false;
        bool _isFullSetupCalibDone=false;

        void startCalibrateOpen(std::function<void(void)> onCalibratedOpen, std::function<void(void)> onCancel);
        void startCalibrateClose(std::function<void(void)> onCalibratedClose, std::function<void(void)> onCancel);
        
        
        std::shared_ptr<IWingStatus> _currentWingStatus;
        std::shared_ptr<IWingStatus> _lastWingStatus;
        static int instanceCounter;


    private :        
        void blockedByOppositeRelation();
        void blockedByCornerRelation();
        void unBlockedByOppositeRelation();
        void unBlockedByCornerRelation();
        
        void releaseWing();
        void logWingStatus();

        void validateRelation(std::tuple<std::shared_ptr<IWing>,WingSiblingType> & wingInfo, int position);
        std::string _wingName;

        
        std::promise<void> _cancelCalibrationWorkerSignal;
        std::future<void> _workOnCalibFuture;
        std::set<std::string> blockingSiblings;
        
};





#endif //WING_H