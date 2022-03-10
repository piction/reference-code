#ifndef MQTTMOTORMANAGER_H
#define MQTTMOTORMANAGER_H


#include "pch.h"

#include "motorMotionManager.h"
#include "motorData.h"
#include "mqttData.h"
#include "commandsManager.h"

class IMqttMotor {
    public:     
        virtual ~IMqttMotor() {}   ;
        virtual void onMotorInput(const MotorData & data) = 0;
        virtual void onMotorConnected() = 0;
        virtual void onMotorDisconnected() = 0; 
        virtual void setDelegateMotorOutput(std::function<void(MqttData)> delegateMotorOutput)=0;
        virtual std::string getId() const =0;    
        
};

class MqttMotor: public MotorMotionManager, public IMqttMotor {
    public :
        MqttMotor(const std::string& pn,const std::string& serial);
        ~MqttMotor();
        void setHighSpeed() override;
        void setLowSpeed() override ;
        

        void stop() override;
        void close() override;
        void open() override ;    
        
        void setPosition(int position) override;        
        void onMotorInput(const MotorData & data) override;
        void onMotorConnected() override;
        void onMotorDisconnected() override;
               
        void setDelegateMotorOutput(std::function<void(MqttData)> delegateMotorOutput) override;
        std::string getId() const override {return _id;}
        int getLowSpeed() const  {return _lowSpeed;}
        bool getIsMotorStopped() const {return _isMotorStopped;}
        bool isCalibrated() const override {return _currentMotorStatusData.isCalibrated;}
        bool getIsConfigured() const {return _isMotorConfigured;}
        int getStroke() const override { return isCalibrated() ?  _stroke : -1 ;}
        
        
        std::future<bool> clearCalibration() override ;
        void cancelAsyncTasks();
 
        MotorStatusData getMotorStatusData() const override {return _currentMotorStatusData;}
    protected:
        std::shared_ptr<CommandsManager> _commandsManager;
        // Mqtt related members
        std::string _baseTopic;
        std::string _id;
        int _stroke = -1;
    private:
        bool clearCalibrationWorker(std::future<void> cancelObj);
       
        std::function<void(MqttData)> _delegateMotorOutput;
        bool _motorDelegateOutputIsSet = false;

        // motor relatede members
        int _lowSpeed=0;
        int _highSpeed=0;
        int _currentTargetSpeed =0;
        bool _isMotorConfigured=false;
        bool _isMotorStopped =false;
        MotorStatusData _currentMotorStatusData;
        void limitSpeedIfNeeded();
        

        // worker related members
        std::condition_variable _cv;
        bool _pollingActive = false;
        std::thread _workerThread;
        std::mutex _mutex;

        std::promise<void> _cancelWorkerSignal;

        // send related messages        
        void polStatus();
        
    
};

#endif  //MOTORMOTIONMANAGER_H

//systemcontroller/motor/#