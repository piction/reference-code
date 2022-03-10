#include "wingCalibrationHandler.h"
#include "log.h"


// IMPORTANT THIS RUN ASSUMES A CLOSED POSITION TO START WITH AND WILL END IN A CLOSED POSITION
// First all the corners will be calibrated 
// Second all opposites that are not yet calibrated are calibrated
// Calibrated until all wings are erased from the allwings list
bool WingCalibrationHandler::runCorrectSequence(std::future<void> & cancelObj,std::vector<std::shared_ptr<IWing>> allWings, std::function<bool(std::shared_ptr<IWing>)> openAction, std::function <bool(std::shared_ptr<IWing>)> closeAction) {

    auto isCancelled = [&]() {
        if ((cancelObj.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) ) {
            //_wingStatusPublisher->publishCalibratedCanceled(getWingId());
            return true;
        }
        return false;
    };

    if ( allWings.empty()) {
        LOG_ERROR("No wings found to calibrate");
        return false;
    }
   
    // if Only one wing to calibration here 
    if ( allWings.size() == 1) {      
        bool calibOpenSuccess = openAction(allWings[0]);
        if (isCancelled() || !calibOpenSuccess) return false;

        bool calibCloseSuccess = closeAction(allWings[0]);
        if (isCancelled() || !calibCloseSuccess) return false;

        // no other wings to calibrate
        return true;
    }
   
    // IMPORTANT THIS CALIBRATION SHOULD END WITH A CLOSED POSITION    
     // find a wing with a sibling of type Female
    auto getWingWithFemaleTypeAsSibling = [& allWings]() {
        return getWingWithSpecificTypeAsSibling(allWings, [](WingSiblingType t){ return t == WingSiblingType::CornerFemale || t == WingSiblingType::MiddleFemale;});
    };

    auto nextWingItt = getWingWithFemaleTypeAsSibling();
    while (nextWingItt != allWings.end())
    {
        for ( auto & bb: allWings) {
            LOG_DEBUG("still on the list at corner loop: " + bb->getWingId());
        }
        // Wing with sibling type Female        
        auto &wing = *nextWingItt;
        
        // search for female sibling
        bool handledFemaleRelation = false;
        for (auto &s : *wing->getSiblings())
        {
            std::shared_ptr<IWing> &sibling = std::get<0>(s);
            WingSiblingType &siblingType = std::get<1>(s);

            // ignore none corner relations
            if (siblingType == WingSiblingType::CornerFemale || siblingType == WingSiblingType::MiddleFemale)
            {
                // when arrived here the sibling type is Female
                auto male = std::reference_wrapper<std::shared_ptr<IWing>>(wing);
                auto female = std::reference_wrapper<std::shared_ptr<IWing>>(sibling);
                auto maleId = male.get()->getWingId();
                auto femalId = female.get()->getWingId();

                // start the calibration
                bool calibOpenSuccess = openAction(male.get());
                if (isCancelled() || !calibOpenSuccess) return false;

                bool femaleCalibOpenSuccess = openAction(female.get());
                if (isCancelled() || !femaleCalibOpenSuccess) return false;

                bool calibFemaleCloseSuccess = closeAction(female.get());
                if (isCancelled() || !calibFemaleCloseSuccess) return false;

                bool calibCloseSuccess = closeAction(male.get());
                if (isCancelled() || !calibCloseSuccess) return false;

                // no need to search further found female relation
                allWings.erase(nextWingItt); // clear from list to not be found again

                // remove other female from the allWingsList if still present  (both are calibrated, so also this can be erased)      
                auto it = std::find(allWings.begin(), allWings.end(), sibling);
                if (it != allWings.end()) {
                    allWings.erase(it);
                }

                handledFemaleRelation = true;
                break;
            }
        }
        if ( !handledFemaleRelation) {
            LOG_ERROR("Failed to handle female relation !! -> programming error");
            return false;
        }

        nextWingItt = getWingWithFemaleTypeAsSibling();
    };

   
    // find a wing with a sibling of type Opposite
    auto getWingWithOppositeTypeAsSibling = [& allWings]() {
        return getWingWithSpecificTypeAsSibling(allWings, [](WingSiblingType t){ return t == WingSiblingType::Opposite ;});
    };

    nextWingItt = getWingWithOppositeTypeAsSibling();
    while (nextWingItt != allWings.end())
    {
        for ( auto & bb: allWings) {
            LOG_DEBUG("still on the list at opposite loop: " + bb->getWingId());
        }
        // new wing with sibling type Opposite
        auto &wing = *nextWingItt;
        
        bool handledOppositeRelation = false;
        // search for opposite sibling
        for (auto &s : *(wing->getSiblings()))
        {
            std::shared_ptr<IWing> &sibling = std::get<0>(s);
            WingSiblingType &siblingType = std::get<1>(s);

            // ignore none female relations
            if (siblingType == WingSiblingType::Opposite)
            {    
                // check if wing is part of corner
                // if not than calibrate    
                auto calibrateOppositeIfNoCornerRelation = [&isCancelled, &openAction, &closeAction](std::shared_ptr<IWing> oppWing) {
                    auto wingOpp_siblings = oppWing->getSiblings();
                    if (std::find_if(std::begin(*wingOpp_siblings), std::end(*wingOpp_siblings), [](const std::tuple<std::shared_ptr<IWing>, WingSiblingType> &y) {
                            return (std::get<1>(y) ==  WingSiblingType::MiddleMale || std::get<1>(y) ==  WingSiblingType::CornerMale 
                                    || std::get<1>(y) ==  WingSiblingType::MiddleFemale || std::get<1>(y) ==  WingSiblingType::CornerFemale );
                        }) == std::end(*wingOpp_siblings))
                    {
                        // if no CORNER relation is found this is not a female wing a can be calibrated
                        bool OpenSuccess =  openAction(oppWing);            
                        if (isCancelled() || !OpenSuccess) return;

                        bool CloseSuccess =  closeAction(oppWing); 
                        if (isCancelled() || !CloseSuccess) return;
                    }              
                };
                calibrateOppositeIfNoCornerRelation(wing);
                calibrateOppositeIfNoCornerRelation(sibling);

                // make sure we can not find this twice to be calibrated
                allWings.erase(nextWingItt);

                // remove other opposite from the allWingsList if still present  (both are calibrated, so also this can be erased)      
                auto it = std::find(allWings.begin(), allWings.end(), sibling);
                if (it != allWings.end()) {
                    allWings.erase(it);
                }

                // no need to search further found opposite relation
                handledOppositeRelation=true;
                break;
            }
        }
        if ( !handledOppositeRelation) {
            LOG_ERROR("Failed to handle opposite relation !! -> programming error");
            return false;
        }
        nextWingItt = getWingWithOppositeTypeAsSibling();
    };
    // No need to search for male relations because already calibrated
    return true;

}


