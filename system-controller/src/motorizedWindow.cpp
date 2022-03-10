#include "motorizedWindow.h"
#include "log.h"
#include <exception>
#include <utility>


MotorizedWindow::MotorizedWindow(  int length, std::shared_ptr<IMotorMotionManager> motionManager) 
: MovingWindow(length, std::move(motionManager))
    {
       
         // set position update handler 
        this->_motionManager->addOnPositionUpdatehandler([&](int pos) {onPositionUpdate(pos);});
    }

void MotorizedWindow::stopWindow() {
    for ( auto  & slaveInfo : _slaves ) {
        auto slave = std::get<0>(slaveInfo);
        slave->stopWindow();
    }    
    push(PushType::Stop);
}

void MotorizedWindow::push(PushType push){
    _pushType = push;
    if ( push == PushType::Stop || _freeToMove == MovementFreedom::None) {
        _motionManager->stop();
    } else {
        // Slow down if needed
        if (_freeToMove == MovementFreedom::Slow || push == PushType::ForceCloseForCalibration || push == PushType::ForceOpenForCalibration) {
            _motionManager->setLowSpeed();
        } else if (_freeToMove == MovementFreedom::Fast ) {
            _motionManager->setHighSpeed();
        }
        // command to open or close (pay attention , when calibration mode no commands are given here, it is done directly by the masterwindow)
        if ( push == PushType::PushToClose ) {
            _motionManager->close();
        } else if ( push == PushType::PushToOpen ) {
            _motionManager->open();
        } else if ( push == PushType::ForceCloseForCalibration || push == PushType::ForceOpenForCalibration){
            // ignore -> commands are given directly from masterwindow
        } else {
            throw std::logic_error("Unhandled pushType");
        }
    }
}


void MotorizedWindow::clearCalibration() { 
    LOG_DEBUG("_clear calib motorized window called_");    
    _motionManager->clearCalibration().get();
    LOG_DEBUG("_cleared calib motorized window called_");
}

std::vector<std::shared_ptr<MotorizedWindow>> MotorizedWindow::getSlaveMotors() const {
    std::vector<std::shared_ptr<MotorizedWindow>> motors;
      for (auto & s : getSlaves()){        
        if (auto item = std::dynamic_pointer_cast<MotorizedWindow>(s)) {
            motors.push_back(item);
        } 
        getMotorsRecursive(motors,s);
    }
    return motors;
}
void MotorizedWindow::getMotorsRecursive(std::vector<std::shared_ptr<MotorizedWindow>> & motors, std::shared_ptr<IMovingWindow> curr) const{
    for (auto & s : curr->getSlaves()){        
        if (auto item = std::dynamic_pointer_cast<MotorizedWindow>(s)) {
            motors.push_back(item);
        } 
        getMotorsRecursive(motors,s);
    }
}
