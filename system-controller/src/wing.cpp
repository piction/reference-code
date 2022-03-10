#include "wing.h"

#include <utility>
#include "log.h"

#define LOG_WING_CRITICAL(...) LOG_CRITICAL("wing " + this->getWingId() + ":" + __VA_ARGS__);
#define LOG_WING_CRITICAL_THROW(...) LOG_CRITICAL_THROW("wing " + this->getWingId() + ":" + __VA_ARGS__);
#define LOG_WING_ERROR(...) LOG_ERROR("wing " + this->getWingId() + ":" + __VA_ARGS__);
#define LOG_WING_WARNING(...) LOG_WARNING("wing " + this->getWingId() + ":" + __VA_ARGS__);
#define LOG_WING_DEBUG(...) LOG_DEBUG("wing " + this->getWingId() + ":" + __VA_ARGS__);
#define LOG_WING_INFO(...)  LOG_INFO("wing " + this->getWingId() + ":" + __VA_ARGS__);
#define LOG_WING_TRACE(...) LOG_TRACE("wing " + this->getWingId() + ":" + __VA_ARGS__);

int Wing::instanceCounter = 0;

Wing::Wing(std::shared_ptr<MasterMotorizedWindow> masterWindow, 
            std::shared_ptr<WingRelationManager> WingRelationManager, 
            std::shared_ptr<IWingStatusPublisher> wingStatusPublisher, 
            //const std::string & wingName,
            bool hasOpeningLeft) 
: _masterWindow(std::move(masterWindow))
, _hasOpeningLeft(hasOpeningLeft)
, _wingRelationManager(std::move(WingRelationManager))
, _wingStatusPublisher(std::move(wingStatusPublisher))
, _siblings(std::make_shared<std::vector<std::tuple<std::shared_ptr<IWing>,WingSiblingType>>>())
 {  
     instanceCounter++;
    _wingName = "wing" + std::to_string(instanceCounter);    
    LOG_WING_INFO("Created wing");
   
    _currentWingStatus= std::make_shared<WingStatus>(0,0,WingTarget::Idle,false,false,std::make_shared<WindowPushZone>(),std::make_shared<WindowPushZone>());
    _lastWingStatus = std::make_shared<WingStatus>(_currentWingStatus);


    LOG_WING_TRACE("Register position handler of masterwindow on wing");
    _masterWindow->getMotionManager()->addOnPositionUpdatehandler([&](int pos) {
        _currentWingStatus->updatePosition(pos);
        updateWingMovement();
        // only update position when moving
        if (_isFullSetupCalibDone && _masterWindow->getMotionManager()->getMotorStatusData().getStatus() == MotorStatus::Moving) {
            double wingperc = (100.0*((double) _masterWindow->getPosition()) /(double) _masterWindow->getMotionManager()->getStroke());
            _wingStatusPublisher->publishLastPostionPerc(_wingName, wingperc);            
        }
    });

    _masterWindow->getMotionManager()->addOnMotorStatusUpdatehandler([&](MotorStatus newStatus) {
        switch (newStatus) {
        case MotorStatus::Closed:
            _wingStatusPublisher->publishFullyClosed(_wingName);
            break;
        case MotorStatus::Open:
            _wingStatusPublisher->publishFullyOpen(_wingName);
            break;
        case MotorStatus::Emergency: {
            _wingStatusPublisher->publishEmergency(_wingName);
            stop();
            for (auto& wingInfo : *_siblings) {
                std::get<0>(wingInfo)->stop();
            }
            break;
        }
        default:
            break;
        };
    });
}
void Wing::addSibling(std::shared_ptr<IWing> sibling, WingSiblingType siblingType) {
    // first check if Wing is not already in the list
    bool passedAlreadyInsertedSibling = false;
    for ( auto & wingInfo : *_siblings)  {
         if (sibling->getWingId().compare(std::get<0>(wingInfo)->getWingId()) == 0) {
             LOG_WING_WARNING("WingId " + sibling->getWingId() + " is already in the sibling list!");
            passedAlreadyInsertedSibling = true;
            break;
         }
    }
    if ( ! passedAlreadyInsertedSibling ) {
        auto s =  std::make_tuple(sibling, siblingType);
        _siblings->push_back(s); 
    }
}
void Wing::setDelegateWingPublishOutput(std::function<void(MqttData)> delegatePublishOutput) {
    _wingStatusPublisher->setDelegateWingPublishOutput(delegatePublishOutput);
}

