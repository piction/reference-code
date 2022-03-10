#include "movingWindow.h"
#include "motorizedWindow.h"
#include <exception>
#include <utility>



MovingWindow::MovingWindow(int length, std::shared_ptr<IMotorMotionManager> motorMotionManager) 
    : _motionManager(std::move(motorMotionManager))
    , _length(length)
    , _position(0)
    , _pushType(PushType::Stop) {
      
    }

void MovingWindow::addSlave(std::shared_ptr<IMovingWindow> slave, SlaveType slaveType) {
    // todo add some checking on double insertion or other mistakes
    _slaves.push_back(std::make_tuple(slave,slaveType));
}
std::vector<std::shared_ptr<IMovingWindow>> MovingWindow::getSlaves() const {
    std::vector<std::shared_ptr<IMovingWindow>> slaveWindows;
    for ( auto  & slaveInfo : _slaves ) {
        slaveWindows.push_back(std::get<0>(slaveInfo));
    }
    return slaveWindows;
}
void MovingWindow::getSlaveTypesTree(std::vector<SlaveType> & slaveTypesTree) {
    for ( auto  & slaveInfo : _slaves ) {
        slaveTypesTree.push_back(std::get<1>(slaveInfo));
        std::get<0>(slaveInfo)->getSlaveTypesTree(slaveTypesTree);
    }
}
void MovingWindow::onPositionUpdate(int newPosition) {
    _position = newPosition;
    pushSlavesToAllowMovement();
    if (_pushType ==  PushType::ForceOpenForCalibration || _pushType == PushType::ForceCloseForCalibration ) {
        return;
    }
    switch( _freeToMove){ 
        case MovementFreedom::None: _motionManager->stop(); break;
        case MovementFreedom::Slow: _motionManager->setLowSpeed(); break;
        case MovementFreedom::Fast: _motionManager->setHighSpeed(); break;
    }
}
void MovingWindow::stopWindow() {
     for ( auto  & slaveInfo : _slaves ) {
        auto slave = std::get<0>(slaveInfo);
        slave->stopWindow();
    }    
    _pushType = PushType::Stop;
}

void MovingWindow::push(PushType push){
    _pushType = push;
}
MovementFreedom MovingWindow::GetLeastAllowedMovement (MovementFreedom  m1 , MovementFreedom m2){
    if ( m1 == MovementFreedom::None || m2 == MovementFreedom::None) return MovementFreedom::None;
    if ( m1 == m2) return m1;
    return MovementFreedom::Slow;
}

void MovingWindow::handleSlaveOnPushOpen(MovementFreedom & allowedMovement, const std::shared_ptr<IMovingWindow>& slave, SlaveType slaveType) {
    allowedMovement = GetLeastAllowedMovement(allowedMovement ,slave->getMovementFreedom());

    if (slaveType == SlaveType::Passive || slaveType == SlaveType::Motor )  {
        int passedOwnLength = _position - _length;
        if ( slaveType == SlaveType::Motor) { 
           //LOG_DEBUG(std::to_string(passedOwnLength) + " " + std::to_string(slave->getPosition()) + "  " + std::to_string(_position));
            if ( (passedOwnLength  + SystemSettings::getInstance().getChicanZone()) > slave->getPosition() ) {
                std::static_pointer_cast<MotorizedWindow>(slave)->push(PushType::PushToOpen);
                
                if ( (passedOwnLength  + SystemSettings::getInstance().getChicanOverlap()) > slave->getPosition() ) {                   
                    
                    // when on the end of the stroke the master slave can fully open 
                    if ( slave->getMotionManager()->getMotorStatusData().getStatus() != MotorStatus::Open) {
                        allowedMovement = MovementFreedom::None;
                    }
                }
            }   
            // check that slave is not pushed to far (if so hold the slave motor!)
            if ( _position - slave->getPosition() < SystemSettings::getInstance().getChicanZone() ) {                
                std::static_pointer_cast<MotorizedWindow>(slave)->push(PushType::Stop);
            }    
        } else if ( slaveType == SlaveType::Passive) {
            std::static_pointer_cast<MotorizedWindow>(slave)->push(PushType::PushToOpen);
            auto lastStandStillPosition = std::static_pointer_cast<PassiveWindow>(slave)->getLastStandStillPosition();
            if (std::abs (passedOwnLength - lastStandStillPosition) < SystemSettings::getInstance().getSlowdownDist()) {
                LOG_TRACE("Slow opening due passed own Length " + std::to_string(passedOwnLength) + " At last standstill "  +  std::to_string(lastStandStillPosition));
                allowedMovement = GetLeastAllowedMovement(allowedMovement ,MovementFreedom::Slow);
            } else {
                allowedMovement = GetLeastAllowedMovement(allowedMovement ,MovementFreedom::Fast);
            }
            std::static_pointer_cast<PassiveWindow>(slave)->setMovementFreedomDuePossibleContact(allowedMovement);
            // update position of passive window when pushing
            if ( passedOwnLength >= slave->getPosition()) {

                std::static_pointer_cast<PassiveWindow>(slave)->setSpeedDuePushingOrPulling(_speed);
                std::static_pointer_cast<PassiveWindow>(slave)->setPositionDuePushingOrPulling(passedOwnLength);
            } else {
                std::static_pointer_cast<PassiveWindow>(slave)->setSpeedDuePushingOrPulling(0);
            }
        } 
    }
    else {
        throw std::logic_error("Unhandled slavetype handleSlaveOnPushOpen");
    }
}

