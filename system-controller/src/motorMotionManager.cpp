
#include "motorMotionManager.h"


void MotorMotionManager::MotorMotionManager::updateMotionData(MotorStatusData data){
    
    if ( data.getStatus() != _lastMotorStatus) {
        _lastMotorStatus = data.getStatus();
        for (auto & handler: _onMotorStatusUpdateHandlers) {
            handler.second(_lastMotorStatus);    
        }
    }
    // only call onCalibrated when stroke is already returned 
    if ( data.isCalibrated && !_lastCalibratedStatus && getStroke() > 0) {
        _lastCalibratedStatus = data.isCalibrated; 
        for (auto & handler : _onMotorCalibratedHandlers) {
            handler.second();
        }
    }
    // only reset when it was set before 
    if ( !data.isCalibrated && _lastCalibratedStatus) {
        _lastCalibratedStatus = data.isCalibrated; 
    }
    
    for (auto & handler: _onPositionUpdateHandlers) {
        handler.second(data.posMm);  
    }
}

int MotorMotionManager::addOnPositionUpdatehandler(std::function<void(int)> onPositionUpdatehandler) {
    int id=-1;
    int counter =1;
    do {
        counter++;
        id = counter;
        for (auto const& x : _onPositionUpdateHandlers)
        {
            if ( id == x.first) {
                id = -1; 
                break;
            }        
        }
    } while (id < 0); // do until new unique id is found
    _onPositionUpdateHandlers.insert(std::pair<int,std::function<void(int)>>(id,onPositionUpdatehandler));
    return id;
}

 int MotorMotionManager::addOnMotorStatusUpdatehandler(std::function<void(MotorStatus)> onMotorStatusUpdatehandler) {
    int id=-1;
    int counter =1;
    do {
        counter++;
        id = counter;
        for (auto const& x : _onMotorStatusUpdateHandlers)
        {
            if ( id == x.first) {
                id = -1; 
                break;
            }        
        }
    } while (id < 0); // do until new unique id is found
    _onMotorStatusUpdateHandlers.insert(std::pair<int,std::function<void(MotorStatus)>>(id,onMotorStatusUpdatehandler));
    return id;
 }

 int MotorMotionManager::addOnMotorCalibratedhandler(std::function<void(void)> onMotorCalibratedhandler)  {
    int id=-1;
    int counter =1;
    do {
        counter++;
        id = counter;
        for (auto const& x : _onMotorCalibratedHandlers)
        {
            if ( id == x.first) {
                id = -1; 
                break;
            }        
        }
    } while (id < 0); // do until new unique id is found
    _onMotorCalibratedHandlers.insert(std::pair<int,std::function<void(void)>>(id,onMotorCalibratedhandler));
    return id;
 }