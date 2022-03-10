#ifndef WINGSTATUSPUBLISHER_H
#define WINGSTATUSPUBLISHER_H

#include "pch.h"
#include "mqttData.h"

class IWingStatusPublisher {
    public:
    virtual ~IWingStatusPublisher(){};
    virtual void publishCalibrateOpenFinished(bool success,std::string id) const = 0;
    virtual void publishCalibrateCloseFinished(bool success, std::string id) const= 0;
    virtual void publishCalibrated(std::string id) const =0;
    virtual void publishCurrentMeasurmentFinished(std::string id) const =0;
    virtual void publishCalibrateFailed(std::string id) const=0;
    virtual void publishCalibrationCleared(std::string id) const =0;
    virtual void publishCalibratedCanceled(std::string id) const =0;
    virtual void publishCalibrationOpenStarted(std::string id) const =0;
    virtual void publishCalibrationCloseStarted(std::string id) const =0;
    virtual void publishCurrentMeasurmentOpenStarted(std::string id) const =0;
    virtual void publishCurrentMeasurmentCloseStarted(std::string id) const =0;
    virtual void publishNoMovementAllowed(std::string id) const =0;

    virtual void publishFullyOpen(std::string id) const= 0;
    virtual void publishFullyClosed(std::string id) const= 0;    
    virtual void publishLastPostionPerc(std::string id, int percentage) const =0;
    virtual void publishEmergency(std::string id) const =0;
    virtual void setDelegateWingPublishOutput(std::function<void(MqttData)> delegatePublishOutput)= 0;
};


class WingStatusPublisher : public IWingStatusPublisher {
    
    public:
    WingStatusPublisher(const std::string & configId);
    ~WingStatusPublisher(){};
    void publishCalibrateOpenFinished(bool success,std::string id) const override;
    void publishCalibrateCloseFinished(bool success, std::string id) const override;
    void publishCalibrated(std::string id) const override;
    void publishCalibrateFailed(std::string id) const override;
    void publishCurrentMeasurmentFinished(std::string id) const override;
    void publishCalibrationCleared(std::string id) const override;
    void publishCalibratedCanceled(std::string id) const override;
    void publishCalibrationOpenStarted(std::string id) const override;
    void publishCalibrationCloseStarted(std::string id) const override;
    void publishCurrentMeasurmentOpenStarted(std::string id) const override;
    void publishCurrentMeasurmentCloseStarted(std::string id) const override;
    void publishNoMovementAllowed(std::string id) const override;
    void publishFullyOpen(std::string id) const override;
    void publishFullyClosed(std::string id) const override;
    void publishLastPostionPerc(std::string id, int percentage) const override;
    void publishEmergency(std::string id) const override;
    
    void setDelegateWingPublishOutput(std::function<void(MqttData)> delegatePublishOutput) override;
private:
    std::string _configId;
    void publishMessage(std::string message , std::string id) const ;    
    std::function<void(MqttData)> _delegatePublishOutput;
    bool _delegatePublishOutputSet = false;
};




#endif //WINGSTATUSPUBLISHER_H