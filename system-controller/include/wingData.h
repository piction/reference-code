#ifndef WINGDATA_H
#define WINGDATA_H

#include "pch.h"


#include "mqttData.h"

enum class WingComandType {
    Stop,
    Open,           // always cmd open 
    OpenOrStop,     // when moving open this command will stop the window, when stopped the cmd will open the window
    Close,
    CloseOrStop,    // when moving closing this command will stop the window, when stopped the cmd will close the window
    Calibrate,
    Cancel,
    Lock,
    SetPosition,
    Pulse,          // when prevsious command was close, cmd open 
    PulseOrStop,    // when previous cmd was moving stop otherwise use pulse startegy
    Ignore
};

class IWingData {
    public:
    virtual ~IWingData() {}
    virtual std::string getId() const =0;
    virtual WingComandType getWingCommand( ) const =0;
    virtual void getPosition(double & perc, int & mm ) =0;
    virtual MqttData getAck()=0;
};

class WingData : public IWingData {
    public:
        WingData(const MqttData &mqttData);
        std::string getId()const  override;
        WingComandType getWingCommand() const override;
        void getPosition(double & perc, int & mm ) override;
        MqttData getAck() override;
        
    private:
        MqttData _mqttData;
        std::string _id;
        WingComandType _wingCommand;
        double _positionPerc = -1;
        int _positionMm = -1;
        bool _positionSet = false;
        
        void parsePayload(const std::string& payload);
};

#endif //WINGDATA_H