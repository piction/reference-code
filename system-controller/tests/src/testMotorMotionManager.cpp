
#include "testMotorMotionManager.h"
#include "log.h"


TestMotorMotionManager::TestMotorMotionManager  (int fakeStroke) : _fakeStroke(fakeStroke) 
{
    SetFakeMotorStatus(MotorStatus::Idle);
}


void TestMotorMotionManager::setHighSpeed() {
    _commandsCalledBuffer.append("setHighSpeed,");
}
void TestMotorMotionManager::setLowSpeed() {
    _commandsCalledBuffer.append("setLowSpeed,");
}
int TestMotorMotionManager::getLowSpeed() const {    
    return 30;
}
bool TestMotorMotionManager::isCalibrated() const {
    LOG_DEBUG("Requested if isCalibrated on mockup")
    _commandsCalledBuffer.append("isCalibrated,");
    return _fakeStatus.isCalibrated;
}
bool TestMotorMotionManager::getIsConfigured() const {
    LOG_DEBUG("Requested if getIsConfigured on mockup")
    _commandsCalledBuffer.append("IsConfigured(,");
    return true;
}

void TestMotorMotionManager::stop() {
    _commandsCalledBuffer.append("stop,");
}
void TestMotorMotionManager::close() {
    _commandsCalledBuffer.append("close,");
}
void TestMotorMotionManager::open() {
    _commandsCalledBuffer.append("open,");
}
void TestMotorMotionManager::setPosition(int position) {
    _commandsCalledBuffer.append("setPosition,");
}
std::future<bool> TestMotorMotionManager::clearCalibration()  {
    _commandsCalledBuffer.append("clearCalibration");
    return std::async(std::launch::async,[](){return true;});
}
MotorStatusData TestMotorMotionManager::getMotorStatusData()  const {
    // can not add to the commandsbuffer because otherwise the function can not be const
    return _fakeStatus;
}
void TestMotorMotionManager::SetFakeMotorStatus( MotorStatus fakeStatus)  {
    if ( _fakeStatus.getStatus() == fakeStatus) {
        return;
    }

    switch (fakeStatus)
    {
    case MotorStatus::Closed:
        _fakeStatus.isClosed = true; _fakeStatus.isOpen = false; _fakeStatus.isEmergencyRun = false; _fakeStatus.speedMm=0;
        break;
    case MotorStatus::Open:
        _fakeStatus.isClosed = false; _fakeStatus.isOpen = true; _fakeStatus.isEmergencyRun = false; _fakeStatus.speedMm=0;        
        break;
    case MotorStatus::Moving:
        _fakeStatus.isClosed = false; _fakeStatus.isOpen = false; _fakeStatus.isEmergencyRun = false; _fakeStatus.speedMm=100;        
        break;
    case MotorStatus::Emergency:
        _fakeStatus.isClosed = false; _fakeStatus.isOpen = false; _fakeStatus.isEmergencyRun = true; _fakeStatus.speedMm=0;        
        break;
    default:
        _fakeStatus.isClosed = false; _fakeStatus.isOpen = false; _fakeStatus.isEmergencyRun = false; _fakeStatus.speedMm=0;        
        break; 
    }
    updateMotionData(_fakeStatus);  
}
void TestMotorMotionManager::SetFakeMotorStatusData( MotorStatusData data)  {
    _fakeStatus = data;
    updateMotionData(data);
}

void TestMotorMotionManager::updateWithFakePosition(int position) {
    for (auto & handler: _onPositionUpdateHandlers) {
        handler.second(position);    
    }
}




// ****** TestMotorMotionManagerFakeCalibration


TestMotorMotionManagerFakeCalibration::TestMotorMotionManagerFakeCalibration  (int fakeStroke) : TestMotorMotionManager(fakeStroke) 
{    
}

void TestMotorMotionManagerFakeCalibration::stop() {
    SetFakeMotorStatus(MotorStatus::Idle);
    _commandsCalledBuffer.append("stop,");
}
void TestMotorMotionManagerFakeCalibration::close() {
     std::this_thread::sleep_for(std::chrono::milliseconds(20));
    SetFakeMotorStatus(MotorStatus::Closed);
    isMovedCloseOnce=true;
    _fakeStatus.isCalibrated = isMovedCloseOnce && isMovedOpenOnce;
    _commandsCalledBuffer.append("close,");
}
void TestMotorMotionManagerFakeCalibration::open() {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    SetFakeMotorStatus(MotorStatus::Open);
    isMovedOpenOnce=true;
    _fakeStatus.isCalibrated = isMovedCloseOnce && isMovedOpenOnce;
    _commandsCalledBuffer.append("open,");
}
void TestMotorMotionManagerFakeCalibration::setPosition(int position) {
    SetFakeMotorStatus(MotorStatus::Moving);
    _commandsCalledBuffer.append("setPosition,");
}
std::future<bool> TestMotorMotionManagerFakeCalibration::clearCalibration()  {
    _commandsCalledBuffer.append("clearCalibration");
    _fakeStatus.isCalibrated =false;
    isMovedCloseOnce = false;
    isMovedOpenOnce = false;
    return std::async(std::launch::async,[](){return true;});
}


