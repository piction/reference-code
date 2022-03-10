#include "wingStatusPublisher.h"
#include "log.h"

WingStatusPublisher::WingStatusPublisher(const std::string & configId) : _configId(configId) {

}
void WingStatusPublisher::publishCalibrateOpenFinished(bool success, std::string id) const {
    if ( success)  {
        publishMessage("calibrateOpenFinishedSuccess",id);
    } else {
        publishMessage("calibrateFail",id);
    }
}
void WingStatusPublisher::publishCalibrateCloseFinished(bool success, std::string id) const {
    if ( success)  {
        publishMessage("calibrateCloseFinishedSuccess",id);
    } else {
        publishMessage("calibrateFail",id);
    }
}
void WingStatusPublisher::publishCalibrateFailed(std::string id) const {
    publishMessage("calibrateFail",id);
}
void WingStatusPublisher::publishCalibrated(std::string id) const {
    publishMessage("calibrationDone",id);
}

void WingStatusPublisher::publishCurrentMeasurmentFinished(std::string id) const {
    publishMessage("currentMeasurementDone",id);
}
void WingStatusPublisher::publishCalibratedCanceled(std::string id) const {
    publishMessage("calibrationCanceled",id);
}

void WingStatusPublisher::publishCalibrationCleared(std::string id) const {
    publishMessage("calibrationCleared",id);
}

void WingStatusPublisher::publishCalibrationOpenStarted(std::string id) const {
    publishMessage("calibrationOpenStarted",id);
}
void WingStatusPublisher::publishCalibrationCloseStarted(std::string id) const {
    publishMessage("calibrationCloseStarted",id);
}
void WingStatusPublisher::publishCurrentMeasurmentOpenStarted(std::string id) const {
    publishMessage("currentMeasurmentOpenStarted",id);
}
void WingStatusPublisher::publishCurrentMeasurmentCloseStarted(std::string id) const {
    publishMessage("currentMeasurmentCloseStarted",id);
}
void WingStatusPublisher::publishNoMovementAllowed(std::string id) const {
    publishMessage("noMovementAllowed",id);
}

void WingStatusPublisher::publishFullyOpen(std::string id) const {
    publishMessage("fullyOpen",id);
}
void WingStatusPublisher::publishFullyClosed(std::string id) const {
    publishMessage("fullyClose",id);
}
void WingStatusPublisher::publishLastPostionPerc(std::string id, int percentage) const { 
    if(!_delegatePublishOutputSet) {
        LOG_WARNING("Can not publish wing message due no publish handler set");
        return;
    }    
    std::string json ="{";
    json.append("\"positionperc\":");
    json.append("\""); json.append(std::to_string(percentage)); json.append("\"");
    json.append("}");
  MqttData data( "systemcontroller/"+ _configId + "/wing/" + id + "/position",json);
    _delegatePublishOutput(data);
};
void WingStatusPublisher::publishEmergency(std::string id)const {
    publishMessage("Emergency",id);
}

void WingStatusPublisher::publishMessage(std::string message ,std::string id) const {
    if(!_delegatePublishOutputSet) {
        LOG_WARNING("Can not publish wing message due no publish handler set");
        return;
    }
    LOG_DEBUG("==> Publishing message [" + id + "] :" + message );
    std::string json ="{";
    json.append("\"status\":");
    json.append("\""); json.append(message); json.append("\"");
    json.append("}");
    MqttData data( "systemcontroller/"+ _configId + "/wing/" + id + "/info",json);
    _delegatePublishOutput(data);
}



void WingStatusPublisher::setDelegateWingPublishOutput(std::function<void(MqttData)> delegatePublishOutput) {
    _delegatePublishOutput = delegatePublishOutput;
    _delegatePublishOutputSet = true;
}