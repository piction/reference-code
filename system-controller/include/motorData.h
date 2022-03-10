#ifndef MOTORDATA_H
#define MOTORDATA_H

#include "pch.h"

#include "mqttData.h"

enum class MotorStatus {
    Idle,
    Moving,
    Closed,
    Open,
    Emergency
};

struct MotorStatusData {
    int posMm =0;
    int speedMm=0;
    bool isLocked=false; 
    bool isOpen=false; 
    bool isClosed=false;
    bool isCalibrated=false;
    bool isEmergencyRun=false;

    bool isMotorStopped() const { return isLocked || isClosed || isOpen || (speedMm <= 1);}    

    MotorStatus getStatus() {
        if (isClosed) {return  MotorStatus::Closed;}
        if (isOpen) {return  MotorStatus::Open;}
        if( speedMm > 0) {return   MotorStatus::Moving;}
        if (isEmergencyRun) {return MotorStatus::Emergency;}
        return MotorStatus::Idle;
    }
};


class IMotorData {
    public:
    virtual ~IMotorData(){}
    virtual std::string getId() const =0;
    virtual std::string getCommand() const =0;    
    virtual MqttData getMqttData() const =0;
};

class MotorData : public IMotorData {
    public:
        MotorData(const MqttData &mqttData);
        std::string getId()const  override;
        std::string getCommand() const {return _command;}       
        bool hasInfo() const {return _hasInfo;} 
        bool isAck() const {return _isAck;} 
        void parseOneValue(int & value) const;
        void parseStatusData(MotorStatusData & motorStatus) const ;
        MqttData getMqttData() const override {return _mqttData;}   

    private:
        MqttData _mqttData;
        std::string _id;
        std::string _command;
        bool _hasInfo;
        bool _isAck;
    };

#endif //WINGDATA_H