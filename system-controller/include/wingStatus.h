#ifndef WINGSTATUS_H
#define WINGSTATUS_H

#include "pch.h"
#include "windowPushZone.h"

enum class WingTarget {
    Idle,
    Open,
    Close,
    Position,
    Calibration
};



class IWingStatus {
    public:
        virtual ~IWingStatus(){}
        virtual int getPosition() const =0;
        virtual int getTarget() const=0;
        virtual bool waslastMovementOpening() const =0;
        virtual bool getIsBlockedByOpposite()const =0;
        virtual bool getIsBlockedByCorner()const =0;
        virtual std::shared_ptr<IWindowPushZone> getOppositePushZone() const=0;
        virtual std::shared_ptr<IWindowPushZone> getCornerPushZone() const=0;
        virtual WingTarget getTargetType() const=0;
        virtual void setCalibrationMode(bool isOn) =0;
        virtual bool isEqual(const std::shared_ptr<IWingStatus> otherWingStatus) const =0;
        virtual void updatePosition(int position)=0;
        virtual void updateTarget(WingTarget wingTargetType,int target) =0;
        virtual void updateBlockedByOpposite(bool isBlocked) =0;
        virtual void updateBlockedByCorner(bool isBlocked) =0;
        virtual std::string toString() const =0;
};

class WingStatus : public IWingStatus {
    public:
        WingStatus(int position, int target, WingTarget targetType,bool isBlockedByOpposite,bool isBlockedByCorner, std::shared_ptr<IWindowPushZone> oppositePushZone, std::shared_ptr<IWindowPushZone> cornerPushZone) {
            _oppositePushZone = std::make_shared<WindowPushZone> (oppositePushZone);
            _cornerPushZone = std::make_shared<WindowPushZone> (cornerPushZone);
            _pos= position;
            _target=target;
            _isBlockedCorner = isBlockedByCorner;
            _isBlockedOpposite = isBlockedByOpposite;
            _targetType=targetType;
        }
        WingStatus ( const std::shared_ptr<IWingStatus> otherWingStatus) {
            _oppositePushZone = std::make_shared<WindowPushZone>(otherWingStatus->getOppositePushZone());
            _cornerPushZone = std::make_shared<WindowPushZone>(otherWingStatus->getCornerPushZone());
            _pos=otherWingStatus->getPosition();
            _target=otherWingStatus->getTarget();
            _isBlockedCorner = otherWingStatus->getIsBlockedByCorner();
            _isBlockedOpposite = otherWingStatus->getIsBlockedByOpposite();
            _targetType = otherWingStatus->getTargetType();
        }
        bool isEqual(const std::shared_ptr<IWingStatus> otherWingStatus) const override{
            return _oppositePushZone->isEqual(otherWingStatus->getOppositePushZone()) && 
                _cornerPushZone->isEqual(otherWingStatus->getCornerPushZone()) && 
                _pos==otherWingStatus->getPosition()   &&                
                _target == otherWingStatus->getTarget() &&
                _isBlockedCorner == otherWingStatus->getIsBlockedByCorner() &&
                _isBlockedOpposite == otherWingStatus->getIsBlockedByOpposite() &&
                _targetType == otherWingStatus->getTargetType();
        }
        std::string toString() const override {                             
                std::ostringstream oss;
                oss << std::setw(7) << ((getIsBlockedByOpposite() || getIsBlockedByCorner()) ? "BLOCKED" :" FREE"); 
                oss <<" - Pos: ";
                oss << std::setw(5) << getPosition();
                oss <<" - Target: ";
                oss << std::setw(5) << std::min(99999, getTarget()); // set print maximum
                switch (getTargetType())
                {   
                    case  WingTarget::Close:  oss << std::setw(7) << "<CLOSE>"; break;
                    case  WingTarget::Open:   oss << std::setw(7) << "<OPEN>"; break;
                    case  WingTarget::Position:   oss << std::setw(7) << "<POSI>"; break;
                    case  WingTarget::Idle:   oss << std::setw(7) << "<IDLE>"; break;
                    case  WingTarget::Calibration:   oss << std::setw(7) << "<CALIB>"; break;
                    default: break;
                };
                oss <<" C:"<< getCornerPushZone()->toString();
                oss <<" O:"<< getOppositePushZone()->toString();
                return oss.str();
        }

        int getPosition()const {return _pos;}
        int getTarget() const{return _target;}
        
        void updatePosition(int position) override{_pos = position;}
        void updateTarget(WingTarget wingTargetType,int target) override  {
            if ( _targetType == WingTarget::Calibration)  {// ignore if current type is on calib
                return;
            }
            _targetType = wingTargetType;
            _target = target;
            if ( _targetType == WingTarget::Close || _targetType == WingTarget::Open ) {
                _lastOpenCloseTarget = _targetType;
            }
        }
        void setCalibrationMode(bool isOn) {
            if ( isOn) {
                _targetType= WingTarget::Calibration;                
            } else {
                _targetType= WingTarget::Idle;                
            }
        }
        void updateBlockedByOpposite(bool isBlocked) override{_isBlockedOpposite = isBlocked;}
        void updateBlockedByCorner(bool isBlocked) override{_isBlockedCorner = isBlocked;}
        WingTarget getTargetType() const override {return _targetType;}
        bool waslastMovementOpening() const override {return _lastOpenCloseTarget == WingTarget::Open;}
        bool getIsBlockedByOpposite()const {return _isBlockedOpposite;}
        bool getIsBlockedByCorner()const {return _isBlockedCorner;}
        std::shared_ptr<IWindowPushZone> getOppositePushZone() const { return _oppositePushZone;}
        std::shared_ptr<IWindowPushZone> getCornerPushZone() const { return _cornerPushZone;}

    private:
        int _pos;
        int _target;        
        bool _isBlockedCorner;
        bool _isBlockedOpposite;
        std::shared_ptr<IWindowPushZone> _oppositePushZone;
        std::shared_ptr<IWindowPushZone> _cornerPushZone;
        WingTarget _targetType = WingTarget::Position;
        WingTarget _lastOpenCloseTarget =  WingTarget::Close;
};

#endif //WINGSTATUS_H