void Wing::open() {    
    if ( _isFullSetupCalibDone || WingCalibrationHandler::areAllWingsCalibrated(shared_from_this())) { 
        LOG_WING_DEBUG("Request for OPEN");  
        _currentWingStatus->updateTarget(WingTarget::Open, std::numeric_limits<int>::max());
        updateWingMovement();
    } else {     
        _wingStatusPublisher->publishNoMovementAllowed( getWingId());  
        LOG_WING_DEBUG("Request for OPEN denied, not all motors are calibrated");    
    }   
}
void Wing::close() {   
    if ( _isFullSetupCalibDone || WingCalibrationHandler::areAllWingsCalibrated(shared_from_this())) { 
        LOG_WING_DEBUG("Request for CLOSE");  
        _currentWingStatus->updateTarget(WingTarget::Close, 0);
        updateWingMovement();
    } else {
        _wingStatusPublisher->publishNoMovementAllowed( getWingId());  
        LOG_WING_DEBUG("Request for CLOSE denied, not all motors are calibrated");    
    }   
}
void Wing::stop() {    
    LOG_WING_DEBUG("Request for STOP");
    _currentWingStatus->updateTarget(WingTarget::Position, getPosition());
    _masterWindow->stop();
    _currentWingStatus->updateTarget(WingTarget::Position, getPosition());
}
void Wing::setPositionPerc(double positionPerc) {
    if ( _isFullSetupCalibDone || WingCalibrationHandler::areAllWingsCalibrated(shared_from_this())) { 
        double posMm = positionPerc * (double) _masterWindow->getMotionManager()->getStroke();
        setPositionMm((int)posMm);
    } else {
        _wingStatusPublisher->publishNoMovementAllowed( getWingId());  
        LOG_WING_DEBUG("Request for set POSITION: " + std::to_string(positionPerc) + "% denied, not all motors are calibrated");  
    } 
}
void Wing::setPositionMm(int positionMm) {
    if ( _isFullSetupCalibDone || WingCalibrationHandler::areAllWingsCalibrated(shared_from_this())) { 
       LOG_WING_DEBUG("Request for set POSITION: " + std::to_string(positionMm) + "mm");
        _currentWingStatus->updateTarget(WingTarget::Position, positionMm);
        updateWingMovement();
    } else {
        _wingStatusPublisher->publishNoMovementAllowed( getWingId());  
        LOG_WING_DEBUG("Request for set POSITION: " + std::to_string(positionMm) + "mm denied, not all motores are calibrated");  
    }     
}
int Wing::getPosition() {
    return _masterWindow->getPosition();
}
std::shared_ptr<MasterMotorizedWindow> Wing::getMasterWindow() const {
    return _masterWindow;
}

bool Wing::hasCalibratedMotors(){
    bool allMotorsAreCalibrated = true;
    for ( auto & m : getMotors())  {
        allMotorsAreCalibrated = m->getMotionManager()->isCalibrated();
        if ( !allMotorsAreCalibrated ){ break;}
    }
    return allMotorsAreCalibrated;
}
bool Wing::calibrateOpen() {
    _currentWingStatus->setCalibrationMode(true);
    bool isCalibratedOpen  =_masterWindow->calibrateOpen().get();
    _wingStatusPublisher->publishCalibrateOpenFinished(isCalibratedOpen, getWingId());     
    _currentWingStatus->setCalibrationMode(false);
    return isCalibratedOpen;
}
bool Wing::calibrateClose() {
    _currentWingStatus->setCalibrationMode(true);
    bool isCalibratedClose  =_masterWindow->calibrateClose().get();
    _wingStatusPublisher->publishCalibrateCloseFinished(isCalibratedClose, getWingId());  
    _currentWingStatus->setCalibrationMode(false);
    return isCalibratedClose;
}

// can not clear own siblings because than this function will be called in a loop 
void Wing::clearCalibration() {
    LOG_WING_DEBUG("Clear calibration");
    _masterWindow->clearCalibration();
    _isFullSetupCalibDone= false;
    if (_currentWingStatus->getTargetType() == WingTarget::Calibration ) {
        _cancelCalibrationWorkerSignal.set_value(); //set void value to flag a cancel            
    }
    _wingStatusPublisher->publishCalibrationCleared(getWingId());
}

