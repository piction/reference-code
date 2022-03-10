#ifndef MOTORMOTIONMANAGER_H
#define MOTORMOTIONMANAGER_H


#include "pch.h"
#include "motorData.h"


// this manager will abstract the communication details
// of interacting with a representation of a motor (for example an MQTT variant)

class  IMotorMotionManager {
    public :
        virtual ~IMotorMotionManager(){}
        virtual void setHighSpeed() =0;
        virtual void setLowSpeed()=0;
        virtual int getLowSpeed() const =0;

        virtual void stop()=0;
        virtual void close()=0;
        virtual void open()=0;
        virtual void setPosition(int position)=0;
        virtual int getStroke() const =0; 
        virtual std::future<bool> clearCalibration() =0;
        virtual MotorStatusData getMotorStatusData() const =0;
        virtual std::string getId() const = 0;

        virtual bool isCalibrated() const =0;
        virtual bool getIsConfigured() const =0;
        virtual int addOnPositionUpdatehandler(std::function<void(int)> onPositionUpdatehandler) =0;
        virtual int addOnMotorStatusUpdatehandler(std::function<void(MotorStatus)> onMotorStatusUpdatehandler) =0;
        virtual int addOnMotorCalibratedhandler(std::function<void(void)> onMotorCalibratedhandler) =0;

    protected:
        std::map<int,std::function<void(int)>> _onPositionUpdateHandlers;
        std::map<int,std::function<void(MotorStatus)>> _onMotorStatusUpdateHandlers;
        std::map<int,std::function<void(void)>> _onMotorCalibratedHandlers;
        MotorStatus _lastMotorStatus = MotorStatus::Idle;
        bool _lastCalibratedStatus = false;
};



class MotorMotionManager : public IMotorMotionManager {
    public :
        virtual ~MotorMotionManager(){}
        virtual std::string getId() const override {return "";};
        virtual void setPosition(int position) override {};
        int addOnPositionUpdatehandler(std::function<void(int)> onPositionUpdatehandler) override;
        int addOnMotorStatusUpdatehandler(std::function<void(MotorStatus)> onMotorStatusUpdatehandler) override;
        int addOnMotorCalibratedhandler(std::function<void(void)> onMotorCalibratedhandler) override;
    protected:
        void updateMotionData(MotorStatusData data);
};


class EmptyMotionManager: public IMotorMotionManager {
    public :
        EmptyMotionManager() {}
        ~EmptyMotionManager(){}
        void setHighSpeed() override {}
        void setLowSpeed() override {}
        int getLowSpeed() const override {return -1;}
        std::string getId() const override {return "EmptyMotionManager";};
        virtual bool isCalibrated() const override {return true;}
        virtual bool getIsConfigured() const override {return true;}
            

        void stop() override {}
        void close() override {}
        void open() override {};
        void setPosition(int position) override {}
        MotorStatusData getMotorStatusData() const override {MotorStatusData a; return a;}
        int getStroke() const override {return 0;}
        std::future<bool> clearCalibration() override {return std::async(std::launch::async,[](){return true;});}

        int addOnPositionUpdatehandler(std::function<void(int)> onPositionUpdatehandler) override {
            throw new std::logic_error("Empty motion manager has not motor position to change and doesn't need a handler!");
            return -1;
        }
        int addOnMotorStatusUpdatehandler(std::function<void(MotorStatus)> onMotorStatusUpdatehandler) override {
            throw new std::logic_error("Empty motion manager has no motor status to change and doesn't need a handler!");
            return -1;
        }
        int addOnMotorCalibratedhandler(std::function<void(void)> onMotorCalibratedhandler) override {
            throw new std::logic_error("Empty motion manager has no motor to calibrate and doesn't need a calibtraion handler!");
            return -1;
        };
    
};

#endif  //MOTORMOTIONMANAGER_Hlog
