#include "wingRelationManager.h"
#include "wing.h"
#include "log.h"


WingRelationManager::WingRelationManager() {

}

// this function can controle (block/unblock) the female wing and can push the male
void WingRelationManager::ManageMaleCorner  (
    std::function<void(void)> onBlockedByMaleWing, 
    std::function<void(void)> onFreedByMaleWing, 
    std::shared_ptr<IWindowPushZone> pushZoneMaleWing, 
    std::shared_ptr<IPositionTrack> maleWing,
    std::shared_ptr<IPositionTrack> femaleWing
    ) const 
{    
    bool needToHoldTheFemale = false;
    // if this wing is FULLY closed the Male restriction of the exclusion zone is released
    if( femaleWing->getPosition() <=0 &&  femaleWing->getTarget() <=0 ) {
        pushZoneMaleWing->inActivate();
        needToHoldTheFemale = true;  
    // don't allow movement of female wing whitin corner zone when male is around 
    } else if ( femaleWing->getPosition() <= SystemSettings::getInstance().getCornerZone()) { 
        if ( maleWing->getPosition() <= SystemSettings::getInstance().getCornerZone()) {
            pushZoneMaleWing->setMinOpening(SystemSettings::getInstance().getCornerZone() );   
            needToHoldTheFemale = true;      
        }
    // male is free to move, this female is not in the corner 
    } else {   
        if ( femaleWing->getPosition() <= SystemSettings::getInstance().getTriggerPushWingDistance() && femaleWing->getTarget()  <= SystemSettings::getInstance().getCornerZone()) {            
            // start pushing out of the way upfront to avoid stopping needed
            pushZoneMaleWing->setMinOpening(SystemSettings::getInstance().getCornerZone() );                
        }
        else {
            pushZoneMaleWing->inActivate();
        }
    } 
    if (needToHoldTheFemale) {
        onBlockedByMaleWing();
    } else {
        onFreedByMaleWing();
    }
}
// this function can controle (block/unblock) the Male wing and can push the Female
void WingRelationManager::ManageFemaleCorner(
    std::function<void(void)> onBlockedByFemaleWing, 
    std::function<void(void)> onFreedByFemaleWing, 
    std::shared_ptr<IWindowPushZone> pushZoneFemaleWing, 
    std::shared_ptr<IPositionTrack> maleWing,
    std::shared_ptr<IPositionTrack> femaleWing) const
{
    bool needToHoldTheMale = false;
    // this wing is the male and should push female close only if female is within SystemSettings::getInstance().getCornerZone()    
    if ( maleWing->getPosition() <= SystemSettings::getInstance().getCornerZone())  { 
        if(femaleWing->getPosition() <= SystemSettings::getInstance().getCornerZone() && femaleWing->getPosition() > 0) {
            needToHoldTheMale=true; // when female is not fully closed hold the male when entering cornerzone
        }
        pushZoneFemaleWing->inActivate();
        if(femaleWing->getTarget() <= SystemSettings::getInstance().getCornerZone()) {
            pushZoneFemaleWing->setMaxOpening(0);
        }else {
            pushZoneFemaleWing->setMinOpening(SystemSettings::getInstance().getCornerZone());
        }
    } else if ( maleWing->getPosition() < SystemSettings::getInstance().getTriggerPushWingDistance() && maleWing->getTarget() <= SystemSettings::getInstance().getCornerZone()) {        
        pushZoneFemaleWing->inActivate();
         if(femaleWing->getTarget() <= SystemSettings::getInstance().getCornerZone()) {
            pushZoneFemaleWing->setMaxOpening(0);
        }else {
            pushZoneFemaleWing->setMinOpening(SystemSettings::getInstance().getCornerZone());
        }
    } else {
        pushZoneFemaleWing->inActivate();
    }
     
    if (needToHoldTheMale) {
        onBlockedByFemaleWing();
    }else {
        onFreedByFemaleWing();
    }
}



// this function can controle the one wing (block/unblock) and can push the other opposite
void WingRelationManager::ManageOpposite  (
    std::function<void(void)> onBlockedByOtherWing, 
    std::function<void(void)> onFreedByOtherWing,
    std::shared_ptr<IWindowPushZone> pushZoneOtherWing, 
    std::shared_ptr<IPositionTrack> thisWing,    
    int tailDistanceToFullyCloseOfOtherWing,
    bool otherWingFullyClosed,
    bool isWingOnTarget
    ) const 
{    
    bool needToHold = false;
    
    if (  tailDistanceToFullyCloseOfOtherWing - thisWing->getPosition() < SystemSettings::getInstance().getTriggerPushWingDistance() && thisWing->getPosition() < thisWing->getTarget()) {
        // prevent collision
        if ( tailDistanceToFullyCloseOfOtherWing - thisWing->getPosition() < SystemSettings::getInstance().getOppositeZone() ) {
            needToHold=true;
        }

        // when target is near other wing push it out of the way (start pushing when arriving at the other wing)        
        if ( tailDistanceToFullyCloseOfOtherWing - thisWing->getTarget() < SystemSettings::getInstance().getOppositeZone()) {
            int maxOpening = tailDistanceToFullyCloseOfOtherWing - (thisWing->getTarget() + SystemSettings::getInstance().getOppositeZone());
            pushZoneOtherWing->setMaxOpening(maxOpening);
        }
    }
    // when wing is pushed by opposite it looses its original target,  so when this wing is on target it can release the pushzone because
    // it will not go back. When the wing target is outside the collision zone there is no need to push
    if (isWingOnTarget || (tailDistanceToFullyCloseOfOtherWing - thisWing->getTarget() > SystemSettings::getInstance().getOppositeZone() ) ){
        pushZoneOtherWing->inActivate();
    }
    // because there is no fixed value for fully closed that the tailDistanceToFullyCloseOfOtherWing can express there is a boolean used
    if (otherWingFullyClosed) {
        needToHold=false;
    }
    // if sibling is fully open don't allow the wing to move
    if (tailDistanceToFullyCloseOfOtherWing < 5 ) {
        needToHold=true;
    }
    
    // We can deal with this maybe with only releasing once somehow
    if (needToHold) {
        onBlockedByOtherWing();
    } else {
        onFreedByOtherWing();
    }
    // std::string hold = needToHold ? "HOLD" : "RELEASE";
    // LOG_DEBUG( hold +  " ->TailToClose:" + std::to_string(tailDistanceToFullyCloseOfOtherWing) 
    //     + "[curr/target]:[" + std::to_string(thisWing->getPosition()) + "/" + std::to_string(thisWing->getTarget()) + "]" );
}