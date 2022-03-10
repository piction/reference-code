#ifndef TESTWINGSTATUSPUBLISHER_H
#define TESTWINGSTATUSPUBLISHER_H

#include "pch.h"
#include "wingStatusPublisher.h"
#include "verifier.h"

class TestWingStatusPublisher : public IWingStatusPublisher, public Verifier
{
public:
    TestWingStatusPublisher()
    {
    }
    void publishCalibrateOpenFinished(bool success, std::string id) const
    {
        LOG_DEBUG("publishCalibrateOpenFinished:" + std::to_string(success) + " " + id);
        _commandsCalledBuffer.append("publishCalibrateOpenFinished,");
    }
    void publishCalibrateCloseFinished(bool success, std::string id) const
    {
        LOG_DEBUG("publishCalibrateCloseFinished:" + std::to_string(success) + " " + id);
        _commandsCalledBuffer.append("publishCalibrateCloseFinished,");
    }
    void publishCalibrated(std::string id) const
    {
        LOG_DEBUG("publishCalibrated:" + id);
        _commandsCalledBuffer.append("publishCalibrated,");
    }
    void publishCalibrateFailed(std::string id) const override
    {
        LOG_DEBUG("publishCalibrateFailed:" + id);
        _commandsCalledBuffer.append("publishCalibrateFailed,");
    }
    void publishCurrentMeasurmentFinished(std::string id) const
    {
        LOG_DEBUG("publishCurrrentMeasurementFinished:" + id);
        _commandsCalledBuffer.append("publishCurrentMeasurmentFinished,");
    }
    void publishCalibrationCleared(std::string id) const
    {
        LOG_DEBUG("publishCalibrationCleared:" + id);
        _commandsCalledBuffer.append("publishCalibrationCleared,");
    }
    void publishCalibratedCanceled(std::string id) const
    {
        LOG_DEBUG("publishCalibratedCanceled:" + id);
        _commandsCalledBuffer.append("publishCalibratedCanceled,");
    }
    void publishCalibrationOpenStarted(std::string id) const
    {
        LOG_DEBUG("publishCalibrationOpenStarted: " + id);
        _commandsCalledBuffer.append("publishCalibrationOpenStarted,");
    }
    void publishCalibrationCloseStarted(std::string id) const
    {
        LOG_DEBUG("publishCalibrationCloseStarted: " + id);
        _commandsCalledBuffer.append("publishCalibrationCloseStarted,");
    }
    void publishCurrentMeasurmentOpenStarted(std::string id) const
    {
        LOG_DEBUG("publishCurrentMeasurmentOpenStarted: " + id);
        _commandsCalledBuffer.append("publishCurrentMeasurmentOpenStarted,");
    }
    void publishCurrentMeasurmentCloseStarted(std::string id) const
    {
        LOG_DEBUG("publishCurrentMeasurmentCloseStarted: " + id);
        _commandsCalledBuffer.append("publishCurrentMeasurmentCloseStarted,");
    }
    void publishFullyOpen(std::string id) const
    {
        LOG_DEBUG("publishFullyOpen:" + id);
        _commandsCalledBuffer.append("publishFullyOpen,");
    }
    void publishFullyClosed(std::string id) const
    {
        LOG_DEBUG("publishFullyClosed:" + id);
        _commandsCalledBuffer.append("publishFullyClosed,");
    }
    void publishLastPostionPerc(std::string id, int percentage) const override {
        LOG_DEBUG("publishLastPostionPerc:" + id);
        _commandsCalledBuffer.append("publishLastPostionPerc,");
    } 
    void publishEmergency(std::string id) const
    {
        LOG_DEBUG("publishEmergency:" + id);
        _commandsCalledBuffer.append("publishEmergency,");
    }
    void publishNoMovementAllowed(std::string id) const {
        LOG_DEBUG("publishNoMovementAllowed:" + id);
        _commandsCalledBuffer.append("publishNoMovementAllowed,");
    }

    void setDelegateWingPublishOutput(std::function<void(MqttData)> delegatePublishOutput) override {}
};

#endif