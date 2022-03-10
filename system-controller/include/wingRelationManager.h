    #ifndef WINGRELATIONMANAGER_H
#define WINGRELATIONMANAGER_H

#include "pch.h"
#include "positionTrack.h"
#include "windowPushZone.h"
#include "masterMotorizedWindow.h"
#include "systemSettings.h"

class IWingRelationManager {
    public:
        virtual ~IWingRelationManager(){};
        virtual void ManageMaleCorner(
            std::function<void(void)> onBlockedByMaleWing, 
            std::function<void(void)> onFreedByMaleWing, 
            std::shared_ptr<IWindowPushZone> pushZoneMaleWing, 
            std::shared_ptr<IPositionTrack> maleWing,
            std::shared_ptr<IPositionTrack> femaleWing ) const = 0;
        virtual void ManageFemaleCorner(
            std::function<void(void)> onBlockedByFemaleWing, 
            std::function<void(void)> onFreedByFemaleWing, 
            std::shared_ptr<IWindowPushZone> pushZoneFemaleWing, 
            std::shared_ptr<IPositionTrack> maleWing,
            std::shared_ptr<IPositionTrack> femaleWing) const = 0;
        virtual void ManageOpposite(
            std::function<void(void)> onBlockedByOtherWing,
            std::function<void(void)> onFreedByOtherWing,
            std::shared_ptr<IWindowPushZone> pushZoneOtherWing,
            std::shared_ptr<IPositionTrack> thisWing,
            int tailDistanceToFullyCloseOfOtherWing,
            bool otherWingFullyClosed,
            bool isWingOnTarget) const = 0;
};



class WingRelationManager : public IWingRelationManager{

    public:
        WingRelationManager();
        void ManageMaleCorner  (
            std::function<void(void)> onBlockedByMaleWing, 
            std::function<void(void)> onFreedByMaleWing, 
            std::shared_ptr<IWindowPushZone> pushZoneMaleWing, 
            std::shared_ptr<IPositionTrack> maleWing,
            std::shared_ptr<IPositionTrack> femaleWing) const override;
          

        void  ManageFemaleCorner(
            std::function<void(void)> onBlockedByFemaleWing, 
            std::function<void(void)> onFreedByFemaleWing, 
            std::shared_ptr<IWindowPushZone> pushZoneFemaleWing, 
            std::shared_ptr<IPositionTrack> maleWing,
            std::shared_ptr<IPositionTrack> femaleWing) const override;

        void ManageOpposite(
            std::function<void(void)> onBlockedByOtherWing,
            std::function<void(void)> onFreedByOtherWing,
            std::shared_ptr<IWindowPushZone> pushZoneOtherWing,
            std::shared_ptr<IPositionTrack> thisWing,
            int tailDistanceToFullyCloseOfOtherWing,
            bool otherWingFullyClosed,
            bool isWingOnTarget) const override;
};




#endif //WINGRELATIONMANAGER_H