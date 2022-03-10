#ifndef ICANCALIBRATE_H
#define ICANCALIBRATE_H


class ICanCalibrate {
    public:
       virtual ~ICanCalibrate(){}
        virtual void waitOnCalibration()=0;
        virtual void startCalibrate()=0;
        virtual bool calibrateOpen()=0;
        virtual bool calibrateClose()=0;
        virtual bool isCalibrated()=0;
        virtual void clearCalibration()=0;
        virtual bool isBusyCalibrating() const = 0;
};

#endif //ICANCALIBRATE  {
