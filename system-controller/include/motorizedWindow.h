#ifndef MOTORIZEDWINDOW_H
#define MOTORIZEDWINDOW_H

#include "pch.h"
#include "movingWindow.h"
#include "passiveWindow.h"
#include "motorMotionManager.h"

class MotorizedWindow  : public MovingWindow ,public std::enable_shared_from_this<MotorizedWindow> {
    public:
        virtual ~MotorizedWindow() {}
        MotorizedWindow( int length, std::shared_ptr<IMotorMotionManager> motionManager);
        void push(PushType push) override;
        void stopWindow() override;      
        int getAllowedPositionTolerance() {return  2;}   
        virtual void clearCalibration();
        std::vector<std::shared_ptr<MotorizedWindow>> getSlaveMotors() const ;
    private:
        void getMotorsRecursive(std::vector<std::shared_ptr<MotorizedWindow>> & motors, std::shared_ptr<IMovingWindow> curr) const;
};





#endif //MOTORIZEDWINDOW_H