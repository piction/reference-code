
#ifndef WINDOWPUSHZONE_H
#define WINDOWPUSHZONE_H

#include "pch.h"

class IWindowPushZone {
    public:
        virtual ~IWindowPushZone(){};
        virtual void setZone(int minOpen, int maxOpen)=0;
        virtual void setMinOpening(int minOpen)=0;
        virtual void setMaxOpening(int maxOpen)=0;
        virtual void inActivate()=0;
        virtual int getMaxOpen() const =0;
        virtual int getMinOpen() const =0;
        virtual bool isActive() const =0;
        virtual bool isMinLimActive() const =0;
        virtual bool isMaxLimActive() const =0;
        virtual bool isInZone(int position) const =0;
        virtual bool shouldOpen(int position) const=0;
        virtual bool shouldClose(int position) const =0;
        virtual bool isEqual (const std::shared_ptr<IWindowPushZone> otherZone) const=0;
        virtual std::string toString() = 0;
};

// this class is used to 'push' a window in a specific zone of the opening.
// the zone is defined with a min and max opening position. 
// This class manages this zone and can 'calculate' if a window position is in this zone or not
class WindowPushZone : public IWindowPushZone{
    public:
        WindowPushZone() {
            _hasActiveMinOpening = false;
            _hasActiveMaxOpening = false;
            _minOpen = -1;
            _maxOpen = std::numeric_limits<int>::max();
        }
        WindowPushZone(const std::shared_ptr<IWindowPushZone> otherPushZone) { // copy constructor
            _hasActiveMinOpening = false;
            _hasActiveMaxOpening = false;
            _minOpen = -1;
            _maxOpen = std::numeric_limits<int>::max();

            if(otherPushZone->isMaxLimActive() ) {
                setMaxOpening(otherPushZone->getMaxOpen());
            }
            if(otherPushZone->isMinLimActive() ) {
                setMinOpening(otherPushZone->getMinOpen());
            }
        }   
        bool isEqual (const std::shared_ptr<IWindowPushZone> otherZone)  const override{
            return (otherZone->getMaxOpen() == getMaxOpen()) && (otherZone->getMinOpen() == getMinOpen());
        }   
  
        void setZone(int minOpen, int maxOpen) override{          
            setMinOpening(minOpen); setMaxOpening(maxOpen);
        }
        // only sets the minzone to a new value, does not check with previous version
        void setMinOpening(int minOpen)override {       
            if(_hasActiveMaxOpening && minOpen > _maxOpen) {
                LOG_CRITICAL_THROW("Impossible window push zone Min:" + std::to_string(minOpen) + ", Max:" + std::to_string(_maxOpen) );
            }     
            _minOpen =  minOpen;
            _hasActiveMinOpening = true; 
        }
        // only sets the maxzone to a new value, does not check with previous version
        void setMaxOpening(int maxOpen) override{   
              if(_hasActiveMinOpening && maxOpen < _minOpen) {
                LOG_CRITICAL_THROW("Impossible window push zone Min:" + std::to_string(_minOpen) + ", Max:" + std::to_string(maxOpen) );
            }              
            _maxOpen =  std::max(0,maxOpen);            
            _hasActiveMaxOpening = true;
        }
        void inActivate() override{
            _hasActiveMinOpening = false;
            _hasActiveMaxOpening = false;
        }
        bool isMinLimActive() const override {return _hasActiveMinOpening;}
        bool isMaxLimActive() const override {return _hasActiveMaxOpening;}
        bool isActive() const override {return _hasActiveMinOpening || _hasActiveMaxOpening;}
        int getMaxOpen () const override {return _hasActiveMaxOpening ? _maxOpen : std::numeric_limits<int>::max();}
        int getMinOpen () const override{return _hasActiveMinOpening ? _minOpen : -1 ;}
        bool isInZone(int position) const override { return !(shouldOpen(position) || shouldClose(position));}
        bool shouldOpen(int position) const override { return (_hasActiveMinOpening && (position < _minOpen)) ;}
        bool shouldClose(int position) const override { return (_hasActiveMaxOpening && (position > _maxOpen)) ;}
        std::string toString() override {
            if (!isActive()) {
                std::ostringstream oss;
                oss << std::setw(22) <<"--- not active --"; // same length as when range is active
               return  oss.str();
            }

            std::ostringstream oss;
            oss << std::setw(8) << "range:[ ";      // total length of current stream: 8
            if (_hasActiveMinOpening) {
                oss << std::setw(6) << _minOpen;    // total length of current stream: 14
            } else {
                oss << std::setw(6) << "-inf";       // total length of current stream: 14
            }
            oss << ",";                             // total length of current stream: 15
            if (_hasActiveMaxOpening) {
                oss << std::setw(6) << _maxOpen;    // total length of current stream: 21
            } else {
                oss << std::setw(6) << "inf";       // total length of current stream: 21
            }
            oss <<"]";                              // total length of current stream: 22
            return  oss.str();
            
        };
        
    private:
        bool _hasActiveMinOpening;
        bool _hasActiveMaxOpening;
        int _minOpen;
        int _maxOpen;
};

#endif //WINDOWPUSHZONE_H