void MovingWindow::handleSlaveOnPullClose(MovementFreedom& allowedMovement, std::shared_ptr<IMovingWindow> slave, SlaveType slaveType)
{
    allowedMovement = GetLeastAllowedMovement(allowedMovement, slave->getMovementFreedom());
    if (slaveType == SlaveType::Motor) {
        if ((_position - SystemSettings::getInstance().getChicanZone()) < slave->getPosition()) {
            std::static_pointer_cast<MotorizedWindow>(slave)->push(PushType::PushToClose);
            if ((_position - SystemSettings::getInstance().getChicanOverlap()) < slave->getPosition()) {
                if (slave->getMotionManager()->getMotorStatusData().getStatus() != MotorStatus::Closed) { // when on the end of the stroke the master slave can close fully
                    if (slave->getPosition() > 10) {
                        LOG_DEBUG(_motionManager->getId() + " failed complete close @ " + std::to_string((int)slave->getMotionManager()->getMotorStatusData().getStatus()) + std::to_string(getPosition()))
                        allowedMovement = MovementFreedom::None;
                    }
                }
            }
        }
        //check that slave is not closed to far (if so hold the slave motor!)
        if ((slave->getPosition() + slave->getLength() - _position) < SystemSettings::getInstance().getChicanZone()) {
            std::static_pointer_cast<MotorizedWindow>(slave)->push(PushType::Stop);
        }
    } else if (slaveType == SlaveType::Passive) {
        std::static_pointer_cast<MotorizedWindow>(slave)->push(PushType::PushToClose);
        auto lastStandStillPosition = std::static_pointer_cast<PassiveWindow>(slave)->getLastStandStillPosition();
        if (std::abs(_position - lastStandStillPosition) < SystemSettings::getInstance().getSlowdownDist()) {
            allowedMovement = GetLeastAllowedMovement(allowedMovement, MovementFreedom::Slow);
            LOG_TRACE("Slow closing due position " + std::to_string(_position) + " At last standstill " + std::to_string(lastStandStillPosition));
        } else {
            allowedMovement = GetLeastAllowedMovement(allowedMovement, MovementFreedom::Fast);
        }
        allowedMovement = GetLeastAllowedMovement(allowedMovement, slave->getMovementFreedom());
        std::static_pointer_cast<PassiveWindow>(slave)->setMovementFreedomDuePossibleContact(allowedMovement);
        // update position of passive window when pulling
        if (_position <= slave->getPosition()) {
            std::static_pointer_cast<PassiveWindow>(slave)->setSpeedDuePushingOrPulling(_speed);
            std::static_pointer_cast<PassiveWindow>(slave)->setPositionDuePushingOrPulling(_position);
        }
    } else {
        LOG_CRITICAL_THROW("Unhandled slavetype handleSlaveOnPullClose");
    }
}

// check all slaves of the master window (can be passive/motorized/mastermotorized) can move
// if a slave needs to pused away, push the slave 
// if the slave can not be moved the slave will push his slave away first
void MovingWindow::pushSlavesToAllowMovement() {
   
    MovementFreedom allowedMovement=MovementFreedom::Fast;
       
    for ( auto  & slaveInfo : _slaves ) {
        auto slave = std::get<0>(slaveInfo);
        // always update the slaves with there current position
        if (  std::get<1>(slaveInfo) == SlaveType::Passive ) {
            std::static_pointer_cast<PassiveWindow>(slave)->updateWithCurrentPosition();
        }

        switch (_pushType) {
            case PushType::PushToOpen:                  
                handleSlaveOnPushOpen(allowedMovement,slave, std::get<1>(slaveInfo));
                break;
            case PushType::PushToClose:
                handleSlaveOnPullClose(allowedMovement, slave, std::get<1>(slaveInfo));
                break;
            case PushType::ForceOpenForCalibration:
                std::static_pointer_cast<MotorizedWindow>(slave)->push(PushType::ForceOpenForCalibration);
                break;
            case PushType::ForceCloseForCalibration:
                std::static_pointer_cast<MotorizedWindow>(slave)->push(PushType::ForceCloseForCalibration);
                break;
            default:
                std::static_pointer_cast<MovingWindow>(slave)->push(PushType::Stop);
                break;
        }     
           
    }
    _freeToMove= allowedMovement;

    //     // logging !
    // static MovementFreedom prev = MovementFreedom::Fast ;
    // if ( prev != _freeToMove) 
    // {
    //     switch ( _freeToMove) {
    //         case MovementFreedom::Fast : LOG_DEBUG(_motionManager->getId() +  " Allowed masterMovement due slaves: FAST " + std::to_string(getPosition()) ) break;
    //         case MovementFreedom::Slow : LOG_DEBUG(_motionManager->getId() +  " Allowed masterMovement due slaves: SLOW "+ std::to_string(getPosition()) ) break;
    //         default : LOG_DEBUG(_motionManager->getId() +  " Allowed masterMovement due slaves: STOP " + std::to_string(getPosition()) ) break;
    //     };        
    // }
    // prev = _freeToMove;
}

void MovingWindow::updateWithCurrentPosition()  {
    onPositionUpdate(_position);
}


// only overwrite panel lengths with new one if no panel length was filled in
void MovingWindow::updateAllPanelLenghts(int newPanelLength) {
    if ( _length <= 0 ) {
        LOG_DEBUG("Update panelLength -> " +  std::to_string(newPanelLength));
        _length = newPanelLength;
    }
    for ( auto & s : getSlaves()) {
        s->updateAllPanelLenghts(newPanelLength);
    }
}