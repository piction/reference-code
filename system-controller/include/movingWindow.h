
#ifndef MOVINGWINDOW_H
#define MOVINGWINDOW_H

#include "pch.h"
#include "motorMotionManager.h"
#include "motorData.h"
#include "systemSettings.h"



enum class SlaveType {
    Motor,
    Passive
};

enum class PushType {
    Stop,
    PushToOpen,
    PushToClose,
    ForceOpenForCalibration,
    ForceCloseForCalibration
};
enum class MovementFreedom {
    None,
    Slow,
    Fast
};

class IMovingWindow {
    public:
        
        virtual ~IMovingWindow() {}
        virtual void addSlave(std::shared_ptr<IMovingWindow> slave, SlaveType slaveType) =0;
        virtual std::vector<std::shared_ptr<IMovingWindow>> getSlaves() const = 0;
        virtual int getPosition() =0 ; 
        virtual int getTarget()=0;
        virtual int getSpeed() =0;
        virtual int getLength() =0;        
        virtual void stopWindow()=0;
        virtual MovementFreedom getMovementFreedom () const =0;
        virtual std::shared_ptr<IMotorMotionManager> getMotionManager() const =0;
        virtual void getSlaveTypesTree(std::vector<SlaveType> & slaveTypesTree)=0;
        virtual void updateWithCurrentPosition()=0;
        virtual void updateAllPanelLenghts(int newPanelLength)=0;
};

/*
 * Generic class that will handle interaction with slave windows
 * All slaves of a master will be checked and if a slave needs 
 * to move away the slaves of the slaves will be checked also
 *  
 */

class MovingWindow: public IMovingWindow {
    public:
        virtual ~MovingWindow() {}
        void addSlave(std::shared_ptr<IMovingWindow> slave, SlaveType slaveType) override;        
        std::vector<std::shared_ptr<IMovingWindow>> getSlaves() const override;  
        int getPosition() override {return _position;}
        int getTarget() override {return _position;} // should be overriden by master
        int getSpeed() override {return _speed;}
        int getLength() override {return _length;}        
        MovementFreedom getMovementFreedom () const override {return _freeToMove;}
        void stopWindow() override; 
        virtual void onPositionUpdate(int newPosition);
        void getSlaveTypesTree(std::vector<SlaveType> & slaveTypesTree) override;
        std::shared_ptr<IMotorMotionManager> getMotionManager() const override {return _motionManager;}
        void updateWithCurrentPosition() override;
        static MovementFreedom GetLeastAllowedMovement (MovementFreedom  m1 , MovementFreedom m2);
        void updateAllPanelLenghts( int newPanelLength) override;

        
    protected:
        MovingWindow( int length, std::shared_ptr<IMotorMotionManager> motorMotionManager) ;
        void pushSlavesToAllowMovement();
        virtual void push(PushType push);
        int _length;
        int _position;
        int _speed;
        PushType _pushType;
        MovementFreedom _freeToMove = MovementFreedom::None;  
        std::shared_ptr<IMotorMotionManager> _motionManager;
        // store slaves with a slave type to identify 
        std::vector<std::tuple<std::shared_ptr<IMovingWindow>,SlaveType>> _slaves; 
        
    private:
        void handleSlaveOnPushOpen(MovementFreedom & canStartOwnMovement, const std::shared_ptr<IMovingWindow>& slave, SlaveType slaveType) ;
        void handleSlaveOnPullClose(MovementFreedom & canStartOwnMovement, std::shared_ptr<IMovingWindow> slave, SlaveType slaveType) ;
};
#endif //MOVINGWINDOW_H