#include <gtest/gtest.h>

#include "log.h"
#include "systemSettings.h"
using namespace std;


TEST(systemSettings,testSetters ){
    Log::Init();
   
    int refChicanoverlap = SystemSettings::getInstance().getChicanOverlap();
    int refChicanzone = SystemSettings::getInstance().getChicanZone();
    int refSlowdowndist = SystemSettings::getInstance().getSlowdownDist();
    int refOppositeZone = SystemSettings::getInstance().getOppositeZone();
    int refCornerZone = SystemSettings::getInstance().getCornerZone();
    int refTriggerPushWingDistance = SystemSettings::getInstance().getTriggerPushWingDistance();
    
    int belowMin = 5;
    int aboveMax = 50000;
    SystemSettings::getInstance().setChicanOverlap(belowMin);   
    ASSERT_NE(belowMin,SystemSettings::getInstance().getChicanOverlap()) << "Minimum boudry should be hit";
    SystemSettings::getInstance().setChicanOverlap(aboveMax);   
    ASSERT_NE(aboveMax,SystemSettings::getInstance().getChicanOverlap()) << "Maximum boudry should be hit";
    
    SystemSettings::getInstance().setChicanZone(belowMin);  
    ASSERT_NE(belowMin,SystemSettings::getInstance().getChicanZone()) << "Minimum boudry should be hit";
    SystemSettings::getInstance().setChicanZone(aboveMax);  
    ASSERT_NE(aboveMax,SystemSettings::getInstance().getChicanZone()) << "Maximum boudry should be hit";
    
    SystemSettings::getInstance().setSlowdownDist(belowMin);    
    ASSERT_NE(belowMin,SystemSettings::getInstance().getSlowdownDist()) << "Minimum boudry should be hit";
    SystemSettings::getInstance().setSlowdownDist(aboveMax);    
    ASSERT_NE(aboveMax,SystemSettings::getInstance().getSlowdownDist()) << "Maximum boudry should be hit";
    
    SystemSettings::getInstance().setOppositeZone(belowMin);    
    ASSERT_NE(belowMin,SystemSettings::getInstance().getOppositeZone()) << "Minimum boudry should be hit";
    SystemSettings::getInstance().setOppositeZone(aboveMax);    
    ASSERT_NE(aboveMax,SystemSettings::getInstance().getOppositeZone()) << "Maximum boudry should be hit";
    
    SystemSettings::getInstance().setCornerZone(belowMin);  
    ASSERT_NE(belowMin,SystemSettings::getInstance().getCornerZone()) << "Minimum boudry should be hit";
    SystemSettings::getInstance().setCornerZone(aboveMax);  
    ASSERT_NE(aboveMax,SystemSettings::getInstance().getCornerZone()) << "Maximum boudry should be hit";
    
    SystemSettings::getInstance().setTriggerPushWingDistance(belowMin); 
    ASSERT_NE(belowMin,SystemSettings::getInstance().getTriggerPushWingDistance()) << "Minimum boudry should be hit";
    SystemSettings::getInstance().setTriggerPushWingDistance(aboveMax); 
    ASSERT_NE(aboveMax,SystemSettings::getInstance().getTriggerPushWingDistance()) << "Maximum boudry should be hit";

}