#ifndef H_WINGCALIBRATIONHANDLER
#define H_WINGCALIBRATIONHANDLER

#include "wing.h"   
#include "pch.h"

//upfront declaration
class IWing;

class WingCalibrationHandler {
    public:        
        static void startCalibration(std::future<void> & cancelObj,std::shared_ptr<IWing> entryWing, std::shared_ptr<IWingStatusPublisher> statusPublisher);
        static void clearCalibration(std::shared_ptr<IWing> entryWing, std::shared_ptr<IWingStatusPublisher> statusPublisher);
        static std::vector<std::shared_ptr<IWing>> getAllWings(std::shared_ptr<IWing> entryWing);
        static std::vector<std::shared_ptr<IWing>>::iterator getWingWithSpecificTypeAsSibling ( std::vector<std::shared_ptr<IWing>> & allWings, std::function<bool(WingSiblingType)> isSiblingTypeValid); 
        static bool areAllWingsCalibrated(std::shared_ptr<IWing> entryWing);
        static void setAllWingsCalibratedFlag(std::shared_ptr<IWing> entryWing);
        
    private:
        static bool runCorrectSequence(std::future<void> & cancelObj,std::vector<std::shared_ptr<IWing>> allWings, std::function<bool(std::shared_ptr<IWing>)> openAction, std::function <bool(std::shared_ptr<IWing>)> closeAction);
        static void recursiveListOtherWings(std::vector<std::shared_ptr<IWing>> & list, std::reference_wrapper<IWing> wing  );
        static void setAllWingsCalibratedFlag(std::vector<std::shared_ptr<IWing>> allWings);
        
};
  
#endif