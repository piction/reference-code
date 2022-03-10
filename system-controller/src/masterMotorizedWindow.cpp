#include "masterMotorizedWindow.h"
#include <stdexcept>
#include <utility>
#include "log.h"

MasterMotorizedWindow::MasterMotorizedWindow(  int length, std::shared_ptr<IMotorMotionManager> motionManager) 
: MotorizedWindow(length, std::move(motionManager)) , _target(0) , _targetType(TargetType::None)  {
    
    getMotionManager()->addOnMotorCalibratedhandler([&]() { updatePanelLengtsBasedOnstroke();});
}

MasterMotorizedWindow::~MasterMotorizedWindow() {
    stopCalibrationWorkers();
}
void MasterMotorizedWindow::setTarget(int newTarget){
    _target=newTarget;
    // if strok is valid make sure that the target is not passed the stroke otherwise it is never reachable
    if(_motionManager->getStroke() > 0 ) {
        _target = std::min(newTarget,_motionManager->getStroke());
    }
    
    //LOG_DEBUG("new target for motor " + _motionManager->getId() + " @ " + std::to_string(newTarget));
    _target = std::max(0,_target);// make sure the target is positive
    _targetType = TargetType::Position;
    onPositionUpdate(getPosition());
}
void MasterMotorizedWindow::stopWindow() {
    //LOG_DEBUG("STOP window for motor " + _motionManager->getId() );
    stopCalibrationWorkers();
    MotorizedWindow::stopWindow();
}
void MasterMotorizedWindow::stopCalibrationWorkers() {
    if( _calibrateBusy) {        
        LOG_WARNING("Running calibration will be canceled");
        _cancelWorkerCalibCloseSignal.set_value(); //set void value to flag a cancel    
        _cancelWorkerCalibOpenSignal.set_value();
        _cancelWorkerCalibCloseSignal = std::promise<void>();
        _cancelWorkerCalibOpenSignal = std::promise<void>();
    } 
}
std::future<bool> MasterMotorizedWindow::calibrateOpen(){
    stopWindow();
    _targetType = TargetType::CalibrateStrokeOpen;
    LOG_INFO("calibrateOpen async worker started");
    _cancelWorkerCalibOpenSignal = std::promise<void>();
    std::future<void> cancelObj = _cancelWorkerCalibOpenSignal.get_future();    
    return std::async(std::launch::async, & MasterMotorizedWindow::calibrateWorker,this,std::move(cancelObj));
}
std::future<bool> MasterMotorizedWindow::calibrateClose() {
    stopWindow();
    _targetType = TargetType::CalibrateStrokeClose;
    LOG_INFO("calibrateClose async worker started");
    _cancelWorkerCalibCloseSignal = std::promise<void>();
    std::future<void> cancelObj = _cancelWorkerCalibCloseSignal.get_future();    
    return std::async(std::launch::async, &MasterMotorizedWindow::calibrateWorker,this,std::move(cancelObj));
}
void MasterMotorizedWindow::open() {      
    //LOG_DEBUG("OPEN for motor " + _motionManager->getId() );
    if( _targetType == TargetType::Open)  return;    
    stopWindow();
    _target = std::numeric_limits<int>::max();    
    _targetType = TargetType::Open; 
    onPositionUpdate(getPosition());
}
void MasterMotorizedWindow::close() {
    //LOG_DEBUG("CLOSE for motor " + _motionManager->getId() );
    if( _targetType == TargetType::Close)  return;
    stopWindow();
    _target=0;
    _targetType = TargetType::Close ;
    onPositionUpdate(getPosition());
}
void MasterMotorizedWindow::stop(){
    //LOG_DEBUG("STOP for motor " + _motionManager->getId() );
    stopWindow();        
    _targetType = TargetType::None;
}
bool MasterMotorizedWindow::isOntarget() {
    if ( _targetType == TargetType::Open && _motionManager->getMotorStatusData().getStatus() == MotorStatus::Open) return true;
    if ( _targetType == TargetType::Close && _motionManager->getMotorStatusData().getStatus() == MotorStatus::Closed) return true;
    if ( _targetType == TargetType::None) return true;
    return false;
}

