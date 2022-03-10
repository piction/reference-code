#ifndef MASTERMOTORIZEDWINDOW_H
#define MASTERMOTORIZEDWINDOW_H

#include "pch.h"
#include "motorizedWindow.h"
#include "motorMotionManager.h"


enum class TargetType {
    Open,
    Close,
    Position,
    CalibrateStrokeOpen,
    CalibrateStrokeClose,
    None
};


class MasterMotorizedWindow  : public MotorizedWindow  {
    public:
        ~MasterMotorizedWindow ();
        MasterMotorizedWindow(int length, std::shared_ptr<IMotorMotionManager> motionManager);        
        void open();
        void close();
        bool areAllMotorsConfigured();
        std::future<bool> calibrateOpen();
        std::future<bool> calibrateClose();
        void clearCalibration() override;
        void stop();
        void setTarget(int newTarget);
        int getTarget() override {return _target;}
        int getTailDistanceToFullyClose();
        void stopWindow() override;
        int const interSlaveCalibrationDistance = 100;   
        int const maximumStrokeForCalibration = 20000;
        void onPositionUpdate(int newPosition) override;
        bool isOntarget();
        
    protected : 
        int _target;   
        int _positionOfTailWhenFullyOpen=0;
        TargetType _targetType;  
    private:
        bool _calibrateBusy=false;
        bool _calibratedOpen=false;
        bool _calibratedClose=false;
        void manageOperationalMovement() ;    
        bool calibrateWorker(std::future<void> cancelObj);
        void stopCalibrationWorkers();        
        void updatePanelLengtsBasedOnstroke();
        
        std::promise<void> _cancelWorkerCalibOpenSignal;
        std::promise<void> _cancelWorkerCalibCloseSignal;

};





#endif //MASTERMOTORIZEDWINDOW_H