void Wing::startCalibrate() {
    // skip wheb sibling is already busy calibrating
    for ( auto & wingInfo : *_siblings)  {
        if (std::get<0>(wingInfo)->isBusyCalibrating()) {
            LOG_WING_DEBUG("Skip start calibration because one of thw siblings is already calibrating");
            return;
        }        
    }
    _cancelCalibrationWorkerSignal = std::promise<void>();
    std::future<void> cancelObj = _cancelCalibrationWorkerSignal.get_future();
    
    WingCalibrationHandler::startCalibration(cancelObj,shared_from_this(),_wingStatusPublisher);
}
void Wing::waitOnCalibration() {
    if( _currentWingStatus->getTargetType() == WingTarget::Calibration) {
        _workOnCalibFuture.get();    
    }    
}

void Wing::checkSiblingRelations() {
    // check on inter wing collisions
    for ( auto & wingInfo : *_siblings)  {
        auto sibling = std::get<0>(wingInfo);
        auto siblingType = std::get<1>(wingInfo);
        // todo make sure deadlocks can not happen!
        validateRelation(wingInfo,getPosition());
    }
}

void Wing::logWingStatus() {
    if( _currentWingStatus->isEqual(_lastWingStatus)) {
        return;
    }
    auto curr = _currentWingStatus->toString();
    auto last = _lastWingStatus->toString();
    _lastWingStatus = std::make_shared<WingStatus>(_currentWingStatus);
    
}
// this functin gets called when the masterwindow is moving or when a new target is requested 
// this function will handle collisions
void Wing::updateWingMovement () {      
    logWingStatus();
    // skip controlling when master window's are not calibrated
    if (!_isFullSetupCalibDone) {
        return;
    }

    if ( _currentWingStatus->getTargetType() == WingTarget::Calibration) {
        return; // don't steer when in calibration mode
    }    
    int pushMargin = 10;
    if( !_siblings->empty()) {
        // only check siblings when not pushed (otherwise sibling can deadlock system)
        checkSiblingRelations();      
        
        bool targetInzone = getCornerPushZone()->isInZone(getTarget()) && getOppositePushZone()->isInZone(getTarget());
   
        if( targetInzone) {
            if ( !_currentWingStatus->getIsBlockedByCorner() && !_currentWingStatus->getIsBlockedByOpposite()) {
                // todo this gets called every time it is allowed to move => don't call when already ok               
                switch (_currentWingStatus->getTargetType())
                {   
                    case  WingTarget::Close:  { _masterWindow->close(); break;}
                    case  WingTarget::Open:  {_masterWindow->open(); break;}
                    case  WingTarget::Position:  { _masterWindow->setTarget(getTarget());  break;}
                    case  WingTarget::Idle:  {_masterWindow->stop(); break;
                    }
                    default: break;
                }
            }
        }
        
            else { 
             
            // first time pushed out of the way remember where to go back to if it is not a opposite sibling
            if (_currentWingStatus->getTargetType() == WingTarget::Idle) {                
                _currentWingStatus->updateTarget(WingTarget::Position, getPosition());
            }
            // when pushed in Opposite relation, lose the target 
            if (getOppositePushZone()->isActive()) {                
                _currentWingStatus->updateTarget(WingTarget::Idle,getPosition());                 
            }
        
            if ( !_currentWingStatus->getIsBlockedByCorner()) {
                    if (  getOppositePushZone()->shouldOpen(getTarget())) {  
                    auto minOpen = getOppositePushZone()->getMinOpen() + pushMargin;
                    if ( getCornerPushZone()->shouldClose(minOpen)) {
                        minOpen = getCornerPushZone()->getMaxOpen();
                    } 
                    getMasterWindow()->setTarget(minOpen);                      
                }  
                
                if ( getOppositePushZone()->shouldClose(getTarget())) { 
                    auto maxOpen =std::max(0,getOppositePushZone()->getMaxOpen()- pushMargin );
                    if ( getCornerPushZone()->shouldOpen(maxOpen)) {
                        maxOpen = std::max(0,getCornerPushZone()->getMinOpen());
                    }                
                    getMasterWindow()->setTarget(maxOpen);                    
                }
            }

             if ( !_currentWingStatus->getIsBlockedByOpposite()) {

                if ( getCornerPushZone()->shouldOpen(getTarget()) ) {  
                    auto minOpen =  getCornerPushZone()->getMinOpen() + pushMargin;
                    if ( getCornerPushZone()->shouldClose(minOpen)) {
                        minOpen = getCornerPushZone()->getMaxOpen();
                    } 
                
                    getMasterWindow()->setTarget(minOpen);  
                    
                }  
                
                if (getCornerPushZone()->shouldClose(getTarget()) ) { 
                    auto maxOpen =std::max(0, getCornerPushZone()->getMaxOpen()- pushMargin );
                    if ( getCornerPushZone()->shouldOpen(maxOpen)) {
                        maxOpen = std::max(0,getCornerPushZone()->getMinOpen());
                    }                
                    getMasterWindow()->setTarget(maxOpen);                    
                }
             }

            //         if ( getCornerPushZone()->shouldOpen(getTarget()) || getOppositePushZone()->shouldOpen(getTarget())) {  
            //     auto minOpen = std::max( getCornerPushZone()->getMinOpen(),getOppositePushZone()->getMinOpen()) + pushMargin;
            //     if ( getCornerPushZone()->shouldClose(minOpen)) {
            //         minOpen = getCornerPushZone()->getMaxOpen();
            //     } 
             
            //     getMasterWindow()->setTarget(minOpen);  
                
            // }  
            
            // if (getCornerPushZone()->shouldClose(getTarget()) || getOppositePushZone()->shouldClose(getTarget())) { 
            //     auto maxOpen =std::max(0,std::min( getCornerPushZone()->getMaxOpen(),getOppositePushZone()->getMaxOpen())- pushMargin );
            //      if ( getCornerPushZone()->shouldOpen(maxOpen)) {
            //         maxOpen = std::max(0,getCornerPushZone()->getMinOpen());
            //     }                
            //     getMasterWindow()->setTarget(maxOpen);                    
            // }
        } 
    }   
    else {
        switch (_currentWingStatus->getTargetType())
            {   
                case  WingTarget::Close:  _masterWindow->close(); break;
                case  WingTarget::Open:  _masterWindow->open(); break;
                case  WingTarget::Position:  _masterWindow->setTarget(getTarget());  break;
                case  WingTarget::Idle:   _masterWindow->stop(); break;
                default: break;
            }
    }
    // when on target set new Target Idle
    if ( _masterWindow->isOntarget() && !getCornerPushZone()->isInZone(getTarget())) {
        _currentWingStatus->updateTarget(WingTarget::Idle, getPosition());
    }
}
// this function validates the relation between two wings that interfere in a corner
void Wing::validateRelation(std::tuple<std::shared_ptr<IWing>,WingSiblingType> & wingInfo, int position) {

    std::shared_ptr<IWing> & sibling = std::get<0>(wingInfo);
    WingSiblingType & siblingType = std::get<1>(wingInfo);

    int currentDerrivedTarget = 0;
  

    // THIS WING IS FEMALE REGARDING TO THE OTHER WING        
    if( siblingType == WingSiblingType::CornerMale || siblingType == WingSiblingType::MiddleMale ) {
        _wingRelationManager->ManageMaleCorner(
            [&](){return blockedByCornerRelation(); },
            [&](){return unBlockedByCornerRelation(); },                
            sibling->getCornerPushZone(),
            std::dynamic_pointer_cast<IPositionTrack>(sibling),      // male
            std::dynamic_pointer_cast<IPositionTrack>(shared_from_this())); // female
    } 
    // THIS WING IS MALE REGARDING TO OTHER WING        
    else if (siblingType == WingSiblingType::CornerFemale || siblingType == WingSiblingType::MiddleFemale) {  
        _wingRelationManager->ManageFemaleCorner(
            [&](){return blockedByCornerRelation(); },
            [&](){return unBlockedByCornerRelation(); },          
            sibling->getCornerPushZone(),
            std::dynamic_pointer_cast<IPositionTrack>(shared_from_this()), // male
            std::dynamic_pointer_cast<IPositionTrack>(sibling));     // female 
    }
    else if (siblingType == WingSiblingType::Opposite) {  
        if ( !getOppositePushZone()->isActive()) {
        _wingRelationManager->ManageOpposite(
            [&](){return blockedByOppositeRelation(); },
            [&](){return unBlockedByOppositeRelation(); },
            sibling->getOppositePushZone(),
            std::dynamic_pointer_cast<IPositionTrack>(shared_from_this()), 
            sibling->getMasterWindow()->getTailDistanceToFullyClose(),
            sibling->getMasterWindow()->getMotionManager()->getMotorStatusData().isClosed,            
            getMasterWindow()->isOntarget() && !(_currentWingStatus->getIsBlockedByCorner() || _currentWingStatus->getIsBlockedByOpposite()));  //when blocked the desired target is not yet given to the masterwindow and therefore can not be on target
        } else {
            LOG_TRACE("Currently pushed away");
        }
    }
}