void MasterMotorizedWindow::manageOperationalMovement() {
    switch ( _targetType) {
        case TargetType::Position: {                
                if ( std::abs(_target -_position) < getAllowedPositionTolerance() ) {
                    _pushType = PushType::Stop;
                } else {
                    _pushType =  ( _target > _position) ? PushType::PushToOpen : PushType::PushToClose;
                } 
                break;
        }
        case TargetType::Open: {_pushType = PushType::PushToOpen;break;}
        case TargetType::Close: { _pushType = PushType::PushToClose;break;}
        default: {_pushType = PushType::Stop;break;}
    }
    pushSlavesToAllowMovement(); 


    if( _freeToMove == MovementFreedom::None || _pushType == PushType::Stop) {
        _motionManager->stop();
    }else {
      
        if ( _freeToMove == MovementFreedom::Fast) {
            _motionManager->setHighSpeed();
        } else {
            _motionManager->setLowSpeed();
        }
        switch ( _targetType) {
            case TargetType::Position: {  _motionManager->setPosition(_target); break;}
            case TargetType::Open: {  _motionManager->open(); break;}
            case TargetType::Close: {  _motionManager->close(); break;}
            default:  {  _motionManager->stop(); break;}
        }
    }
}


// This function will manage the calibration procedure either open or close
// The scheduling of the slaves is done time-based 
// The time between the starting of the slaves to prevent hitting each other is calculated based on the slow speed of the master
// The assumption is made that all motors have the same or at least simular slow speeds
// On the end the function call a callback with work that can be done when the calibration is finished (for example calibrate in other direction)
bool MasterMotorizedWindow::calibrateWorker(std::future<void> cancelObj) {
    std::string calibDirectionStr = ((_targetType == TargetType::CalibrateStrokeOpen) ? "open" :  "close");
    LOG_DEBUG("Calibrate worker started :" + calibDirectionStr + " after delay");
    // delay action on the motor because when the motor receives to quick a new command it will not respond
    for ( int i =0; i < 10 ; i++) {
        if(cancelObj.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
            LOG_INFO("cancel calibration worker " + calibDirectionStr);
            _calibrateBusy=false;
            return false; //failed to calibrate
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // todo -> make sure that all motors are status-polling !!!    
    // this to prevent problems like :  _motionManager->getLowSpeed() beeeing zero and later in code devide by zero as result
    
    _calibrateBusy=true;
    auto start = std::chrono::system_clock::now();
    //calculate the delay between the start of each slave
    int interSlaveCalibrationSeconds = interSlaveCalibrationDistance / _motionManager->getLowSpeed();   
    int maxCalibratinSeconds = maximumStrokeForCalibration / _motionManager->getLowSpeed();
    auto slavesToCheck = std::make_shared<std::vector<std::shared_ptr<IMovingWindow>>>(getSlaves());
    if ( slavesToCheck->size() > 0) {
        LOG_DEBUG("Interslave distance in seconds:" + std::to_string(interSlaveCalibrationSeconds) + "s");
    }
    bool allSlavesStarted = slavesToCheck->empty(); // when empty no slaves present and so all slaves are started
    // slaves of slaves needs to be checked as well
    auto slavesToCheckNext = std::make_shared<std::vector<std::shared_ptr<IMovingWindow>>>(std::vector<std::shared_ptr<IMovingWindow>>() );

    _motionManager->setLowSpeed();
    if (_targetType == TargetType::CalibrateStrokeClose) {
        _pushType= PushType::ForceCloseForCalibration;
         pushSlavesToAllowMovement(); // notify the slaves to be in calibration mode
        _motionManager->close();
    } else if ( _targetType == TargetType::CalibrateStrokeOpen) {
        _pushType= PushType::ForceOpenForCalibration;
        _motionManager->open();
         pushSlavesToAllowMovement(); // notify the slaves to be in calibration mode
    } else {
        throw  std::invalid_argument( "A non calibration target type was given!" );
    }
    
        
    int currentSlaveLevel = 0;
    do {
        // make sure that the caibration can be cancelled by checking the cancel-signal
        if(cancelObj.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
            LOG_INFO("cancel calibration worker " + calibDirectionStr);
            _calibrateBusy = false;
            return false; //failed to calibrate
        }
        auto timeSinceStart =  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count(); 
        // check if calibration doesn't take to long due to failure 
        if (timeSinceStart > (maxCalibratinSeconds*1000) ) {
            LOG_INFO("Failed to complete the calibration within for seen time " + std::to_string(maxCalibratinSeconds) + "seconds");
            _calibrateBusy =false;
            return false;
        }

        // see how much time is passed and wich level of slaves should be running     
        int slaveLevelDepthToMove = floor(timeSinceStart/(interSlaveCalibrationSeconds*1000));
        
        // once a certain time is passed start all slaves of the next level needs to be started
        if( !allSlavesStarted && (slaveLevelDepthToMove > currentSlaveLevel) ) {
            currentSlaveLevel = slaveLevelDepthToMove;
            slavesToCheckNext->clear();
            for (auto & s  : *slavesToCheck) {
                LOG_DEBUG("Started new slave for calibration " + calibDirectionStr);
                s->getMotionManager()->setLowSpeed();
                if (_targetType == TargetType::CalibrateStrokeClose) {
                    s->getMotionManager()->close();
                } else {
                    s->getMotionManager()->open();
                }
                auto nextSlaves = s->getSlaves();
                slavesToCheckNext->insert(slavesToCheckNext->end(), nextSlaves.begin(), nextSlaves.end());
                if(slavesToCheckNext->empty()) {
                    allSlavesStarted =true;
                }            
            }            
            std::swap(slavesToCheck,slavesToCheckNext);            
            // build new slaves to check on next level     
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // check if all motors are on target position (either fully open or fully closed)
        if (allSlavesStarted) {   
            if (_targetType == TargetType::CalibrateStrokeClose) {                             
                _calibratedClose = (_motionManager->getMotorStatusData().getStatus() == MotorStatus::Closed);
                _calibrateBusy = !_calibratedClose;
            } else {
                _calibratedOpen = (_motionManager->getMotorStatusData().getStatus() == MotorStatus::Open);
                _calibrateBusy = !_calibratedOpen;
            }           
        }
    }while(_calibrateBusy); 
    _calibrateBusy =false;
    return true;
}


void MasterMotorizedWindow::clearCalibration() { 
    LOG_DEBUG("Clearing calibration master motorized window");
     stopCalibrationWorkers();
     _motionManager->clearCalibration().get();
     // clear all motors of the master
     for ( auto &s : getSlaveMotors() ) {
         s->clearCalibration();
     }
}

void MasterMotorizedWindow::onPositionUpdate(int newPosition) {
    _position = newPosition;
    if (_targetType != TargetType::CalibrateStrokeOpen && _targetType != TargetType::CalibrateStrokeClose)  {
        manageOperationalMovement();
    } 
}

void recursiveGetSlavesCount (int & total , std::vector<std::shared_ptr<IMovingWindow>> slave) {
    total += slave.size();
    for ( auto &s : slave) {
        recursiveGetSlavesCount(total,s->getSlaves());
    } 
}
void MasterMotorizedWindow::updatePanelLengtsBasedOnstroke() {
    int stroke = getMotionManager()->getStroke();
    int totalOfSlaves = 0;
    recursiveGetSlavesCount(totalOfSlaves,getSlaves()); 
    updateAllPanelLenghts(stroke/(totalOfSlaves + 1));

    // todo change when use of custom panel lengths
    _positionOfTailWhenFullyOpen=stroke/(totalOfSlaves + 1);
}

int MasterMotorizedWindow::getTailDistanceToFullyClose() {
    if ( _positionOfTailWhenFullyOpen < 1) {
        LOG_ERROR("tail position cannot be calculated when the _positionOfTailWhenFullyOpen is not set");
        return 0;
    }
    std::vector<std::shared_ptr<IMovingWindow>> slaves = getSlaves();
    if ( slaves.empty()) {
        return getMotionManager()->getStroke() - getPosition();
    }

    int posOfLastSlave = 0;
    while (!slaves.empty()) {
        posOfLastSlave = slaves[0]->getPosition();
        slaves =  slaves[0]->getSlaves(); // todo change slaves to be only one in stead of array!!
    }
    return _positionOfTailWhenFullyOpen - posOfLastSlave;
}