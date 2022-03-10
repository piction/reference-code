#ifndef POSITIONTRACK_H
#define POSITIONTRACK_H

class IPositionTrack {
    public:
        virtual ~IPositionTrack(){}
        virtual int getPosition()=0; 
        virtual int getTarget()=0;
        virtual bool waslastMovementOpening() const = 0;
};

#endif //POSITIONTRACK_H