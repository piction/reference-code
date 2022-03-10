#ifndef PASSIVEWINDOW_H
#define PASSIVEWINDOW_H

#include "pch.h"
#include "movingWindow.h"
#include "motorMotionManager.h"

class PassiveWindow: public MovingWindow {
    public :
        ~PassiveWindow() {}
        PassiveWindow(int length);

        int getLastStandStillPosition(){return _lastStandstillPosition;}
        void setSpeedDuePushingOrPulling(int speed);
        void setPositionDuePushingOrPulling(int position);
        void setMovementFreedomDuePossibleContact(MovementFreedom m);        
        void push(PushType push) override ;
                
    protected:
        
        int _lastStandstillPosition=0;
        int _isCurrentDirectionOpening=false;
};


#endif //PASSIVEWINDOW_H