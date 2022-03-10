#include "passiveWindow.h"


PassiveWindow::PassiveWindow(int length) 
    : _lastStandstillPosition(0)
    , MovingWindow(length,std::make_shared<EmptyMotionManager>()) {
        this->_freeToMove = MovementFreedom::Fast;        
};

void PassiveWindow::setSpeedDuePushingOrPulling(int speed) {
    _speed = speed;
}
void PassiveWindow::setMovementFreedomDuePossibleContact(MovementFreedom m) {
    _freeToMove = m;
}
void PassiveWindow::push(PushType push){
    if ( _pushType != push) {
        _lastStandstillPosition = _position;
    }
     MovingWindow::push( push);
}
void PassiveWindow::setPositionDuePushingOrPulling(int position) {
    // When we limit the update for position diffs bigger than 0.1mm
    // to prevent wrong openingdirection due zero diff  
    if ( std::abs(position - _position) > 0.1) {
        bool isPrevDirectionOpening = _isCurrentDirectionOpening;
        _isCurrentDirectionOpening = (_position < position);
        if (_isCurrentDirectionOpening) {
            push(PushType::PushToOpen);
        }else {
            push(PushType::PushToClose);
        }   
    }
    _position = position;
    onPositionUpdate(position);
}