void Wing::blockedByOppositeRelation(){
  
    _currentWingStatus->updateBlockedByOpposite (true);
    _masterWindow->stop();
    
    switch ( _currentWingStatus->getTargetType())
    {
        case  WingTarget::Close:    LOG_WING_DEBUG("Blocked CLOSE by opposite sibling "); break;
        case  WingTarget::Open:     LOG_WING_DEBUG("Blocked OPEN by opposite sibling " ); break;
        case  WingTarget::Position: LOG_WING_DEBUG("Blocked POSITION [" + std::to_string(getTarget()) + "] by opposite sibling " ); break;
        case  WingTarget::Idle:     LOG_WING_DEBUG("Blocked Idle by opposite sibling " ); break;
        default: break;
    }
}
void Wing::blockedByCornerRelation(){
    _currentWingStatus->updateBlockedByCorner(true);
    _masterWindow->stop();
    
    switch ( _currentWingStatus->getTargetType())
    {
        case  WingTarget::Close:    LOG_WING_DEBUG("Blocked CLOSE by corner sibling "); break;
        case  WingTarget::Open:     LOG_WING_DEBUG("Blocked OPEN by corner sibling " ); break;
        case  WingTarget::Position: LOG_WING_DEBUG("Blocked POSITION [" + std::to_string(getTarget()) + "] by corner sibling " ); break;
        case  WingTarget::Idle:     LOG_WING_DEBUG("Blocked Idle by corner sibling " ); break;
        default: break;
    }
}
void Wing::unBlockedByOppositeRelation(){
    _currentWingStatus->updateBlockedByOpposite(false);
     
     std::string msgPrefix = "";     
     if(!_currentWingStatus->getIsBlockedByCorner()) { 
        msgPrefix = "Fully";
     }
    switch (_currentWingStatus->getTargetType())
    {   
        case  WingTarget::Close:     LOG_WING_DEBUG(msgPrefix + "UnBlocked CLOSE by opposite sibling " ); break;
        case  WingTarget::Open:      LOG_WING_DEBUG(msgPrefix + "UnBlocked OPEN by opposite sibling "); break;
        case  WingTarget::Position:  LOG_WING_DEBUG(msgPrefix + "UnBlocked POSITION [" + std::to_string(getTarget()) + "] by opposite sibling");  break;
        case  WingTarget::Idle:      LOG_WING_DEBUG(msgPrefix + "UnBlocked Idle by opposite sibling "); break;
        default: break;
    }
}
void Wing::unBlockedByCornerRelation(){
     _currentWingStatus->updateBlockedByCorner(false);
     
     std::string msgPrefix = "";     
    if(!_currentWingStatus->getIsBlockedByOpposite()) { 
        msgPrefix = "Fully";
    }
    switch (_currentWingStatus->getTargetType())
    {   
        case  WingTarget::Close:     LOG_WING_DEBUG(msgPrefix + "UnBlocked CLOSE by corner sibling " ); break;
        case  WingTarget::Open:      LOG_WING_DEBUG(msgPrefix + "UnBlocked OPEN by corner sibling "); break;
        case  WingTarget::Position:  LOG_WING_DEBUG(msgPrefix + "UnBlocked POSITION [" + std::to_string(getTarget()) + "] by corner sibling");  break;
        case  WingTarget::Idle:      LOG_WING_DEBUG(msgPrefix + "UnBlocked Idle by corner sibling "); break;
        default: break;
    }
    
}

    

// The motors are gathered on request! Don't call this to freqently! This is done like this because the wing has no idea when some 
// slaves are added or deleted -> improve by notify 
const std::vector<std::shared_ptr<MotorizedWindow>> Wing::getMotors() const {
    auto motors = _masterWindow->getSlaveMotors();
    motors.push_back(_masterWindow);
    return motors;
}



