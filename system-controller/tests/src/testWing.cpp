
#include "testWing.h"
#include "testMotorMotionManager.h"
#include "log.h"

TestWing::TestWing(int fakeStroke, std::string wingName): 
    _fakeStroke(fakeStroke) ,
     _currentCornerPushZone(std::make_shared<WindowPushZone>()),
     _currentOppositePushZone(std::make_shared<WindowPushZone>())     {    
    _wingName = wingName;
    _motors.push_back (std::make_shared<MasterMotorizedWindow>(2000, std::make_shared<TestMotorMotionManager>(4000)));        
}

void TestWing::addSibling(std::shared_ptr<IWing> sibling, WingSiblingType siblingType){
    LOG_DEBUG("Added sibling ");
    _commandsCalledBuffer.append("addSibling,");
    // todo add implementaion of adding a sibling 
}
void TestWing::open() {
    _fakePos=_fakeStroke;
    _wasLastMovementOpening= true;
    _commandsCalledBuffer.append("open,");
}
void TestWing::close() {
    _fakePos=0;
    _wasLastMovementOpening= false;
    _commandsCalledBuffer.append("close,");
}
void TestWing::stop() {
    _commandsCalledBuffer.append("stop,");
}
void TestWing::setPositionMm(int positionMm) {
    _fakePos=positionMm;
    _commandsCalledBuffer.append("setPositionMm,");
}
void TestWing::setPositionPerc(double positionPerc) {
    _fakePos=positionPerc * 100;
    _commandsCalledBuffer.append("setPositionPerc,");
}
int TestWing::getPosition() {
    _commandsCalledBuffer.append("getPosition,");
    return _fakePos;
}
int TestWing::getTarget() {
    _commandsCalledBuffer.append("getTarget,");
    return _fakePos;
}
void TestWing::updateWingMovement(){
    _commandsCalledBuffer.append("updateWingMovement,");
}
std::shared_ptr<MasterMotorizedWindow> TestWing::getMasterWindow() const {
   // not possible to fill the commandsbuffer due const restriction
    return std::dynamic_pointer_cast<MasterMotorizedWindow>(_motors[0]);
}

const std::vector<std::shared_ptr<MotorizedWindow>> TestWing::getMotors() const {   
    return _motors;
}


const std::shared_ptr<std::vector<std::tuple<std::shared_ptr<IWing>,WingSiblingType>>> TestWing::getSiblings() const {
    return std::make_shared<std::vector<std::tuple<std::shared_ptr<IWing>,WingSiblingType>>>();
}
void TestWing::startCalibrate() {
    _commandsCalledBuffer.append("startCalibrate");    
}