// IMPORTANT THIS CALIBRATION ASSUMES A CLOSED POSITION TO START WITH
void WingCalibrationHandler::startCalibration(std::future<void> & cancelObj,std::shared_ptr<IWing> entryWing, std::shared_ptr<IWingStatusPublisher> statusPublisher) {

    auto allWings = getAllWings(entryWing);
    // first clear all a calibrations 
    for ( auto & w: allWings) {
        w->clearCalibration();
    }
    // time how long it takes to calibrate is used later to make sure it is not stuck 
    auto startCalibMoment = std::chrono::steady_clock::now();
    // do calibration of the wings 
    bool success = runCorrectSequence(cancelObj, 
                        getAllWings(entryWing),
                        [&statusPublisher](std::shared_ptr<IWing> w)->bool{
                            statusPublisher->publishCalibrationOpenStarted(w->getWingId());
                            return w->calibrateOpen();}, 
                        [&statusPublisher](std::shared_ptr<IWing> w)->bool{
                            statusPublisher->publishCalibrationCloseStarted(w->getWingId());
                            return w->calibrateClose();});
    if ( !success) {
         for ( auto & w: allWings) {
            statusPublisher->publishCalibrateFailed(w->getWingId());
            w->clearCalibration();
         }
         return;
    }

    auto endOfCalibMoment = std::chrono::steady_clock::now();
    
    std::chrono::duration<double> secondsToCalibrate = endOfCalibMoment-startCalibMoment;
    statusPublisher->publishCalibrated(entryWing->getWingId());    
    LOG_DEBUG("Calibration of wings is done after " + std::to_string(secondsToCalibrate.count()) +" secondes");
    

    // functionality to move the wing OPEN or CLOSE , moveOpen boolean is used to define either OPEN or CLOSE as the movement to be done
    auto moveWingAndWaitOnIt = [& cancelObj, & secondsToCalibrate](std::shared_ptr<IWing> w, bool moveOpen)->bool {

        auto isCancelled = [&]() {
            if ((cancelObj.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) ) {
                return true;
            }
            return false;
        };

        auto startActionMoment = std::chrono::steady_clock::now();
        if(moveOpen) {
            LOG_DEBUG("Open action called on wing " + w->getWingId());
            w->getMasterWindow()->open();
        } else {
            LOG_DEBUG("Close action called on wing " + w->getWingId());
            w->getMasterWindow()->close();
        }
        while( true) {
            if ( isCancelled()) {
                return false;
            }
            if ( w->getMasterWindow()->isOntarget()) {
                LOG_DEBUG("__on target __");
                return true;
            }
            auto endOfActionMoment = std::chrono::steady_clock::now();
            std::chrono::duration<double> secondsActionBusy = endOfActionMoment-startActionMoment;
            // check if not running infinite, should never run longer than a full slower calibration
            if (secondsActionBusy.count() > secondsToCalibrate.count() ) {
                LOG_WARNING("Action is blocked, running already for " + std::to_string(secondsActionBusy.count()) +" secondes");
                return false;
            }
             std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    };
    // Open and close all the wings in the correct order to make sure that also the current measurement is done properly
    success = runCorrectSequence(cancelObj, 
        getAllWings(entryWing),
        [& statusPublisher, & moveWingAndWaitOnIt](std::shared_ptr<IWing> w)->bool{
            statusPublisher->publishCurrentMeasurmentOpenStarted(w->getWingId());
            return moveWingAndWaitOnIt(w,true);}, 
        [& statusPublisher, & moveWingAndWaitOnIt](std::shared_ptr<IWing> w)->bool{
            statusPublisher->publishCurrentMeasurmentCloseStarted(w->getWingId());
            return moveWingAndWaitOnIt(w,false);});

    if ( !success) {
         for ( auto & w: allWings) {
            statusPublisher->publishCalibrateFailed(w->getWingId());
            w->clearCalibration();
         }
    } else {
        for ( auto & w: allWings) {
            w->SetFullSetupCalibDone();
        }
        statusPublisher->publishCurrentMeasurmentFinished(entryWing->getWingId());
    }

}
void WingCalibrationHandler::setAllWingsCalibratedFlag(std::vector<std::shared_ptr<IWing>> allWings) {
    for ( auto & w: allWings) {
        w->SetFullSetupCalibDone();
    }
}
void WingCalibrationHandler::setAllWingsCalibratedFlag(std::shared_ptr<IWing> entryWing) {
    auto allWings = getAllWings(entryWing);
    setAllWingsCalibratedFlag(allWings);
}

void WingCalibrationHandler::clearCalibration(std::shared_ptr<IWing> entryWing, std::shared_ptr<IWingStatusPublisher> statusPublisher) {
   for ( auto & w: getAllWings(entryWing) ) {
        w->clearCalibration();
    }
}

bool WingCalibrationHandler::areAllWingsCalibrated(std::shared_ptr<IWing> entryWing) {
    bool allWingsAreCalibrated = true;
    for ( auto & w: getAllWings(entryWing) ) {
        if (!w->isCalibrated()) { 
            allWingsAreCalibrated=false; 
            break;
        }
    }
    if ( allWingsAreCalibrated) {
        setAllWingsCalibratedFlag(entryWing);
        return true;
    } else {
        return false;
    }
}

void WingCalibrationHandler::recursiveListOtherWings(std::vector<std::shared_ptr<IWing>> & list, std::reference_wrapper<IWing> wing  ) {
    
    const std::shared_ptr<std::vector<std::tuple<std::shared_ptr<IWing>,WingSiblingType>>> & siblings = wing.get().getSiblings();

    for(auto it = siblings->begin(); it< siblings->end(); it++ ) {
        const std::shared_ptr<IWing> & sibling = std::get<0>(*it);
        // check if not already in the list 
        auto findRes = std::find(list.begin(),list.end(),sibling);
        if (findRes == list.end()) {
            // add to list
            list.push_back(sibling);
            recursiveListOtherWings(list,std::ref(* sibling));
        }
    }
}

std::vector<std::shared_ptr<IWing>> WingCalibrationHandler::getAllWings(std::shared_ptr<IWing> entryWing)  {
    std::vector<std::shared_ptr<IWing>> siblingList;
    const auto & siblings = entryWing->getSiblings();
    
    if (siblings->empty()) {
        siblingList.push_back(entryWing);
        return siblingList;
    }

    for (auto it = siblings->begin(); it < siblings->end(); it++) {
        const std::shared_ptr<IWing> &sibling = std::get<0>(*it);
        // check if not already in the list
        auto findRes = std::find(siblingList.begin(), siblingList.end(), sibling);
        if (findRes == siblingList.end()) {
            // add to list
            siblingList.push_back(sibling);
            recursiveListOtherWings(siblingList, std::ref(*sibling));
        }
    }

    return siblingList;
}

std::vector<std::shared_ptr<IWing>>::iterator WingCalibrationHandler::getWingWithSpecificTypeAsSibling ( std::vector<std::shared_ptr<IWing>> & allWings, std::function<bool(WingSiblingType)> isSiblingTypeValid) {
    return std::find_if(std::begin(allWings), std::end(allWings), [& isSiblingTypeValid](std::shared_ptr<IWing> &x) {
        auto &siblings = x->getSiblings();

        // check if the wing has a sibling of valid type
        if (std::find_if(std::begin(*siblings), std::end(*siblings), [& isSiblingTypeValid](const std::tuple<std::shared_ptr<IWing>, WingSiblingType> &y) {
                return isSiblingTypeValid(std::get<1>(y));
            }) != std::end(*siblings))
        {
            return true;
        };
        return false;
    });
}